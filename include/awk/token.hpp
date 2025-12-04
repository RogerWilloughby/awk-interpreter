#ifndef AWK_TOKEN_HPP
#define AWK_TOKEN_HPP

#include <string>
#include <variant>
#include <ostream>

namespace awk {

enum class TokenType {
    // Literals
    NUMBER,
    STRING,
    REGEX,

    // Identifiers
    IDENTIFIER,

    // Keywords
    BEGIN_KW,
    END_KW,
    BEGINFILE_KW,
    ENDFILE_KW,
    IF,
    ELSE,
    WHILE,
    DO,
    FOR,
    IN,
    BREAK,
    CONTINUE,
    NEXT,
    NEXTFILE,
    EXIT,
    RETURN,
    FUNCTION,
    DELETE,
    PRINT,
    PRINTF,
    GETLINE,
    SWITCH,
    CASE,
    DEFAULT,

    // Arithmetic operators
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PERCENT,        // %
    CARET,          // ^

    // Comparison operators
    EQ,             // ==
    NE,             // !=
    LT,             // <
    GT,             // >
    LE,             // <=
    GE,             // >=

    // Regex operators
    MATCH,          // ~
    NOT_MATCH,      // !~

    // Logical operators
    AND,            // &&
    OR,             // ||
    NOT,            // !

    // Assignment operators
    ASSIGN,         // =
    PLUS_ASSIGN,    // +=
    MINUS_ASSIGN,   // -=
    STAR_ASSIGN,    // *=
    SLASH_ASSIGN,   // /=
    PERCENT_ASSIGN, // %=
    CARET_ASSIGN,   // ^=

    // Increment/Decrement
    INCREMENT,      // ++
    DECREMENT,      // --

    // Ternary operator
    QUESTION,       // ?
    COLON,          // :

    // Separators
    COMMA,          // ,
    SEMICOLON,      // ;
    NEWLINE,        // \n (semantically relevant in AWK)

    // Brackets
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]

    // Special operators
    DOLLAR,         // $
    PIPE,           // |
    APPEND,         // >>
    PIPE_BOTH,      // |&

    // Directives (gawk extension)
    AT_INCLUDE,     // @include
    AT_NAMESPACE,   // @namespace
    AT,             // @ (for indirect function calls: @varname(args))
    COLON_COLON,    // :: (namespace separator)

    // Special tokens
    END_OF_FILE,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::variant<std::monostate, double, std::string> literal;
    size_t line;
    size_t column;

    Token() : type(TokenType::ERROR), line(0), column(0) {}

    Token(TokenType t, std::string lex, size_t l, size_t c)
        : type(t), lexeme(std::move(lex)), line(l), column(c) {}

    Token(TokenType t, std::string lex, double num, size_t l, size_t c)
        : type(t), lexeme(std::move(lex)), literal(num), line(l), column(c) {}

    Token(TokenType t, std::string lex, std::string str, size_t l, size_t c)
        : type(t), lexeme(std::move(lex)), literal(std::move(str)), line(l), column(c) {}

    bool is_assignment_op() const {
        return type == TokenType::ASSIGN ||
               type == TokenType::PLUS_ASSIGN ||
               type == TokenType::MINUS_ASSIGN ||
               type == TokenType::STAR_ASSIGN ||
               type == TokenType::SLASH_ASSIGN ||
               type == TokenType::PERCENT_ASSIGN ||
               type == TokenType::CARET_ASSIGN;
    }

    bool is_comparison_op() const {
        return type == TokenType::EQ ||
               type == TokenType::NE ||
               type == TokenType::LT ||
               type == TokenType::GT ||
               type == TokenType::LE ||
               type == TokenType::GE;
    }
};

// Token type to string for debugging
inline const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::REGEX: return "REGEX";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::BEGIN_KW: return "BEGIN";
        case TokenType::END_KW: return "END";
        case TokenType::BEGINFILE_KW: return "BEGINFILE";
        case TokenType::ENDFILE_KW: return "ENDFILE";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::DO: return "DO";
        case TokenType::FOR: return "FOR";
        case TokenType::IN: return "IN";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::NEXT: return "NEXT";
        case TokenType::NEXTFILE: return "NEXTFILE";
        case TokenType::EXIT: return "EXIT";
        case TokenType::RETURN: return "RETURN";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::DELETE: return "DELETE";
        case TokenType::PRINT: return "PRINT";
        case TokenType::PRINTF: return "PRINTF";
        case TokenType::GETLINE: return "GETLINE";
        case TokenType::SWITCH: return "SWITCH";
        case TokenType::CASE: return "CASE";
        case TokenType::DEFAULT: return "DEFAULT";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::CARET: return "CARET";
        case TokenType::EQ: return "EQ";
        case TokenType::NE: return "NE";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LE: return "LE";
        case TokenType::GE: return "GE";
        case TokenType::MATCH: return "MATCH";
        case TokenType::NOT_MATCH: return "NOT_MATCH";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS_ASSIGN: return "PLUS_ASSIGN";
        case TokenType::MINUS_ASSIGN: return "MINUS_ASSIGN";
        case TokenType::STAR_ASSIGN: return "STAR_ASSIGN";
        case TokenType::SLASH_ASSIGN: return "SLASH_ASSIGN";
        case TokenType::PERCENT_ASSIGN: return "PERCENT_ASSIGN";
        case TokenType::CARET_ASSIGN: return "CARET_ASSIGN";
        case TokenType::INCREMENT: return "INCREMENT";
        case TokenType::DECREMENT: return "DECREMENT";
        case TokenType::QUESTION: return "QUESTION";
        case TokenType::COLON: return "COLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::DOLLAR: return "DOLLAR";
        case TokenType::PIPE: return "PIPE";
        case TokenType::APPEND: return "APPEND";
        case TokenType::PIPE_BOTH: return "PIPE_BOTH";
        case TokenType::AT_INCLUDE: return "AT_INCLUDE";
        case TokenType::AT_NAMESPACE: return "AT_NAMESPACE";
        case TokenType::AT: return "AT";
        case TokenType::COLON_COLON: return "COLON_COLON";
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "Token(" << token_type_to_string(token.type)
       << ", \"" << token.lexeme << "\", "
       << token.line << ":" << token.column << ")";
    return os;
}

} // namespace awk

#endif // AWK_TOKEN_HPP
