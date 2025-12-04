#ifndef AWK_LEXER_HPP
#define AWK_LEXER_HPP

#include "token.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace awk {

class Lexer {
public:
    explicit Lexer(std::string source);

    // Main scan method
    Token next_token();

    // Peek without consuming
    Token peek_token();

    // Status queries
    bool is_at_end() const;
    size_t current_line() const { return line_; }
    size_t current_column() const { return column_; }

    // For regex context: Parser can inform Lexer that a regex is expected
    void expect_regex() { expect_regex_ = true; }

    // Re-scan peeked SLASH as regex (for regex context switch)
    void rescan_current_slash() {
        if (has_peeked_ && peeked_token_.type == TokenType::SLASH) {
            // Reset position to the start of the SLASH
            current_ = peeked_start_pos_;
            line_ = peeked_line_;
            column_ = peeked_column_;
            has_peeked_ = false;
            expect_regex_ = true;
        }
    }

    // Scan as regex from the specified position
    // Returns a token (REGEX or ERROR)
    Token scan_regex_from(size_t pos, size_t line, size_t column) {
        current_ = pos;
        line_ = line;
        column_ = column;
        start_ = pos;
        has_peeked_ = false;
        expect_regex_ = true;
        return scan_token();  // Dies wird jetzt Regex scannen weil expect_regex_ = true
    }

private:
    std::string source_;
    size_t start_ = 0;
    size_t current_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;

    bool expect_regex_ = false;  // Parser hint for regex vs division
    Token peeked_token_;
    bool has_peeked_ = false;
    size_t peeked_start_pos_ = 0;  // Position of peeked token for re-scan
    size_t peeked_line_ = 1;       // Line of peeked token
    size_t peeked_column_ = 1;     // Column of peeked token

    // Keyword table
    static const std::unordered_map<std::string, TokenType> keywords_;

    // Main scanner
    Token scan_token();

    // Token creation
    Token make_token(TokenType type);
    Token make_token(TokenType type, double value);
    Token make_token(TokenType type, const std::string& str_value);
    Token error_token(const std::string& message);

    // Specific scanners
    Token scan_string();
    Token scan_regex();
    Token scan_number();
    Token scan_identifier();
    Token scan_at_directive();

    // Character operations
    char advance();
    char peek() const;
    char peek_next() const;
    bool match(char expected);

    // Regex expectation based on last token
    void update_regex_expectation(TokenType last_type);

    // Skipping
    void skip_whitespace();
    void skip_line_comment();
    bool skip_newlines_in_context();

    // Character classification
    static bool is_digit(char c);
    static bool is_alpha(char c);
    static bool is_alnum(char c);
    static bool is_hex_digit(char c);
    static bool is_octal_digit(char c);

    // Process escape sequences in strings
    std::string process_escapes(const std::string& str);
    char escape_char(char c);
};

} // namespace awk

#endif // AWK_LEXER_HPP
