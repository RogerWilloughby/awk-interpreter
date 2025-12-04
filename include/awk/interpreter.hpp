#ifndef AWK_INTERPRETER_HPP
#define AWK_INTERPRETER_HPP

#include "ast.hpp"
#include "value.hpp"
#include "environment.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <cstdio>
#include <streambuf>
#include <regex>

namespace awk {

// ============================================================================
// RegexCache - Cached compiled regex patterns for performance
// ============================================================================
class RegexCache {
public:
    // Cache size (LRU-like, but simply implemented)
    static constexpr size_t MAX_CACHE_SIZE = 64;

    RegexCache() = default;

    // Get regex from cache or compile it
    const std::regex& get(const std::string& pattern,
                          std::regex_constants::syntax_option_type flags);

    // Clear cache (e.g., when IGNORECASE changes)
    void clear() {
        cache_.clear();
        hits_ = 0;
        misses_ = 0;
    }

    // Statistics
    size_t hits() const { return hits_; }
    size_t misses() const { return misses_; }
    size_t size() const { return cache_.size(); }

    double hit_rate() const {
        size_t total = hits_ + misses_;
        return total > 0 ? static_cast<double>(hits_) / total : 0.0;
    }

private:
    // Cache entry with pattern + flags as key
    struct CacheKey {
        std::string pattern;
        std::regex_constants::syntax_option_type flags;

        bool operator==(const CacheKey& other) const {
            return pattern == other.pattern && flags == other.flags;
        }
    };

    struct CacheKeyHash {
        size_t operator()(const CacheKey& key) const {
            size_t h1 = std::hash<std::string>{}(key.pattern);
            size_t h2 = std::hash<unsigned int>{}(static_cast<unsigned int>(key.flags));
            return h1 ^ (h2 << 1);
        }
    };

    std::unordered_map<CacheKey, std::shared_ptr<std::regex>, CacheKeyHash> cache_;
    size_t hits_ = 0;
    size_t misses_ = 0;

    // Remove oldest entry when cache is full
    void evict_if_needed();
};

// ============================================================================
// PipeStreamBuf - streambuf wrapper for FILE* pipes
// ============================================================================
class PipeStreamBuf : public std::streambuf {
public:
    explicit PipeStreamBuf(FILE* pipe) : pipe_(pipe) {}

    ~PipeStreamBuf() override {
        sync();
    }

    // Close the pipe and return the exit code
    int close_pipe() {
        if (pipe_) {
            sync();
#ifdef _WIN32
            int result = _pclose(pipe_);
#else
            int result = pclose(pipe_);
#endif
            pipe_ = nullptr;
            return result;
        }
        return -1;
    }

    bool is_open() const { return pipe_ != nullptr; }

protected:
    int_type overflow(int_type c) override {
        if (c != traits_type::eof() && pipe_) {
            if (fputc(c, pipe_) == EOF) {
                return traits_type::eof();
            }
        }
        return c;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override {
        if (pipe_) {
            return static_cast<std::streamsize>(fwrite(s, 1, n, pipe_));
        }
        return 0;
    }

    int sync() override {
        if (pipe_) {
            return fflush(pipe_) == 0 ? 0 : -1;
        }
        return 0;
    }

private:
    FILE* pipe_;
};

// ============================================================================
// PipeOStream - ostream wrapper for pipes
// ============================================================================
class PipeOStream : public std::ostream {
public:
    explicit PipeOStream(FILE* pipe)
        : std::ostream(&buf_), buf_(pipe) {}

    int close_pipe() { return buf_.close_pipe(); }
    bool is_open() const { return buf_.is_open(); }

private:
    PipeStreamBuf buf_;
};

// ============================================================================
// Coprocess - Bidirectional pipe for |& (gawk extension)
// ============================================================================
struct Coprocess {
    FILE* to_child = nullptr;    // Write to child process
    FILE* from_child = nullptr;  // Read from child process
#ifdef _WIN32
    void* process_handle = nullptr;  // HANDLE on Windows
#else
    int pid = -1;                // Process ID on Unix
#endif

    Coprocess() = default;
    Coprocess(const Coprocess&) = delete;
    Coprocess& operator=(const Coprocess&) = delete;
    Coprocess(Coprocess&& other) noexcept
        : to_child(other.to_child)
        , from_child(other.from_child)
#ifdef _WIN32
        , process_handle(other.process_handle)
#else
        , pid(other.pid)
#endif
    {
        other.to_child = nullptr;
        other.from_child = nullptr;
#ifdef _WIN32
        other.process_handle = nullptr;
#else
        other.pid = -1;
#endif
    }

    ~Coprocess() {
        close();
    }

    int close();
    bool is_open() const { return to_child != nullptr || from_child != nullptr; }
};

// ============================================================================
// Utility Functions
// ============================================================================

// Convert AWK-style replacement string to std::regex replacement format
// & -> $& (matched text), \& -> literal &, \\ -> literal backslash
// If support_backrefs is true, \1-\9 -> $1-$9 (for gensub)
inline std::string convert_awk_replacement(const std::string& replacement, bool support_backrefs = false) {
    std::string result;
    result.reserve(replacement.length() * 2);

    for (size_t i = 0; i < replacement.length(); ++i) {
        if (replacement[i] == '\\' && i + 1 < replacement.length()) {
            char next = replacement[i + 1];
            if (support_backrefs && next >= '0' && next <= '9') {
                // \1-\9 -> $1-$9 (gensub backreferences)
                result += '$';
                result += next;
                ++i;
            } else if (next == '&') {
                result += '&';  // \& -> literal &
                ++i;
            } else if (next == '\\') {
                result += '\\'; // \\ -> literal backslash
                ++i;
            } else {
                result += replacement[i];
            }
        } else if (replacement[i] == '&') {
            result += "$&";  // & -> $& (matched text)
        } else {
            result += replacement[i];
        }
    }

    return result;
}

// ============================================================================
// Control Flow Exceptions
// ============================================================================

// Control flow exceptions
struct BreakException {};
struct ContinueException {};
struct NextException {};
struct NextfileException {};
struct ReturnException {
    AWKValue value;
    explicit ReturnException(AWKValue v = AWKValue()) : value(std::move(v)) {}
};
struct ExitException {
    int status;
    explicit ExitException(int s = 0) : status(s) {}
};

// The AWK interpreter
class Interpreter {
public:
    Interpreter();

    // Execute program
    void run(Program& program, const std::vector<std::string>& input_files);

    // For built-in functions: access to environment
    Environment& environment() { return env_; }
    const Environment& environment() const { return env_; }

    // Field access
    AWKValue& get_field(int index);
    void set_field(int index, AWKValue value);
    int field_count() const;

    // Current record
    const std::string& current_record() const { return current_record_; }
    void set_record(const std::string& record);

    // Output
    std::ostream& output_stream() { return *output_; }
    void set_output_stream(std::ostream& os) { output_ = &os; }

    // Error output
    std::ostream& error_stream() { return *error_; }
    void set_error_stream(std::ostream& os) { error_ = &os; }

    // File management for close() and fflush()
    bool close_file(const std::string& filename);
    bool flush_file(const std::string& filename);
    void flush_all_files();

    // Regex flags based on IGNORECASE
    std::regex_constants::syntax_option_type get_regex_flags() const {
        auto flags = std::regex_constants::extended;
        if (env_.IGNORECASE().to_bool()) {
            flags |= std::regex_constants::icase;
        }
        return flags;
    }

    // i18n (internationalization)
    std::string bind_textdomain(const std::string& domain, const std::string& directory);
    std::string get_textdomain_directory(const std::string& domain) const;

    // Regex cache for performance
    const std::regex& get_cached_regex(const std::string& pattern);
    RegexCache& regex_cache() { return regex_cache_; }
    const RegexCache& regex_cache() const { return regex_cache_; }

private:
    Environment env_;
    Program* current_program_ = nullptr;

    // Fields
    std::string current_record_;
    std::vector<std::string> fields_;
    bool fields_dirty_ = false;
    bool record_dirty_ = false;

    // Field access (non-static to avoid thread-safety and corruption issues)
    AWKValue field0_;
    AWKValue empty_field_;
    std::vector<AWKValue> field_values_;

    // Output streams
    std::ostream* output_ = &std::cout;
    std::ostream* error_ = &std::cerr;

    // Open files/pipes
    std::unordered_map<std::string, std::unique_ptr<std::ofstream>> output_files_;
    std::unordered_map<std::string, std::unique_ptr<std::ifstream>> input_files_;
    std::unordered_map<std::string, FILE*> input_pipes_;  // For command | getline
    std::unordered_map<std::string, std::unique_ptr<PipeOStream>> output_pipes_;  // For print | command
    std::unordered_map<std::string, std::unique_ptr<Coprocess>> coprocesses_;  // For |& (gawk extension)

    // Regex cache for performance
    RegexCache regex_cache_;

    // Cached special variable values (performance optimization)
    // These are frequently accessed but rarely change
    mutable std::string cached_rs_;
    mutable std::string cached_fs_;
    mutable std::string cached_ofs_;
    mutable std::string cached_fpat_;
    mutable std::string cached_subsep_;
    mutable bool special_vars_dirty_ = true;

    // Helper methods to get cached special variables
    const std::string& get_cached_rs();
    const std::string& get_cached_fs();
    const std::string& get_cached_ofs();
    const std::string& get_cached_fpat();
    const std::string& get_cached_subsep();
    void invalidate_special_var_cache() { special_vars_dirty_ = true; }

    // ========================================================================
    // Execution
    // ========================================================================

    void execute_begin_rules();
    void execute_end_rules();
    void execute_beginfile_rules();
    void execute_endfile_rules();
    void execute_main_rules();

    void process_file(const std::string& filename);
    void process_stream(std::istream& input, const std::string& filename);
    bool read_record(std::istream& input);

    // ========================================================================
    // Pattern Matching
    // ========================================================================

    bool pattern_matches(Pattern& pattern);

    // ========================================================================
    // Statement Execution
    // ========================================================================

    void execute(Stmt& stmt);
    void execute(BlockStmt& stmt);
    void execute(IfStmt& stmt);
    void execute(WhileStmt& stmt);
    void execute(DoWhileStmt& stmt);
    void execute(ForStmt& stmt);
    void execute(ForInStmt& stmt);
    void execute(SwitchStmt& stmt);
    void execute(PrintStmt& stmt);
    void execute(PrintfStmt& stmt);
    void execute(ExprStmt& stmt);
    void execute(DeleteStmt& stmt);

    // ========================================================================
    // Expression Evaluation
    // ========================================================================

    AWKValue evaluate(Expr& expr);
    AWKValue evaluate(LiteralExpr& expr);
    AWKValue evaluate(RegexExpr& expr);
    AWKValue evaluate(VariableExpr& expr);
    AWKValue evaluate(FieldExpr& expr);
    AWKValue evaluate(ArrayAccessExpr& expr);
    AWKValue evaluate(BinaryExpr& expr);
    AWKValue evaluate(UnaryExpr& expr);
    AWKValue evaluate(TernaryExpr& expr);
    AWKValue evaluate(AssignExpr& expr);
    AWKValue evaluate(CallExpr& expr);
    AWKValue evaluate(IndirectCallExpr& expr);  // gawk Extension
    AWKValue evaluate(MatchExpr& expr);
    AWKValue evaluate(ConcatExpr& expr);
    AWKValue evaluate(GetlineExpr& expr);
    AWKValue evaluate(InExpr& expr);

    // ========================================================================
    // Helper Functions
    // ========================================================================

    // Truth value of an AWKValue
    bool is_truthy(const AWKValue& value);

    // Parse fields
    void parse_fields();
    void rebuild_record();

    // Function call
    AWKValue call_function(const std::string& name, std::vector<AWKValue>& args);
    AWKValue call_user_function(FunctionDef* func, std::vector<AWKValue>& args);

    // Get LValue reference
    AWKValue& get_lvalue(Expr& expr);

    // Output redirect
    std::ostream* get_output_stream(const std::string& target, RedirectType type);

    // Regex matching
    bool regex_match(const AWKValue& text, const AWKValue& pattern);

    // Register built-in functions
    void register_builtins();
    void register_math_builtins();
    void register_string_builtins();
    void register_io_builtins();
    void register_time_builtins();
    void register_bit_builtins();
    void register_type_builtins();

    // sprintf implementation
    std::string do_sprintf(const std::string& format, const std::vector<AWKValue>& args);

    // getline helper functions
    std::istream* get_input_file(const std::string& filename);
    FILE* get_input_pipe(const std::string& command);
    int getline_from_stream(std::istream& stream, Expr* variable, bool update_nr);
    int getline_from_pipe(FILE* pipe, Expr* variable, bool update_nr);

    // Coprocess helper functions (gawk |& extension)
    Coprocess* get_or_create_coprocess(const std::string& command);
    int getline_from_coprocess(const std::string& command, Expr* variable);
    bool write_to_coprocess(const std::string& command, const std::string& data);
    bool close_coprocess(const std::string& command);

    // Cleanup
    void cleanup_io();
};

} // namespace awk

#endif // AWK_INTERPRETER_HPP
