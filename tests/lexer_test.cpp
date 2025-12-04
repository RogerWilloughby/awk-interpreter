// Lexer Unit Tests
#include "test_framework.hpp"
#include "awk/lexer.hpp"

using namespace awk;
using namespace test;

// ============================================================================
// Basic Token Tests
// ============================================================================

TEST(Lexer_Empty_Source) {
    Lexer lexer("");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::END_OF_FILE);
}

TEST(Lexer_Whitespace_Only) {
    Lexer lexer("   \t   ");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::END_OF_FILE);
}

TEST(Lexer_Newlines) {
    Lexer lexer("\n\n\n");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NEWLINE);
}

// ============================================================================
// Number Tokens
// ============================================================================

TEST(Lexer_Integer) {
    Lexer lexer("42");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_DOUBLE_EQ(std::get<double>(tok.literal), 42.0);
}

TEST(Lexer_Float) {
    Lexer lexer("3.14159");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_TRUE(std::abs(std::get<double>(tok.literal) - 3.14159) < 0.00001);
}

TEST(Lexer_Scientific_Notation) {
    Lexer lexer("1.5e10");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_DOUBLE_EQ(std::get<double>(tok.literal), 1.5e10);
}

TEST(Lexer_Negative_Exponent) {
    Lexer lexer("2.5e-3");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_TRUE(std::abs(std::get<double>(tok.literal) - 0.0025) < 0.000001);
}

TEST(Lexer_Hex_Number) {
    Lexer lexer("0x1F");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_DOUBLE_EQ(std::get<double>(tok.literal), 31.0);
}

TEST(Lexer_Octal_Number) {
    Lexer lexer("017");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_DOUBLE_EQ(std::get<double>(tok.literal), 15.0);
}

// ============================================================================
// String Tokens
// ============================================================================

TEST(Lexer_Simple_String) {
    Lexer lexer("\"hello\"");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::STRING);
    ASSERT_EQ(std::get<std::string>(tok.literal), "hello");
}

TEST(Lexer_Empty_String) {
    Lexer lexer("\"\"");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::STRING);
    ASSERT_EQ(std::get<std::string>(tok.literal), "");
}

TEST(Lexer_String_With_Escapes) {
    Lexer lexer("\"hello\\nworld\"");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::STRING);
    ASSERT_EQ(std::get<std::string>(tok.literal), "hello\nworld");
}

TEST(Lexer_String_With_Tab) {
    Lexer lexer("\"a\\tb\"");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::STRING);
    ASSERT_EQ(std::get<std::string>(tok.literal), "a\tb");
}

// ============================================================================
// Regex Tokens
// ============================================================================

TEST(Lexer_Simple_Regex) {
    Lexer lexer("/hello/");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::REGEX);
    ASSERT_EQ(std::get<std::string>(tok.literal), "hello");
}

TEST(Lexer_Regex_With_Pattern) {
    Lexer lexer("/^[a-z]+$/");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::REGEX);
    ASSERT_EQ(std::get<std::string>(tok.literal), "^[a-z]+$");
}

TEST(Lexer_Regex_With_Escaped_Slash) {
    Lexer lexer("/a\\/b/");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::REGEX);
    // The lexer stores the escape sequence unchanged, as the regex engine interprets it
    ASSERT_EQ(std::get<std::string>(tok.literal), "a\\/b");
}

// ============================================================================
// Operators
// ============================================================================

TEST(Lexer_Arithmetic_Operators) {
    Lexer lexer("+ - * / % ^");
    ASSERT_EQ(lexer.next_token().type, TokenType::PLUS);
    ASSERT_EQ(lexer.next_token().type, TokenType::MINUS);
    ASSERT_EQ(lexer.next_token().type, TokenType::STAR);
    ASSERT_EQ(lexer.next_token().type, TokenType::SLASH);
    ASSERT_EQ(lexer.next_token().type, TokenType::PERCENT);
    ASSERT_EQ(lexer.next_token().type, TokenType::CARET);
}

TEST(Lexer_Comparison_Operators) {
    Lexer lexer("< <= > >= == !=");
    ASSERT_EQ(lexer.next_token().type, TokenType::LT);
    ASSERT_EQ(lexer.next_token().type, TokenType::LE);
    ASSERT_EQ(lexer.next_token().type, TokenType::GT);
    ASSERT_EQ(lexer.next_token().type, TokenType::GE);
    ASSERT_EQ(lexer.next_token().type, TokenType::EQ);
    ASSERT_EQ(lexer.next_token().type, TokenType::NE);
}

TEST(Lexer_Logical_Operators) {
    Lexer lexer("&& || !");
    ASSERT_EQ(lexer.next_token().type, TokenType::AND);
    ASSERT_EQ(lexer.next_token().type, TokenType::OR);
    ASSERT_EQ(lexer.next_token().type, TokenType::NOT);
}

TEST(Lexer_Match_Operators) {
    Lexer lexer("~ !~");
    ASSERT_EQ(lexer.next_token().type, TokenType::MATCH);
    ASSERT_EQ(lexer.next_token().type, TokenType::NOT_MATCH);
}

TEST(Lexer_Assignment_Operators) {
    // Test individually to avoid regex context issues
    {
        Lexer lexer("x = 1");
        lexer.next_token(); // x
        ASSERT_EQ(lexer.next_token().type, TokenType::ASSIGN);
    }
    {
        Lexer lexer("x += 1");
        lexer.next_token(); // x
        ASSERT_EQ(lexer.next_token().type, TokenType::PLUS_ASSIGN);
    }
    {
        Lexer lexer("x -= 1");
        lexer.next_token(); // x
        ASSERT_EQ(lexer.next_token().type, TokenType::MINUS_ASSIGN);
    }
    {
        Lexer lexer("x *= 1");
        lexer.next_token(); // x
        ASSERT_EQ(lexer.next_token().type, TokenType::STAR_ASSIGN);
    }
    {
        Lexer lexer("x /= 1");
        lexer.next_token(); // x
        ASSERT_EQ(lexer.next_token().type, TokenType::SLASH_ASSIGN);
    }
    {
        Lexer lexer("x %= 1");
        lexer.next_token(); // x
        ASSERT_EQ(lexer.next_token().type, TokenType::PERCENT_ASSIGN);
    }
    {
        Lexer lexer("x ^= 1");
        lexer.next_token(); // x
        ASSERT_EQ(lexer.next_token().type, TokenType::CARET_ASSIGN);
    }
}

TEST(Lexer_Increment_Decrement) {
    Lexer lexer("++ --");
    ASSERT_EQ(lexer.next_token().type, TokenType::INCREMENT);
    ASSERT_EQ(lexer.next_token().type, TokenType::DECREMENT);
}

// ============================================================================
// Keywords
// ============================================================================

TEST(Lexer_Control_Keywords) {
    Lexer lexer("if else while for do");
    ASSERT_EQ(lexer.next_token().type, TokenType::IF);
    ASSERT_EQ(lexer.next_token().type, TokenType::ELSE);
    ASSERT_EQ(lexer.next_token().type, TokenType::WHILE);
    ASSERT_EQ(lexer.next_token().type, TokenType::FOR);
    ASSERT_EQ(lexer.next_token().type, TokenType::DO);
}

TEST(Lexer_Jump_Keywords) {
    Lexer lexer("break continue next exit return");
    ASSERT_EQ(lexer.next_token().type, TokenType::BREAK);
    ASSERT_EQ(lexer.next_token().type, TokenType::CONTINUE);
    ASSERT_EQ(lexer.next_token().type, TokenType::NEXT);
    ASSERT_EQ(lexer.next_token().type, TokenType::EXIT);
    ASSERT_EQ(lexer.next_token().type, TokenType::RETURN);
}

TEST(Lexer_Block_Keywords) {
    Lexer lexer("BEGIN END function");
    ASSERT_EQ(lexer.next_token().type, TokenType::BEGIN_KW);
    ASSERT_EQ(lexer.next_token().type, TokenType::END_KW);
    ASSERT_EQ(lexer.next_token().type, TokenType::FUNCTION);
}

TEST(Lexer_IO_Keywords) {
    Lexer lexer("print printf getline");
    ASSERT_EQ(lexer.next_token().type, TokenType::PRINT);
    ASSERT_EQ(lexer.next_token().type, TokenType::PRINTF);
    ASSERT_EQ(lexer.next_token().type, TokenType::GETLINE);
}

TEST(Lexer_Other_Keywords) {
    Lexer lexer("in delete switch case default");
    ASSERT_EQ(lexer.next_token().type, TokenType::IN);
    ASSERT_EQ(lexer.next_token().type, TokenType::DELETE);
    ASSERT_EQ(lexer.next_token().type, TokenType::SWITCH);
    ASSERT_EQ(lexer.next_token().type, TokenType::CASE);
    ASSERT_EQ(lexer.next_token().type, TokenType::DEFAULT);
}

// ============================================================================
// Identifiers
// ============================================================================

TEST(Lexer_Simple_Identifier) {
    Lexer lexer("foo");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(tok.lexeme, "foo");
}

TEST(Lexer_Identifier_With_Underscore) {
    Lexer lexer("foo_bar");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(tok.lexeme, "foo_bar");
}

TEST(Lexer_Identifier_With_Numbers) {
    Lexer lexer("var123");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(tok.lexeme, "var123");
}

// ============================================================================
// Punctuation
// ============================================================================

TEST(Lexer_Brackets) {
    Lexer lexer("( ) [ ] { }");
    ASSERT_EQ(lexer.next_token().type, TokenType::LPAREN);
    ASSERT_EQ(lexer.next_token().type, TokenType::RPAREN);
    ASSERT_EQ(lexer.next_token().type, TokenType::LBRACKET);
    ASSERT_EQ(lexer.next_token().type, TokenType::RBRACKET);
    ASSERT_EQ(lexer.next_token().type, TokenType::LBRACE);
    ASSERT_EQ(lexer.next_token().type, TokenType::RBRACE);
}

TEST(Lexer_Delimiters) {
    Lexer lexer(", ; ? :");
    ASSERT_EQ(lexer.next_token().type, TokenType::COMMA);
    ASSERT_EQ(lexer.next_token().type, TokenType::SEMICOLON);
    ASSERT_EQ(lexer.next_token().type, TokenType::QUESTION);
    ASSERT_EQ(lexer.next_token().type, TokenType::COLON);
}

TEST(Lexer_Field_Access) {
    Lexer lexer("$1");
    ASSERT_EQ(lexer.next_token().type, TokenType::DOLLAR);
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_DOUBLE_EQ(std::get<double>(tok.literal), 1.0);
}

// ============================================================================
// Redirection Operators
// ============================================================================

TEST(Lexer_Redirect_Output) {
    Lexer lexer("> >>");
    ASSERT_EQ(lexer.next_token().type, TokenType::GT);
    ASSERT_EQ(lexer.next_token().type, TokenType::APPEND);
}

TEST(Lexer_Pipe) {
    Lexer lexer("| |&");
    ASSERT_EQ(lexer.next_token().type, TokenType::PIPE);
    ASSERT_EQ(lexer.next_token().type, TokenType::PIPE_BOTH);
}

// ============================================================================
// Comments
// ============================================================================

TEST(Lexer_Line_Comment) {
    Lexer lexer("42 # this is a comment\n10");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_DOUBLE_EQ(std::get<double>(tok.literal), 42.0);

    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NEWLINE);

    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_DOUBLE_EQ(std::get<double>(tok.literal), 10.0);
}

// ============================================================================
// Complex Expressions
// ============================================================================

TEST(Lexer_Array_Access) {
    Lexer lexer("arr[\"key\"]");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(tok.lexeme, "arr");

    ASSERT_EQ(lexer.next_token().type, TokenType::LBRACKET);

    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::STRING);
    ASSERT_EQ(std::get<std::string>(tok.literal), "key");

    ASSERT_EQ(lexer.next_token().type, TokenType::RBRACKET);
}

TEST(Lexer_Function_Call) {
    Lexer lexer("length(x)");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(tok.lexeme, "length");

    ASSERT_EQ(lexer.next_token().type, TokenType::LPAREN);

    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(tok.lexeme, "x");

    ASSERT_EQ(lexer.next_token().type, TokenType::RPAREN);
}

TEST(Lexer_Ternary_Expression) {
    Lexer lexer("a ? b : c");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);

    ASSERT_EQ(lexer.next_token().type, TokenType::QUESTION);

    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);

    ASSERT_EQ(lexer.next_token().type, TokenType::COLON);

    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
}

// ============================================================================
// Context-Based Regex Recognition
// ============================================================================

TEST(Lexer_Regex_After_Match_Operator) {
    Lexer lexer("$0 ~ /test/");
    ASSERT_EQ(lexer.next_token().type, TokenType::DOLLAR);
    ASSERT_EQ(lexer.next_token().type, TokenType::NUMBER);
    ASSERT_EQ(lexer.next_token().type, TokenType::MATCH);

    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::REGEX);
    ASSERT_EQ(std::get<std::string>(tok.literal), "test");
}

TEST(Lexer_Division_After_Number) {
    Lexer lexer("10 / 2");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
    ASSERT_EQ(lexer.next_token().type, TokenType::SLASH);
    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::NUMBER);
}

TEST(Lexer_Division_After_Identifier) {
    Lexer lexer("x / y");
    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(lexer.next_token().type, TokenType::SLASH);
    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
}

// ============================================================================
// Peek Token
// ============================================================================

TEST(Lexer_Peek_Doesnt_Consume) {
    Lexer lexer("a b");
    Token peeked = lexer.peek_token();
    ASSERT_EQ(peeked.type, TokenType::IDENTIFIER);
    ASSERT_EQ(peeked.lexeme, "a");

    Token tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(tok.lexeme, "a");

    tok = lexer.next_token();
    ASSERT_EQ(tok.type, TokenType::IDENTIFIER);
    ASSERT_EQ(tok.lexeme, "b");
}

TEST(Lexer_Multiple_Peeks) {
    Lexer lexer("test");
    Token peek1 = lexer.peek_token();
    Token peek2 = lexer.peek_token();

    ASSERT_EQ(peek1.type, peek2.type);
    ASSERT_EQ(peek1.lexeme, peek2.lexeme);
}
