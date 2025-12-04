#include "awk/lexer.hpp"
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

namespace awk {

// Keyword table
const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"BEGIN", TokenType::BEGIN_KW},
    {"END", TokenType::END_KW},
    {"BEGINFILE", TokenType::BEGINFILE_KW},
    {"ENDFILE", TokenType::ENDFILE_KW},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"do", TokenType::DO},
    {"for", TokenType::FOR},
    {"in", TokenType::IN},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"next", TokenType::NEXT},
    {"nextfile", TokenType::NEXTFILE},
    {"exit", TokenType::EXIT},
    {"return", TokenType::RETURN},
    {"function", TokenType::FUNCTION},
    {"func", TokenType::FUNCTION},  // Short form
    {"delete", TokenType::DELETE},
    {"print", TokenType::PRINT},
    {"printf", TokenType::PRINTF},
    {"getline", TokenType::GETLINE},
    {"switch", TokenType::SWITCH},
    {"case", TokenType::CASE},
    {"default", TokenType::DEFAULT}
};

Lexer::Lexer(std::string source)
    : source_(std::move(source)), expect_regex_(true) {
    // At the start of the program, a regex pattern may appear
}

Token Lexer::next_token() {
    if (has_peeked_) {
        has_peeked_ = false;
        Token result = peeked_token_;
        // After certain tokens we may expect a regex
        update_regex_expectation(result.type);
        return result;
    }
    Token result = scan_token();
    // After certain tokens we may expect a regex
    update_regex_expectation(result.type);
    return result;
}

void Lexer::update_regex_expectation(TokenType last_type) {
    // After these tokens, / should be interpreted as regex start
    switch (last_type) {
        case TokenType::MATCH:       // ~
        case TokenType::NOT_MATCH:   // !~
        case TokenType::LPAREN:      // (
        case TokenType::COMMA:       // ,
        case TokenType::LBRACE:      // {
        case TokenType::SEMICOLON:   // ;
        case TokenType::NEWLINE:     // Start of line
        case TokenType::RETURN:      // return
        case TokenType::PRINT:       // print
        case TokenType::PRINTF:      // printf
        case TokenType::IF:          // if
        case TokenType::WHILE:       // while
        case TokenType::FOR:         // for
        case TokenType::DO:          // do
        case TokenType::ASSIGN:      // =
        case TokenType::PLUS_ASSIGN:
        case TokenType::MINUS_ASSIGN:
        case TokenType::STAR_ASSIGN:
        case TokenType::SLASH_ASSIGN:
        case TokenType::PERCENT_ASSIGN:
        case TokenType::CARET_ASSIGN:
        case TokenType::NOT:         // !
        case TokenType::AND:         // &&
        case TokenType::OR:          // ||
        case TokenType::QUESTION:    // ?
        case TokenType::COLON:       // :
            expect_regex_ = true;
            break;
        default:
            expect_regex_ = false;
            break;
    }
}

Token Lexer::peek_token() {
    if (!has_peeked_) {
        peeked_token_ = scan_token();

        // start_ now points to the beginning of the token (after skip_whitespace)
        peeked_start_pos_ = start_;
        peeked_line_ = peeked_token_.line;
        peeked_column_ = peeked_token_.column;
        has_peeked_ = true;
    }
    return peeked_token_;
}

bool Lexer::is_at_end() const {
    return current_ >= source_.length();
}

Token Lexer::scan_token() {
    skip_whitespace();

    start_ = current_;

    if (is_at_end()) {
        return make_token(TokenType::END_OF_FILE);
    }

    char c = advance();

    // Identifiers and keywords
    if (is_alpha(c) || c == '_') {
        return scan_identifier();
    }

    // Numbers
    if (is_digit(c)) {
        return scan_number();
    }

    // Dot followed by digit is also a number
    if (c == '.' && is_digit(peek())) {
        return scan_number();
    }

    switch (c) {
        // Strings
        case '"':
            return scan_string();

        // Regex (if expected)
        case '/':
            if (expect_regex_) {
                expect_regex_ = false;
                return scan_regex();
            }
            if (match('=')) return make_token(TokenType::SLASH_ASSIGN);
            return make_token(TokenType::SLASH);

        // Single-character tokens
        case '(': return make_token(TokenType::LPAREN);
        case ')': return make_token(TokenType::RPAREN);
        case '{': return make_token(TokenType::LBRACE);
        case '}': return make_token(TokenType::RBRACE);
        case '[': return make_token(TokenType::LBRACKET);
        case ']': return make_token(TokenType::RBRACKET);
        case ',': return make_token(TokenType::COMMA);
        case ';': return make_token(TokenType::SEMICOLON);
        case '$': return make_token(TokenType::DOLLAR);
        case '?': return make_token(TokenType::QUESTION);
        case ':':
            if (match(':')) return make_token(TokenType::COLON_COLON);
            return make_token(TokenType::COLON);
        case '^':
            if (match('=')) return make_token(TokenType::CARET_ASSIGN);
            return make_token(TokenType::CARET);
        case '~': return make_token(TokenType::MATCH);

        // Newline is semantically relevant in AWK
        case '\n':
            line_++;
            column_ = 1;
            return make_token(TokenType::NEWLINE);

        // Two- or single-character tokens
        case '+':
            if (match('+')) return make_token(TokenType::INCREMENT);
            if (match('=')) return make_token(TokenType::PLUS_ASSIGN);
            return make_token(TokenType::PLUS);

        case '-':
            if (match('-')) return make_token(TokenType::DECREMENT);
            if (match('=')) return make_token(TokenType::MINUS_ASSIGN);
            return make_token(TokenType::MINUS);

        case '*':
            if (match('*')) {
                // ** ist auch Exponentiation (gawk)
                if (match('=')) return make_token(TokenType::CARET_ASSIGN);
                return make_token(TokenType::CARET);
            }
            if (match('=')) return make_token(TokenType::STAR_ASSIGN);
            return make_token(TokenType::STAR);

        case '%':
            if (match('=')) return make_token(TokenType::PERCENT_ASSIGN);
            return make_token(TokenType::PERCENT);

        case '=':
            if (match('=')) return make_token(TokenType::EQ);
            return make_token(TokenType::ASSIGN);

        case '!':
            if (match('=')) return make_token(TokenType::NE);
            if (match('~')) return make_token(TokenType::NOT_MATCH);
            return make_token(TokenType::NOT);

        case '<':
            if (match('=')) return make_token(TokenType::LE);
            return make_token(TokenType::LT);

        case '>':
            if (match('>')) return make_token(TokenType::APPEND);
            if (match('=')) return make_token(TokenType::GE);
            return make_token(TokenType::GT);

        case '&':
            if (match('&')) return make_token(TokenType::AND);
            // Single & is not valid in AWK, but we allow it for |&
            return error_token("Unexpected character '&'");

        case '|':
            if (match('|')) return make_token(TokenType::OR);
            if (match('&')) return make_token(TokenType::PIPE_BOTH);
            return make_token(TokenType::PIPE);

        case '#':
            // Comment until end of line
            skip_line_comment();
            return scan_token();  // Next token after comment

        case '\\':
            // Backslash-newline is line continuation
            if (peek() == '\n') {
                advance();  // Consume newline
                line_++;
                column_ = 1;
                return scan_token();  // Next token
            }
            if (peek() == '\r' && peek_next() == '\n') {
                advance();  // \r
                advance();  // \n
                line_++;
                column_ = 1;
                return scan_token();
            }
            return error_token("Unexpected backslash");

        case '@':
            // gawk @-directives
            return scan_at_directive();

        default: {
            std::string msg = "Unexpected character '";
            msg += c;
            msg += "' (ASCII ";
            msg += std::to_string(static_cast<unsigned char>(c));
            msg += ")";
            return error_token(msg);
        }
    }
}

Token Lexer::scan_string() {
    std::string value;
    size_t start_line = line_;

    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') {
            return error_token("Unterminated string (started at line " + std::to_string(start_line) + ")");
        }
        if (peek() == '\\') {
            advance();  // Backslash
            if (is_at_end()) {
                return error_token("Unterminated string (started at line " + std::to_string(start_line) + ")");
            }
            char escaped = advance();
            value += escape_char(escaped);
        } else {
            value += advance();
        }
    }

    if (is_at_end()) {
        return error_token("Unterminated string (started at line " + std::to_string(start_line) + ")");
    }

    advance();  // Closing "
    return make_token(TokenType::STRING, value);
}

Token Lexer::scan_regex() {
    std::string pattern;
    size_t start_line = line_;

    while (peek() != '/' && !is_at_end()) {
        if (peek() == '\n') {
            return error_token("Unterminated regex (started at line " + std::to_string(start_line) + ")");
        }
        if (peek() == '\\') {
            pattern += advance();  // Backslash
            if (!is_at_end() && peek() != '\n') {
                pattern += advance();  // Escaped character
            }
        } else {
            pattern += advance();
        }
    }

    if (is_at_end()) {
        return error_token("Unterminated regex (started at line " + std::to_string(start_line) + ")");
    }

    advance();  // Closing /
    return make_token(TokenType::REGEX, pattern);
}

Token Lexer::scan_number() {
    // Back to the first character of the number
    if (source_[current_ - 1] == '.') {
        // Started with decimal point
    } else {
        // Hexadecimal?
        if (source_[current_ - 1] == '0' && (peek() == 'x' || peek() == 'X')) {
            advance();  // x/X
            while (is_hex_digit(peek())) {
                advance();
            }
            std::string hex_str = source_.substr(start_, current_ - start_);
            double value = static_cast<double>(std::strtoll(hex_str.c_str(), nullptr, 16));
            return make_token(TokenType::NUMBER, value);
        }

        // Octal?
        if (source_[current_ - 1] == '0' && is_octal_digit(peek())) {
            while (is_octal_digit(peek())) {
                advance();
            }
            std::string oct_str = source_.substr(start_, current_ - start_);
            double value = static_cast<double>(std::strtoll(oct_str.c_str(), nullptr, 8));
            return make_token(TokenType::NUMBER, value);
        }
    }

    // Decimal number
    while (is_digit(peek())) {
        advance();
    }

    // Decimal point?
    if (peek() == '.' && is_digit(peek_next())) {
        advance();  // .
        while (is_digit(peek())) {
            advance();
        }
    } else if (peek() == '.' && source_[current_ - 1] != '.') {
        // Decimal point without following digit, but preceded by digits
        advance();  // Accept decimal point
        while (is_digit(peek())) {
            advance();
        }
    }

    // Exponent?
    if (peek() == 'e' || peek() == 'E') {
        size_t exp_start = current_;
        advance();  // e/E
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        if (!is_digit(peek())) {
            // Not a valid exponent, go back
            current_ = exp_start;
        } else {
            while (is_digit(peek())) {
                advance();
            }
        }
    }

    std::string num_str = source_.substr(start_, current_ - start_);
    double value = std::strtod(num_str.c_str(), nullptr);
    return make_token(TokenType::NUMBER, value);
}

Token Lexer::scan_identifier() {
    while (is_alnum(peek()) || peek() == '_') {
        advance();
    }

    std::string text = source_.substr(start_, current_ - start_);

    // Check for keyword
    auto it = keywords_.find(text);
    if (it != keywords_.end()) {
        return make_token(it->second);
    }

    return make_token(TokenType::IDENTIFIER);
}

Token Lexer::scan_at_directive() {
    // @-directives begin with @ followed by an identifier
    while (is_alnum(peek()) || peek() == '_') {
        advance();
    }

    std::string text = source_.substr(start_, current_ - start_);

    if (text == "@include") {
        return make_token(TokenType::AT_INCLUDE);
    }

    if (text == "@namespace") {
        return make_token(TokenType::AT_NAMESPACE);
    }

    // @ alone or @identifier for indirect function calls
    if (text == "@") {
        // Just @ without identifier - return AT token
        return make_token(TokenType::AT);
    }

    // @identifier - return AT token, identifier is parsed separately
    // Set current_ back to after the @, so identifier comes as next token
    current_ = start_ + 1;  // Position after @
    return make_token(TokenType::AT);
}

char Lexer::advance() {
    char c = source_[current_++];
    column_++;
    return c;
}

char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[current_];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (is_at_end()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

void Lexer::skip_whitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance();
                break;
            default:
                return;
        }
    }
}

void Lexer::skip_line_comment() {
    while (peek() != '\n' && !is_at_end()) {
        advance();
    }
}

Token Lexer::make_token(TokenType type) {
    std::string lexeme = source_.substr(start_, current_ - start_);
    return Token(type, lexeme, line_, column_ - lexeme.length());
}

Token Lexer::make_token(TokenType type, double value) {
    std::string lexeme = source_.substr(start_, current_ - start_);
    return Token(type, lexeme, value, line_, column_ - lexeme.length());
}

Token Lexer::make_token(TokenType type, const std::string& str_value) {
    std::string lexeme = source_.substr(start_, current_ - start_);
    return Token(type, lexeme, str_value, line_, column_ - lexeme.length());
}

Token Lexer::error_token(const std::string& message) {
    return Token(TokenType::ERROR, message, line_, column_);
}

bool Lexer::is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

bool Lexer::is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

bool Lexer::is_hex_digit(char c) {
    return is_digit(c) ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

bool Lexer::is_octal_digit(char c) {
    return c >= '0' && c <= '7';
}

char Lexer::escape_char(char c) {
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'a': return '\a';
        case 'v': return '\v';
        case '\\': return '\\';
        case '"': return '"';
        case '/': return '/';
        case '0': return '\0';
        default: return c;  // Unknown escape sequence: return character itself
    }
}

std::string Lexer::process_escapes(const std::string& str) {
    std::string result;
    result.reserve(str.length());

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            result += escape_char(str[++i]);
        } else {
            result += str[i];
        }
    }
    return result;
}

} // namespace awk
