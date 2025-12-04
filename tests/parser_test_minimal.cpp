// Parser Unit Tests - Minimal version for debugging
#include "test_framework.hpp"
#include "awk/lexer.hpp"
#include "awk/parser.hpp"
#include "awk/ast.hpp"

using namespace awk;
using namespace test;

// Helper to parse a string
std::unique_ptr<Program> parse_min(const std::string& source) {
    return Parser::parse_string(source);
}

TEST(ParserMin_Empty_Program) {
    auto prog = parse_min("");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_BEGIN_Block) {
    auto prog = parse_min("BEGIN { print 1 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Simple_Pattern) {
    auto prog = parse_min("/test/");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Simple_Action) {
    auto prog = parse_min("{ print }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Function_Definition) {
    auto prog = parse_min("function foo() { return 1 }");
    ASSERT_TRUE(prog != nullptr);
}

// Weitere Tests aus parser_test.cpp
TEST(ParserMin_Pattern_With_Action) {
    auto prog = parse_min("/test/ { print }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Expression_Pattern) {
    auto prog = parse_min("NR > 1 { print }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Range_Pattern) {
    auto prog = parse_min("/start/,/end/ { print }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_If_Statement) {
    auto prog = parse_min("BEGIN { if (1) print }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_While_Statement) {
    auto prog = parse_min("BEGIN { while (x < 10) x++ }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_For_Statement) {
    auto prog = parse_min("BEGIN { for (i = 0; i < 10; i++) print i }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_For_In_Statement) {
    auto prog = parse_min("BEGIN { for (k in arr) print k }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Assignment) {
    auto prog = parse_min("BEGIN { x = 1 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Arithmetic) {
    auto prog = parse_min("BEGIN { x = 1 + 2 * 3 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Regex_Match) {
    auto prog = parse_min("BEGIN { x = $0 ~ /test/ }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Array_Access) {
    auto prog = parse_min("BEGIN { arr[1] = 10 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Print_To_File) {
    auto prog = parse_min("BEGIN { print \"hello\" > \"file.txt\" }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(ParserMin_Getline) {
    auto prog = parse_min("{ getline }");
    ASSERT_TRUE(prog != nullptr);
}
