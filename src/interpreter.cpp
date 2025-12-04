// ============================================================================
// interpreter.cpp - AWK Interpreter Core
// ============================================================================

#include "awk/interpreter.hpp"
#include "awk/i18n.hpp"
#include "awk/platform.hpp"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <regex>
#include <cstdio>
#include <ctime>
#include <random>
#include <cstring>
#include <cerrno>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace awk {

// ============================================================================
// Interpreter Implementation
// ============================================================================

Interpreter::Interpreter() {
    register_builtins();
}

// ============================================================================
// Cached Special Variable Accessors (Performance Optimization)
// ============================================================================

void Interpreter::refresh_special_var_cache() {
    if (!special_vars_dirty_) return;

    cached_rs_ = env_.RS().to_string();
    cached_fs_ = env_.FS().to_string();
    cached_ofs_ = env_.OFS().to_string();
    cached_ors_ = env_.ORS().to_string();
    cached_ofmt_ = env_.OFMT().to_string();
    cached_fpat_ = env_.FPAT().to_string();
    cached_subsep_ = env_.SUBSEP().to_string();

    special_vars_dirty_ = false;
}

const std::string& Interpreter::get_cached_rs() {
    refresh_special_var_cache();
    return cached_rs_;
}

const std::string& Interpreter::get_cached_fs() {
    refresh_special_var_cache();
    return cached_fs_;
}

const std::string& Interpreter::get_cached_ofs() {
    refresh_special_var_cache();
    return cached_ofs_;
}

const std::string& Interpreter::get_cached_ors() {
    refresh_special_var_cache();
    return cached_ors_;
}

const std::string& Interpreter::get_cached_ofmt() {
    refresh_special_var_cache();
    return cached_ofmt_;
}

const std::string& Interpreter::get_cached_fpat() {
    refresh_special_var_cache();
    return cached_fpat_;
}

const std::string& Interpreter::get_cached_subsep() {
    refresh_special_var_cache();
    return cached_subsep_;
}

// ============================================================================
// Program Execution
// ============================================================================

void Interpreter::run(Program& program, const std::vector<std::string>& input_files) {
    current_program_ = &program;

    // Register functions
    for (auto& func : program.functions) {
        env_.register_function(func->name, func.get());
    }

    // Set ARGV
    std::vector<std::string> argv;
    argv.push_back("awk");  // ARGV[0]
    for (const auto& file : input_files) {
        argv.push_back(file);
    }
    env_.set_argv(argv);

    try {
        // Execute BEGIN rules
        execute_begin_rules();

        // Process files
        if (input_files.empty()) {
            // No files: read from stdin
            env_.FILENAME() = AWKValue("");
            process_stream(std::cin, "");
        } else {
            for (const auto& filename : input_files) {
                try {
                    process_file(filename);
                } catch (const NextfileException&) {
                    // Next file
                    continue;
                }
            }
        }

        // Execute END rules
        execute_end_rules();

    } catch (const ExitException&) {
        // Program end via exit()
        // Status is ignored (could be used as return code)
    }

    // Close all open pipes and files
    cleanup_io();

    current_program_ = nullptr;
}

void Interpreter::process_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        *error_ << "awk: can't open file " << filename << ": " << safe_strerror(errno) << "\n";
        return;
    }

    env_.FILENAME() = AWKValue(filename);
    env_.FNR() = AWKValue(0);

    execute_beginfile_rules();

    process_stream(file, filename);

    execute_endfile_rules();
}

void Interpreter::process_stream(std::istream& input, [[maybe_unused]] const std::string& filename) {
    while (read_record(input)) {
        try {
            execute_main_rules();
        } catch (const NextException&) {
            // Next record
            continue;
        }
    }
}

// Helper: Read record in paragraph mode (RS = "")
// Records are separated by one or more blank lines
static bool read_record_paragraph_mode(std::istream& input, std::string& record, std::string& rt) {
    record.clear();
    std::string line;
    bool found_content = false;

    // Skip leading blank lines
    while (std::getline(input, line)) {
        if (!line.empty()) {
            record = line;
            found_content = true;
            break;
        }
    }

    if (!found_content) {
        rt = "";  // EOF - no terminator
        return false;
    }

    // Read more lines until blank line or EOF
    while (std::getline(input, line)) {
        if (line.empty()) {
            rt = "\n";  // Blank line as terminator
            return true;
        }
        record += "\n" + line;
    }

    rt = "";  // EOF as terminator
    return true;
}

// Helper: Read record in line mode (RS = "\n")
static bool read_record_line_mode(std::istream& input, std::string& record, std::string& rt) {
    if (!std::getline(input, record)) {
        rt = "";  // EOF
        return false;
    }
    rt = "\n";  // Newline as terminator
    return true;
}

// Helper: Read record with single-character RS
static bool read_record_single_char_mode(std::istream& input, std::string& record,
                                          std::string& rt, char delimiter) {
    record.clear();
    char c;

    while (input.get(c)) {
        if (c == delimiter) {
            rt = std::string(1, delimiter);
            return true;
        }
        record += c;
    }

    if (record.empty() && input.eof()) {
        rt = "";  // EOF
        return false;
    }

    rt = "";  // EOF, no terminator found
    return true;
}

// Helper: Read record with multi-character RS (simplified)
static bool read_record_multi_char_mode(std::istream& input, std::string& record, std::string& rt) {
    // Simplified: just read lines (full regex RS would require more complex parsing)
    if (!std::getline(input, record)) {
        rt = "";  // EOF
        return false;
    }
    rt = "\n";  // getline removes \n
    return true;
}

bool Interpreter::read_record(std::istream& input) {
    // Use cached RS for performance (frequently accessed, rarely changes)
    const std::string& rs = get_cached_rs();
    std::string rt;
    bool success;

    if (rs.empty()) {
        success = read_record_paragraph_mode(input, current_record_, rt);
    } else if (rs == "\n") {
        success = read_record_line_mode(input, current_record_, rt);
    } else if (rs.length() == 1) {
        success = read_record_single_char_mode(input, current_record_, rt, rs[0]);
    } else {
        success = read_record_multi_char_mode(input, current_record_, rt);
    }

    if (!success) {
        env_.RT() = AWKValue("");
        return false;
    }

    // Set RT variable (gawk extension)
    env_.RT() = AWKValue(rt);

    // Update counters
    double nr = env_.NR().to_number() + 1;
    double fnr = env_.FNR().to_number() + 1;
    env_.NR() = AWKValue(nr);
    env_.FNR() = AWKValue(fnr);

    // Invalidate special var cache after user code runs (in case they modified RS/FS/etc)
    special_vars_dirty_ = true;

    // Re-parse fields
    record_dirty_ = true;
    parse_fields();

    return true;
}

// ============================================================================
// Execute Rules
// ============================================================================

void Interpreter::execute_begin_rules() {
    if (!current_program_) return;

    for (auto& rule : current_program_->rules) {
        if (rule->pattern.type == PatternType::BEGIN) {
            if (rule->action) {
                execute(*rule->action);
            }
        }
    }
}

void Interpreter::execute_end_rules() {
    if (!current_program_) return;

    for (auto& rule : current_program_->rules) {
        if (rule->pattern.type == PatternType::END) {
            if (rule->action) {
                execute(*rule->action);
            }
        }
    }
}

void Interpreter::execute_beginfile_rules() {
    if (!current_program_) return;

    for (auto& rule : current_program_->rules) {
        if (rule->pattern.type == PatternType::BEGINFILE) {
            if (rule->action) {
                execute(*rule->action);
            }
        }
    }
}

void Interpreter::execute_endfile_rules() {
    if (!current_program_) return;

    for (auto& rule : current_program_->rules) {
        if (rule->pattern.type == PatternType::ENDFILE) {
            if (rule->action) {
                execute(*rule->action);
            }
        }
    }
}

void Interpreter::execute_main_rules() {
    if (!current_program_) return;

    for (auto& rule : current_program_->rules) {
        if (rule->pattern.type == PatternType::BEGIN ||
            rule->pattern.type == PatternType::END ||
            rule->pattern.type == PatternType::BEGINFILE ||
            rule->pattern.type == PatternType::ENDFILE) {
            continue;
        }

        if (pattern_matches(rule->pattern)) {
            if (rule->action) {
                execute(*rule->action);
            } else {
                // Default action: print $0
                *output_ << current_record_ << env_.ORS().to_string();
            }
        }
    }
}

// ============================================================================
// Pattern Matching
// ============================================================================

bool Interpreter::pattern_matches(Pattern& pattern) {
    switch (pattern.type) {
        case PatternType::EMPTY:
            return true;

        case PatternType::EXPRESSION:
            return is_truthy(evaluate(*pattern.expr));

        case PatternType::REGEX: {
            AWKValue regex_val = evaluate(*pattern.expr);
            return regex_match(AWKValue(current_record_), regex_val);
        }

        case PatternType::RANGE: {
            // Helper function to evaluate expression as pattern
            // If it's a RegexExpr, match against $0, otherwise evaluate as truthy
            auto eval_range_expr = [this](Expr* expr) -> bool {
                if (auto* regex_expr = dynamic_cast<RegexExpr*>(expr)) {
                    // Regex pattern: match against current line ($0)
                    AWKValue regex_val = evaluate(*regex_expr);
                    return regex_match(AWKValue(current_record_), regex_val);
                } else {
                    // Expression pattern: evaluate as truthy
                    return is_truthy(evaluate(*expr));
                }
            };

            if (pattern.range_active) {
                // Check if end pattern matches
                bool end_matches = eval_range_expr(pattern.range_end.get());
                if (end_matches) {
                    pattern.range_active = false;
                }
                return true;  // Still in range
            } else {
                // Check if start pattern matches
                bool start_matches = eval_range_expr(pattern.expr.get());
                if (start_matches) {
                    // Check if end matches simultaneously
                    bool end_matches = eval_range_expr(pattern.range_end.get());
                    if (!end_matches) {
                        pattern.range_active = true;
                    }
                    return true;
                }
                return false;
            }
        }

        default:
            return false;
    }
}

// ============================================================================
// Function Call Expression (CallExpr) - Remains here due to complexity
// ============================================================================

AWKValue Interpreter::evaluate(CallExpr& expr) {
    // Special handling for sub/gsub/split - these require LValue access
    if (expr.function_name == "sub" || expr.function_name == "gsub") {
        if (expr.arguments.size() >= 2) {
            std::string pattern = evaluate(*expr.arguments[0]).to_string();
            std::string replacement = evaluate(*expr.arguments[1]).to_string();

            // Determine target variable
            bool modify_record = (expr.arguments.size() < 3);
            std::string target_str;
            AWKValue* target_var = nullptr;

            if (!modify_record) {
                // Third argument is the target variable
                if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[2].get())) {
                    target_var = &env_.get_variable(var_expr->name);
                    target_str = target_var->to_string();
                } else if (auto* field_expr = dynamic_cast<FieldExpr*>(expr.arguments[2].get())) {
                    int idx = static_cast<int>(evaluate(*field_expr->index).to_number());
                    target_str = get_field(idx).to_string();
                    // For fields, set target_var to nullptr and handle separately
                } else {
                    target_str = evaluate(*expr.arguments[2]).to_string();
                }
            } else {
                target_str = current_record_;
            }

            try {
                const std::regex& re = get_cached_regex(pattern);
                std::string awk_replacement = convert_awk_replacement(replacement);

                std::string result;
                int count = 0;

                if (expr.function_name == "sub") {
                    result = std::regex_replace(target_str, re, awk_replacement,
                                                std::regex_constants::format_first_only);
                    count = (result != target_str) ? 1 : 0;
                } else {  // gsub
                    // Count replacements
                    std::string::const_iterator searchStart(target_str.cbegin());
                    std::smatch match;
                    while (std::regex_search(searchStart, target_str.cend(), match, re)) {
                        ++count;
                        searchStart = match.suffix().first;
                        if (match.length() == 0) {
                            if (searchStart != target_str.cend()) ++searchStart;
                            else break;
                        }
                    }
                    result = std::regex_replace(target_str, re, awk_replacement);
                }

                // Write result back
                if (result != target_str) {
                    if (modify_record) {
                        set_record(result);
                    } else if (target_var) {
                        *target_var = AWKValue(result);
                    } else if (auto* field_expr = dynamic_cast<FieldExpr*>(expr.arguments[2].get())) {
                        int idx = static_cast<int>(evaluate(*field_expr->index).to_number());
                        set_field(idx, AWKValue(result));
                    }
                }

                return AWKValue(static_cast<double>(count));

            } catch (const std::regex_error& e) {
                *error_ << "awk: " << expr.function_name << ": invalid regex '" << pattern << "': " << e.what() << "\n";
                return AWKValue(0.0);
            }
        }
        return AWKValue(0.0);
    }

    if (expr.function_name == "split") {
        if (expr.arguments.size() >= 2) {
            std::string str = evaluate(*expr.arguments[0]).to_string();

            // Second argument must be an array name
            std::string array_name;
            if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[1].get())) {
                array_name = var_expr->name;
            } else {
                return AWKValue(0.0);  // Error: not a valid array name
            }

            std::string fs = (expr.arguments.size() >= 3)
                           ? evaluate(*expr.arguments[2]).to_string()
                           : env_.FS().to_string();

            // Get array and clear it
            AWKValue& arr = env_.get_variable(array_name);
            arr.array_clear();

            std::vector<std::string> parts;
            if (fs.empty() || fs == " ") {
                // Default behavior: whitespace separation
                std::istringstream iss(str);
                std::string part;
                while (iss >> part) {
                    parts.push_back(part);
                }
            } else if (fs.length() == 1) {
                // Single character separator
                std::string::size_type start = 0;
                std::string::size_type pos;
                while ((pos = str.find(fs, start)) != std::string::npos) {
                    parts.push_back(str.substr(start, pos - start));
                    start = pos + 1;
                }
                parts.push_back(str.substr(start));
            } else {
                // Regex separator - with cache
                try {
                    const std::regex& re = get_cached_regex(fs);
                    std::sregex_token_iterator it(str.begin(), str.end(), re, -1);
                    std::sregex_token_iterator end;
                    for (; it != end; ++it) {
                        parts.push_back(*it);
                    }
                } catch (const std::regex_error& e) {
                    *error_ << "awk: split: invalid regex separator '" << fs << "': " << e.what() << "\n";
                    return AWKValue(0.0);
                }
            }

            // Write parts to array (1-based)
            for (size_t i = 0; i < parts.size(); ++i) {
                arr.array_access(std::to_string(i + 1)) = AWKValue(parts[i]);
            }

            return AWKValue(static_cast<double>(parts.size()));
        }
        return AWKValue(0.0);
    }

    // match() with optional 3rd parameter (array for capturing groups)
    if (expr.function_name == "match") {
        if (expr.arguments.size() >= 2) {
            std::string str = evaluate(*expr.arguments[0]).to_string();
            std::string pattern = evaluate(*expr.arguments[1]).to_string();

            // Optional: array for capturing groups
            std::string array_name;
            if (expr.arguments.size() >= 3) {
                if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[2].get())) {
                    array_name = var_expr->name;
                }
            }

            try {
                const std::regex& re = get_cached_regex(pattern);
                std::smatch match;

                if (std::regex_search(str, match, re)) {
                    int start = static_cast<int>(match.position()) + 1;
                    int length = static_cast<int>(match.length());

                    env_.RSTART() = AWKValue(static_cast<double>(start));
                    env_.RLENGTH() = AWKValue(static_cast<double>(length));

                    // Store capturing groups in array
                    if (!array_name.empty()) {
                        AWKValue& arr = env_.get_variable(array_name);
                        arr.array_clear();
                        for (size_t i = 0; i < match.size(); ++i) {
                            arr.array_access(std::to_string(i)) = AWKValue(match[i].str());
                        }
                    }

                    return AWKValue(static_cast<double>(start));
                } else {
                    env_.RSTART() = AWKValue(0.0);
                    env_.RLENGTH() = AWKValue(-1.0);
                    if (!array_name.empty()) {
                        env_.get_variable(array_name).array_clear();
                    }
                    return AWKValue(0.0);
                }
            } catch (const std::regex_error& e) {
                *error_ << "awk: match: invalid regex '" << pattern << "': " << e.what() << "\n";
                env_.RSTART() = AWKValue(0.0);
                env_.RLENGTH() = AWKValue(-1.0);
                return AWKValue(0.0);
            }
        }
        return AWKValue(0.0);
    }

    // patsplit() with array population
    if (expr.function_name == "patsplit") {
        if (expr.arguments.size() >= 3) {
            std::string str = evaluate(*expr.arguments[0]).to_string();
            std::string pattern = evaluate(*expr.arguments[2]).to_string();

            // Second argument: array for matches
            std::string array_name;
            if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[1].get())) {
                array_name = var_expr->name;
            } else {
                return AWKValue(0.0);
            }

            // Optional 4th parameter: separators array
            std::string seps_name;
            if (expr.arguments.size() >= 4) {
                if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[3].get())) {
                    seps_name = var_expr->name;
                }
            }

            AWKValue& arr = env_.get_variable(array_name);
            arr.array_clear();
            if (!seps_name.empty()) {
                env_.get_variable(seps_name).array_clear();
            }

            try {
                const std::regex& re = get_cached_regex(pattern);
                std::sregex_iterator it(str.begin(), str.end(), re);
                std::sregex_iterator end;

                int count = 0;
                size_t last_end = 0;
                while (it != end) {
                    // Store separator (before this match)
                    if (!seps_name.empty()) {
                        std::string sep = str.substr(last_end, it->position() - last_end);
                        env_.get_variable(seps_name).array_access(std::to_string(count)) = AWKValue(sep);
                    }

                    // Store match (1-based)
                    count++;
                    arr.array_access(std::to_string(count)) = AWKValue(it->str());

                    last_end = it->position() + it->length();
                    ++it;
                }

                // Last separator
                if (!seps_name.empty() && last_end < str.length()) {
                    env_.get_variable(seps_name).array_access(std::to_string(count)) = AWKValue(str.substr(last_end));
                }

                return AWKValue(static_cast<double>(count));
            } catch (const std::regex_error& e) {
                *error_ << "awk: patsplit: invalid regex '" << pattern << "': " << e.what() << "\n";
                return AWKValue(0.0);
            }
        }
        return AWKValue(0.0);
    }

    // asort(source [, dest]) - Sort array by values
    if (expr.function_name == "asort") {
        if (expr.arguments.size() >= 1) {
            // Source array
            std::string source_name;
            if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[0].get())) {
                source_name = var_expr->name;
            } else {
                return AWKValue(0.0);
            }

            // Optional destination array
            std::string dest_name = source_name;  // Default: in-place
            if (expr.arguments.size() >= 2) {
                if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[1].get())) {
                    dest_name = var_expr->name;
                }
            }

            AWKValue& source = env_.get_variable(source_name);
            if (!source.is_array()) return AWKValue(0.0);

            // Collect values
            auto keys = source.array_keys();
            std::vector<AWKValue> values;
            for (const auto& key : keys) {
                const AWKValue* val = source.array_get(key);
                if (val) values.push_back(*val);
            }

            // Sort by string value
            std::sort(values.begin(), values.end(),
                      [](const AWKValue& a, const AWKValue& b) {
                          return a.to_string() < b.to_string();
                      });

            // Write to destination array
            AWKValue& dest = env_.get_variable(dest_name);
            dest.array_clear();
            for (size_t i = 0; i < values.size(); ++i) {
                dest.array_access(std::to_string(i + 1)) = values[i];
            }

            return AWKValue(static_cast<double>(values.size()));
        }
        return AWKValue(0.0);
    }

    // asorti(source [, dest]) - Sort array by indices
    if (expr.function_name == "asorti") {
        if (expr.arguments.size() >= 1) {
            // Source array
            std::string source_name;
            if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[0].get())) {
                source_name = var_expr->name;
            } else {
                return AWKValue(0.0);
            }

            // Optional destination array
            std::string dest_name = source_name;  // Default: in-place
            if (expr.arguments.size() >= 2) {
                if (auto* var_expr = dynamic_cast<VariableExpr*>(expr.arguments[1].get())) {
                    dest_name = var_expr->name;
                }
            }

            AWKValue& source = env_.get_variable(source_name);
            if (!source.is_array()) return AWKValue(0.0);

            // Collect and sort indices
            auto keys = source.array_keys();
            std::sort(keys.begin(), keys.end());

            // Write sorted indices as values to destination array
            AWKValue& dest = env_.get_variable(dest_name);
            dest.array_clear();
            for (size_t i = 0; i < keys.size(); ++i) {
                dest.array_access(std::to_string(i + 1)) = AWKValue(keys[i]);
            }

            return AWKValue(static_cast<double>(keys.size()));
        }
        return AWKValue(0.0);
    }

    // Standard processing for other functions
    std::vector<AWKValue> args;
    for (auto& arg : expr.arguments) {
        args.push_back(evaluate(*arg));
    }

    return call_function(expr.function_name, args);
}

// ============================================================================
// Helper Functions
// ============================================================================

bool Interpreter::is_truthy(const AWKValue& value) {
    return value.to_bool();
}

AWKValue& Interpreter::get_lvalue(Expr& expr) {
    if (auto* var = dynamic_cast<VariableExpr*>(&expr)) {
        return env_.get_variable(var->name);
    }

    if (auto* field = dynamic_cast<FieldExpr*>(&expr)) {
        int index = static_cast<int>(evaluate(*field->index).to_number());
        return get_field(index);
    }

    if (auto* arr = dynamic_cast<ArrayAccessExpr*>(&expr)) {
        std::vector<AWKValue> idx_vals;
        for (auto& idx : arr->indices) {
            idx_vals.push_back(evaluate(*idx));
        }
        std::string key = AWKValue::make_array_key(idx_vals, get_cached_subsep());

        // Special handling for SYMTAB - direct variable access
        if (arr->name == "SYMTAB") {
            return env_.get_variable(key);
        }

        AWKValue& array = env_.get_variable(arr->name);
        return array.array_access(key);
    }

    // Should not happen
    static AWKValue dummy;
    return dummy;
}

void Interpreter::parse_fields() {
    if (!record_dirty_) return;

    fields_.clear();
    fields_.reserve(16);  // Pre-allocate for typical field count

    // Invalidate cached field AWKValues
    std::fill(field_values_valid_.begin(), field_values_valid_.end(), false);

    // FPAT takes precedence over FS (gawk extension)
    // Use cached value for performance
    const std::string& fpat = get_cached_fpat();
    if (!fpat.empty()) {
        // FPAT mode: match fields via regex (not split)
        try {
            const std::regex& re = get_cached_regex(fpat);
            std::sregex_iterator it(current_record_.begin(),
                                    current_record_.end(), re);
            std::sregex_iterator end;
            while (it != end) {
                fields_.push_back(it->str());
                ++it;
            }
        } catch (const std::regex_error& e) {
            // On regex error: report and treat whole record as one field
            *error_ << "awk: FPAT: invalid regex '" << fpat << "': " << e.what() << "\n";
            fields_.push_back(current_record_);
        }
        env_.NF() = AWKValue(static_cast<double>(fields_.size()));
        record_dirty_ = false;
        fields_dirty_ = false;
        return;
    }

    // Use cached FS for performance
    const std::string& fs = get_cached_fs();

    if (fs == " ") {
        // Standard splitting: whitespace, multiple spaces ignored
        std::istringstream iss(current_record_);
        std::string field;
        while (iss >> field) {
            fields_.push_back(std::move(field));
        }
    } else if (fs.length() == 1) {
        // Single character separator - optimized path
        char sep = fs[0];
        std::string::size_type start = 0;
        std::string::size_type pos;

        while ((pos = current_record_.find(sep, start)) != std::string::npos) {
            fields_.push_back(current_record_.substr(start, pos - start));
            start = pos + 1;
        }
        fields_.push_back(current_record_.substr(start));
    } else {
        // Regex separator - with cache
        try {
            const std::regex& re = get_cached_regex(fs);
            std::sregex_token_iterator it(current_record_.begin(),
                                          current_record_.end(),
                                          re, -1);
            std::sregex_token_iterator end;
            while (it != end) {
                fields_.push_back(*it++);
            }
        } catch (const std::regex_error& e) {
            // On regex error: report and treat whole record as one field
            *error_ << "awk: FS: invalid regex '" << fs << "': " << e.what() << "\n";
            fields_.push_back(current_record_);
        }
    }

    env_.NF() = AWKValue(static_cast<double>(fields_.size()));
    record_dirty_ = false;
    fields_dirty_ = false;
}

void Interpreter::rebuild_record() {
    if (!fields_dirty_) return;

    // Use cached OFS for performance
    const std::string& ofs = get_cached_ofs();

    // Pre-calculate total size needed to avoid reallocations
    size_t total_size = 0;
    for (const auto& field : fields_) {
        total_size += field.length();
    }
    if (!fields_.empty()) {
        total_size += ofs.length() * (fields_.size() - 1);
    }

    current_record_.clear();
    current_record_.reserve(total_size);

    for (size_t i = 0; i < fields_.size(); ++i) {
        if (i > 0) {
            current_record_ += ofs;
        }
        current_record_ += fields_[i];
    }

    fields_dirty_ = false;
}

AWKValue& Interpreter::get_field(int index) {
    parse_fields();

    if (index == 0) {
        // $0 = entire record
        rebuild_record();
        field0_ = AWKValue::strnum(current_record_);
        return field0_;
    }

    if (index < 0) {
        empty_field_ = AWKValue("");
        return empty_field_;
    }

    // Extend fields if necessary
    while (static_cast<size_t>(index) > fields_.size()) {
        fields_.push_back("");
    }

    // Ensure field_values_ and validity vectors are large enough
    if (field_values_.size() < fields_.size()) {
        field_values_.resize(fields_.size());
        field_values_valid_.resize(fields_.size(), false);
    }

    // Only create AWKValue if not already cached
    size_t idx = static_cast<size_t>(index - 1);
    if (!field_values_valid_[idx]) {
        field_values_[idx] = AWKValue::strnum(fields_[idx]);
        field_values_valid_[idx] = true;
    }
    return field_values_[idx];
}

void Interpreter::set_field(int index, AWKValue value) {
    parse_fields();

    if (index == 0) {
        current_record_ = value.to_string();
        record_dirty_ = true;
        parse_fields();
        return;
    }

    if (index < 0) return;

    // Extend fields if necessary
    while (static_cast<size_t>(index) > fields_.size()) {
        fields_.push_back("");
    }

    fields_[index - 1] = value.to_string();
    fields_dirty_ = true;

    // Invalidate cached AWKValue for this field
    if (field_values_valid_.size() >= static_cast<size_t>(index)) {
        field_values_valid_[index - 1] = false;
    }

    env_.NF() = AWKValue(static_cast<double>(fields_.size()));
}

void Interpreter::set_record(const std::string& record) {
    current_record_ = record;
    record_dirty_ = true;
    parse_fields();
}

int Interpreter::field_count() const {
    return static_cast<int>(fields_.size());
}

bool Interpreter::regex_match(const AWKValue& text, const AWKValue& pattern) {
    std::string text_str = text.to_string();
    std::string pattern_str = pattern.is_regex() ? pattern.regex_pattern() : pattern.to_string();

    try {
        // Use cached regex - automatically respects IGNORECASE
        const std::regex& re = get_cached_regex(pattern_str);
        return std::regex_search(text_str, re);
    } catch (const std::regex_error& e) {
        *error_ << "awk: invalid regex '" << pattern_str << "': " << e.what() << "\n";
        return false;
    }
}

std::ostream* Interpreter::get_output_stream(const std::string& target,
                                              RedirectType type) {
    // Special files (gawk compatibility)
    if (target == "/dev/stdout" || target == "-") {
        return &std::cout;
    }
    if (target == "/dev/stderr") {
        return &std::cerr;
    }
    if (target == "/dev/null") {
        // Null stream: discard all output
        static std::ofstream null_stream;
        if (!null_stream.is_open()) {
#ifdef _WIN32
            null_stream.open("NUL");
#else
            null_stream.open("/dev/null");
#endif
        }
        return &null_stream;
    }

    if (type == RedirectType::PIPE) {
        // Output pipe: print | "command"
        auto it = output_pipes_.find(target);
        if (it != output_pipes_.end()) {
            return it->second.get();
        }

        // Open new pipe
#ifdef _WIN32
        FILE* pipe = _popen(target.c_str(), "w");
#else
        FILE* pipe = popen(target.c_str(), "w");
#endif

        if (!pipe) {
            *error_ << "awk: can't open pipe to command: " << target << ": " << safe_strerror(errno) << "\n";
            return output_;
        }

        auto pipe_stream = std::make_unique<PipeOStream>(pipe);
        std::ostream* result = pipe_stream.get();
        output_pipes_[target] = std::move(pipe_stream);
        return result;
    }

    if (type == RedirectType::PIPE_BOTH) {
        // Bidirectional pipe: print |& "command" (gawk extension)
        Coprocess* coproc = get_or_create_coprocess(target);
        if (coproc && coproc->to_child) {
            // We need to create a temporary ostream for the FILE*
            // Since we don't have a persistent ostream, we write directly
            // and return a special stream
            // For now: use a PipeOStream-like solution
            auto it = output_pipes_.find("__coproc__" + target);
            if (it != output_pipes_.end()) {
                return it->second.get();
            }
            auto pipe_stream = std::make_unique<PipeOStream>(coproc->to_child);
            std::ostream* result = pipe_stream.get();
            output_pipes_["__coproc__" + target] = std::move(pipe_stream);
            return result;
        }
        *error_ << "awk: can't open coprocess to command: " << target << ": " << safe_strerror(errno) << "\n";
        return output_;
    }

    // File
    auto it = output_files_.find(target);
    if (it != output_files_.end()) {
        return it->second.get();
    }

    std::ios_base::openmode mode = std::ios::out;
    if (type == RedirectType::APPEND) {
        mode |= std::ios::app;
    }

    auto file = std::make_unique<std::ofstream>(target, mode);
    if (!file->is_open()) {
        *error_ << "awk: can't open file " << target << " for output: " << safe_strerror(errno) << "\n";
        return output_;
    }

    std::ostream* result = file.get();
    output_files_[target] = std::move(file);
    return result;
}

AWKValue Interpreter::call_function(const std::string& name,
                                    std::vector<AWKValue>& args) {
    // Check if name is namespace-qualified (contains ::)
    std::string unqualified_name;
    size_t sep_pos = name.find("::");
    if (sep_pos != std::string::npos) {
        unqualified_name = name.substr(sep_pos + 2);
    }

    // Built-in function? (try qualified name first, then unqualified)
    auto builtin = env_.get_builtin(name);
    if (builtin) {
        return builtin(args, *this);
    }

    // If qualified, also check unqualified name for built-ins
    if (!unqualified_name.empty()) {
        builtin = env_.get_builtin(unqualified_name);
        if (builtin) {
            return builtin(args, *this);
        }
    }

    // User-defined function?
    FunctionDef* func = env_.get_function(name);
    if (func) {
        return call_user_function(func, args);
    }

    *error_ << "awk: function " << name << " not defined\n";
    return AWKValue();
}

AWKValue Interpreter::call_user_function(FunctionDef* func,
                                         std::vector<AWKValue>& args) {
    env_.push_scope();

    // Set parameters
    for (size_t i = 0; i < func->parameters.size(); ++i) {
        if (i < args.size()) {
            env_.set_local(func->parameters[i], args[i]);
        } else {
            env_.set_local(func->parameters[i], AWKValue());
        }
    }

    // Extra arguments as local variables (AWK convention for local vars)
    for (size_t i = func->parameters.size(); i < args.size(); ++i) {
        // Ignore - AWK allows more arguments than parameters
    }

    AWKValue result;
    try {
        execute(*func->body);
    } catch (const ReturnException& e) {
        result = e.value;
    }

    env_.pop_scope();
    return result;
}

// Helper: Parse format flags (-+#0 space)
static void parse_format_flags(const std::string& format, size_t& i, std::string& spec) {
    while (i < format.length() &&
           (format[i] == '-' || format[i] == '+' || format[i] == ' ' ||
            format[i] == '#' || format[i] == '0')) {
        spec += format[i++];
    }
}

// Helper: Parse width (number or * for dynamic)
static void parse_format_width(const std::string& format, size_t& i, std::string& spec,
                               const std::vector<AWKValue>& args, size_t& arg_idx) {
    if (i < format.length() && format[i] == '*') {
        // Dynamic width from next argument
        int width = (arg_idx < args.size())
            ? static_cast<int>(args[arg_idx++].to_number()) : 0;
        spec += std::to_string(width);
        i++;
    } else {
        while (i < format.length() && std::isdigit(format[i])) {
            spec += format[i++];
        }
    }
}

// Helper: Parse precision (.number or .* for dynamic)
static void parse_format_precision(const std::string& format, size_t& i, std::string& spec,
                                   const std::vector<AWKValue>& args, size_t& arg_idx) {
    if (i < format.length() && format[i] == '.') {
        spec += format[i++];
        if (i < format.length() && format[i] == '*') {
            // Dynamic precision from next argument
            int prec = (arg_idx < args.size())
                ? static_cast<int>(args[arg_idx++].to_number()) : 0;
            spec += std::to_string(prec);
            i++;
        } else {
            while (i < format.length() && std::isdigit(format[i])) {
                spec += format[i++];
            }
        }
    }
}

// Helper: Format a single value according to conversion specifier
// Optimized version with larger buffer and direct string handling for %s
static std::string format_value(char conv, const std::string& spec, const AWKValue& arg) {
    // Use larger buffer for numeric formats (1024 handles extreme precision)
    char buffer[1024];

    switch (conv) {
        case 'd':
        case 'i':
            std::snprintf(buffer, sizeof(buffer), spec.c_str(),
                         static_cast<long long>(arg.to_number()));
            return buffer;
        case 'o':
        case 'x':
        case 'X':
        case 'u':
            std::snprintf(buffer, sizeof(buffer), spec.c_str(),
                         static_cast<unsigned long long>(arg.to_number()));
            return buffer;
        case 'e':
        case 'E':
        case 'f':
        case 'F':
        case 'g':
        case 'G':
            std::snprintf(buffer, sizeof(buffer), spec.c_str(), arg.to_number());
            return buffer;
        case 'c': {
            std::string s = arg.to_string();
            if (!s.empty()) {
                return std::string(1, s[0]);
            }
            return "";
        }
        case 's': {
            // For strings, use dynamic allocation to handle any length
            std::string str = arg.to_string();
            // Check if we need special formatting (width/precision)
            if (spec == "%s") {
                return str;  // Fast path: no formatting needed
            }
            // Calculate required buffer size (string length + potential padding)
            size_t required = str.length() + 256;
            std::vector<char> dyn_buffer(required);
            std::snprintf(dyn_buffer.data(), required, spec.c_str(), str.c_str());
            return dyn_buffer.data();
        }
        default:
            return std::string(1, conv);
    }
}

std::string Interpreter::do_sprintf(const std::string& format,
                                    const std::vector<AWKValue>& args) {
    std::string result;
    // Pre-allocate: estimate 2x format length + average arg contribution
    result.reserve(format.length() * 2 + args.size() * 16);
    size_t arg_idx = 0;
    size_t i = 0;

    while (i < format.length()) {
        if (format[i] != '%') {
            result += format[i++];
            continue;
        }

        i++;  // Skip %

        if (i >= format.length()) {
            result += '%';
            break;
        }

        if (format[i] == '%') {
            result += '%';
            i++;
            continue;
        }

        // Build format specification
        std::string spec = "%";
        parse_format_flags(format, i, spec);
        parse_format_width(format, i, spec, args, arg_idx);
        parse_format_precision(format, i, spec, args, arg_idx);

        // Get conversion specifier
        if (i >= format.length()) break;
        char conv = format[i++];
        spec += conv;

        // Get argument and format it
        AWKValue arg = (arg_idx < args.size()) ? args[arg_idx++] : AWKValue();
        result += format_value(conv, spec, arg);
    }

    return result;
}

// ============================================================================
// Register Built-in Functions
// ============================================================================

void Interpreter::register_builtins() {
    register_math_builtins();
    register_string_builtins();
    register_io_builtins();
    register_time_builtins();
    register_bit_builtins();
    register_type_builtins();
}

// ============================================================================
// File Management Methods
// ============================================================================

bool Interpreter::close_file(const std::string& filename) {
    // Try to close output file
    auto out_it = output_files_.find(filename);
    if (out_it != output_files_.end()) {
        out_it->second->close();
        output_files_.erase(out_it);
        return true;
    }

    // Try to close input file
    auto in_it = input_files_.find(filename);
    if (in_it != input_files_.end()) {
        in_it->second->close();
        input_files_.erase(in_it);
        return true;
    }

    // Try to close input pipe
    auto in_pipe_it = input_pipes_.find(filename);
    if (in_pipe_it != input_pipes_.end()) {
#ifdef _WIN32
        _pclose(in_pipe_it->second);
#else
        pclose(in_pipe_it->second);
#endif
        input_pipes_.erase(in_pipe_it);
        return true;
    }

    // Try to close output pipe
    auto out_pipe_it = output_pipes_.find(filename);
    if (out_pipe_it != output_pipes_.end()) {
        out_pipe_it->second->close_pipe();
        output_pipes_.erase(out_pipe_it);
        return true;
    }

    // Try to close coprocess (gawk |& extension)
    if (close_coprocess(filename)) {
        return true;
    }

    return false;
}

bool Interpreter::flush_file(const std::string& filename) {
    if (filename.empty()) {
        std::cout.flush();
        return true;
    }

    // Flush output file
    auto file_it = output_files_.find(filename);
    if (file_it != output_files_.end()) {
        file_it->second->flush();
        return true;
    }

    // Flush output pipe
    auto pipe_it = output_pipes_.find(filename);
    if (pipe_it != output_pipes_.end()) {
        pipe_it->second->flush();
        return true;
    }

    // Flush coprocess (gawk |& extension)
    auto coproc_it = coprocesses_.find(filename);
    if (coproc_it != coprocesses_.end() && coproc_it->second->to_child) {
        fflush(coproc_it->second->to_child);
        return true;
    }

    return false;
}

void Interpreter::flush_all_files() {
    std::cout.flush();
    std::cerr.flush();
    for (auto& [name, file] : output_files_) {
        file->flush();
    }
    for (auto& [name, pipe] : output_pipes_) {
        pipe->flush();
    }
    // Flush coprocesses (gawk |& extension)
    for (auto& [name, coproc] : coprocesses_) {
        if (coproc->to_child) {
            fflush(coproc->to_child);
        }
    }
}

// ============================================================================
// i18n (Internationalization) - bindtextdomain
// ============================================================================

std::string Interpreter::bind_textdomain(const std::string& domain, const std::string& directory) {
    if (domain.empty()) {
        return "";
    }
    // Use the I18n singleton for actual binding
    return I18n::instance().bindtextdomain(domain, directory);
}

std::string Interpreter::get_textdomain_directory(const std::string& domain) const {
    return I18n::instance().get_textdomain_directory(domain);
}

void Interpreter::cleanup_io() {
    // Close output files
    output_files_.clear();

    // Close input files
    input_files_.clear();

    // Close input pipes
    for (auto& [cmd, pipe] : input_pipes_) {
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
    }
    input_pipes_.clear();

    // Close output pipes (automatic via PipeOStream destructor)
    output_pipes_.clear();

    // Close coprocesses (gawk |& extension)
    coprocesses_.clear();
}

} // namespace awk
