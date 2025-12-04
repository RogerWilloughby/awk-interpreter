#ifndef AWK_PARSER_HPP
#define AWK_PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"
#include <memory>
#include <vector>
#include <string>
#include <set>
#include <functional>
#include <fstream>
#include <sstream>

namespace awk {

class Parser {
public:
    explicit Parser(Lexer& lexer);

    // Constructor with include tracking for recursive includes
    Parser(Lexer& lexer, std::set<std::string>& included_files,
           const std::string& current_file, const std::string& base_path);

    // Main method: Parses the entire program
    std::unique_ptr<Program> parse();

    // Static method to parse a file with include support
    static std::unique_ptr<Program> parse_file(const std::string& filename);

    // Static method to parse a string with include support
    static std::unique_ptr<Program> parse_string(const std::string& source,
                                                  const std::string& base_path = "");

    // Error access
    bool had_error() const { return had_error_; }
    const std::vector<std::string>& errors() const { return errors_; }

private:
    Lexer& lexer_;
    Token current_;
    Token previous_;
    std::vector<std::string> errors_;
    bool had_error_ = false;
    bool panic_mode_ = false;

    // Include tracking (gawk extension)
    std::set<std::string>* included_files_ = nullptr;  // Externally managed for recursive includes
    std::set<std::string> local_included_files_;       // Local when no external set provided
    std::string current_file_;                          // Currently parsed file
    std::string base_path_;                             // Base directory for relative includes

    // Token management
    void advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);
    bool is_at_end() const;
    bool peek_next_is(TokenType type);  // Peek at token after current

    // Newline-Handling (AWK-spezifisch)
    void skip_newlines();
    void skip_optional_newlines();
    bool is_statement_terminator() const;
    void consume_statement_terminator();

    // Error handling
    void error(const std::string& message);
    void error_at_current(const std::string& message);
    void synchronize();

    // ========================================================================
    // Program Structure
    // ========================================================================
    std::unique_ptr<Program> program();
    std::unique_ptr<Rule> rule();
    std::unique_ptr<FunctionDef> function_definition();
    Pattern parse_pattern();

    // ========================================================================
    // Statements
    // ========================================================================
    StmtPtr statement();
    StmtPtr declaration_or_statement();
    StmtPtr block();
    StmtPtr if_statement();
    StmtPtr while_statement();
    StmtPtr do_while_statement();
    StmtPtr for_statement();
    StmtPtr switch_statement();
    StmtPtr print_statement();
    StmtPtr printf_statement();
    StmtPtr delete_statement();
    StmtPtr expression_statement();
    StmtPtr simple_statement();  // For for-init

    // ========================================================================
    // Expressions (precedence parsing)
    // ========================================================================
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr ternary();
    ExprPtr logical_or();
    ExprPtr logical_and();
    ExprPtr array_membership();  // in
    ExprPtr regex_match();       // ~ !~
    ExprPtr comparison();
    ExprPtr pipe_getline();      // cmd | getline, cmd |& getline
    ExprPtr concatenation();
    ExprPtr addition();
    ExprPtr multiplication();
    ExprPtr power();
    ExprPtr unary();
    ExprPtr postfix();
    ExprPtr primary();
    ExprPtr getline_expression();
    ExprPtr field_expression();

    // ========================================================================
    // Helper Methods
    // ========================================================================
    ExprPtr finish_call(const std::string& name);
    ExprPtr finish_array_access(const std::string& name);
    std::pair<ExprPtr, RedirectType> parse_output_redirect();

    // Checks if an expression can be used as an lvalue
    bool is_lvalue(const Expr* expr) const;

    // ========================================================================
    // @include Processing (gawk extension)
    // ========================================================================
    void process_include(std::unique_ptr<Program>& prog);
    std::string resolve_include_path(const std::string& filename) const;
    static std::string read_file(const std::string& filename);
    static std::string get_directory(const std::string& filepath);
    static std::string normalize_path(const std::string& path);
    std::set<std::string>& get_included_files();

    // ========================================================================
    // @namespace Processing (gawk extension)
    // ========================================================================
    void process_namespace();
    std::string current_namespace_;  // Current namespace (empty = default "awk")
    std::string qualify_name(const std::string& name) const;  // Qualify name with namespace
};

} // namespace awk

#endif // AWK_PARSER_HPP
