// Parser Unit Tests
#include "test_framework.hpp"
#include "awk/lexer.hpp"
#include "awk/parser.hpp"
#include "awk/ast.hpp"

using namespace awk;
using namespace test;

// Helper zum Parsen eines Strings
std::unique_ptr<Program> parse(const std::string& source) {
    return Parser::parse_string(source);
}

// ============================================================================
// Basic Program Structure
// ============================================================================

TEST(Parser_Empty_Program) {
    auto prog = parse("");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_TRUE(prog->rules.empty());
    ASSERT_TRUE(prog->functions.empty());
}

TEST(Parser_BEGIN_Block) {
    auto prog = parse("BEGIN { print 1 }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(1));
    ASSERT_TRUE(prog->rules[0]->pattern.type == PatternType::BEGIN);
}

TEST(Parser_END_Block) {
    auto prog = parse("END { print \"done\" }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(1));
    ASSERT_TRUE(prog->rules[0]->pattern.type == PatternType::END);
}

TEST(Parser_Multiple_BEGIN_Blocks) {
    auto prog = parse("BEGIN { x = 1 } BEGIN { x = 2 }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(2));
    ASSERT_TRUE(prog->rules[0]->pattern.type == PatternType::BEGIN);
    ASSERT_TRUE(prog->rules[1]->pattern.type == PatternType::BEGIN);
}

// ============================================================================
// Pattern Rules
// ============================================================================

TEST(Parser_Pattern_Only) {
    auto prog = parse("/test/");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(1));
    ASSERT_TRUE(prog->rules[0]->pattern.type == PatternType::REGEX);
}

TEST(Parser_Pattern_With_Action) {
    auto prog = parse("/test/ { print }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(1));
    ASSERT_TRUE(prog->rules[0]->action != nullptr);
}

TEST(Parser_Expression_Pattern) {
    auto prog = parse("NR > 1 { print }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(1));
    ASSERT_TRUE(prog->rules[0]->pattern.type == PatternType::EXPRESSION);
}

TEST(Parser_Range_Pattern) {
    auto prog = parse("/start/,/end/ { print }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(1));
    ASSERT_TRUE(prog->rules[0]->pattern.type == PatternType::RANGE);
}

TEST(Parser_Action_Only) {
    auto prog = parse("{ print $0 }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(1));
    ASSERT_TRUE(prog->rules[0]->pattern.type == PatternType::EMPTY);
}

// ============================================================================
// Function Definitions
// ============================================================================

TEST(Parser_Function_No_Params) {
    auto prog = parse("function foo() { return 42 }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->functions.size(), static_cast<size_t>(1));
    ASSERT_EQ(prog->functions[0]->name, "foo");
    ASSERT_TRUE(prog->functions[0]->parameters.empty());
}

TEST(Parser_Function_With_Params) {
    auto prog = parse("function add(a, b) { return a + b }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->functions.size(), static_cast<size_t>(1));
    ASSERT_EQ(prog->functions[0]->name, "add");
    ASSERT_EQ(prog->functions[0]->parameters.size(), static_cast<size_t>(2));
    ASSERT_EQ(prog->functions[0]->parameters[0], "a");
    ASSERT_EQ(prog->functions[0]->parameters[1], "b");
}

TEST(Parser_Multiple_Functions) {
    auto prog = parse("function a() { } function b() { }");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->functions.size(), static_cast<size_t>(2));
}

// ============================================================================
// Statements
// ============================================================================

TEST(Parser_If_Statement) {
    auto prog = parse("BEGIN { if (1) print }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    ASSERT_TRUE(block != nullptr);
    ASSERT_EQ(block->statements.size(), static_cast<size_t>(1));
    auto* if_stmt = dynamic_cast<IfStmt*>(block->statements[0].get());
    ASSERT_TRUE(if_stmt != nullptr);
}

TEST(Parser_If_Else_Statement) {
    auto prog = parse("BEGIN { if (1) print \"yes\"; else print \"no\" }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    auto* if_stmt = dynamic_cast<IfStmt*>(block->statements[0].get());
    ASSERT_TRUE(if_stmt != nullptr);
    ASSERT_TRUE(if_stmt->else_branch != nullptr);
}

TEST(Parser_While_Statement) {
    auto prog = parse("BEGIN { while (x < 10) x++ }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    auto* while_stmt = dynamic_cast<WhileStmt*>(block->statements[0].get());
    ASSERT_TRUE(while_stmt != nullptr);
}

TEST(Parser_Do_While_Statement) {
    auto prog = parse("BEGIN { do x++ while (x < 10) }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    auto* do_while = dynamic_cast<DoWhileStmt*>(block->statements[0].get());
    ASSERT_TRUE(do_while != nullptr);
}

TEST(Parser_For_Statement) {
    auto prog = parse("BEGIN { for (i = 0; i < 10; i++) print i }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    auto* for_stmt = dynamic_cast<ForStmt*>(block->statements[0].get());
    ASSERT_TRUE(for_stmt != nullptr);
}

TEST(Parser_For_In_Statement) {
    auto prog = parse("BEGIN { for (k in arr) print k }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    auto* for_in = dynamic_cast<ForInStmt*>(block->statements[0].get());
    ASSERT_TRUE(for_in != nullptr);
    ASSERT_EQ(for_in->variable, "k");
    ASSERT_EQ(for_in->array_name, "arr");
}

TEST(Parser_Print_Statement) {
    auto prog = parse("BEGIN { print \"hello\" }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    auto* print_stmt = dynamic_cast<PrintStmt*>(block->statements[0].get());
    ASSERT_TRUE(print_stmt != nullptr);
}

TEST(Parser_Printf_Statement) {
    auto prog = parse("BEGIN { printf \"%d\\n\", 42 }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    auto* printf_stmt = dynamic_cast<PrintfStmt*>(block->statements[0].get());
    ASSERT_TRUE(printf_stmt != nullptr);
}

TEST(Parser_Delete_Statement) {
    auto prog = parse("BEGIN { delete arr[1] }");
    ASSERT_TRUE(prog != nullptr);
    auto* block = dynamic_cast<BlockStmt*>(prog->rules[0]->action.get());
    auto* del_stmt = dynamic_cast<DeleteStmt*>(block->statements[0].get());
    ASSERT_TRUE(del_stmt != nullptr);
}

// ============================================================================
// Expressions
// ============================================================================

TEST(Parser_Literal_Number) {
    auto prog = parse("BEGIN { x = 42 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Literal_String) {
    auto prog = parse("BEGIN { x = \"hello\" }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Binary_Addition) {
    auto prog = parse("BEGIN { x = 1 + 2 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Binary_Multiplication) {
    auto prog = parse("BEGIN { x = 3 * 4 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Binary_Power) {
    auto prog = parse("BEGIN { x = 2 ^ 10 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Comparison) {
    auto prog = parse("BEGIN { x = a < b }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Logical_And) {
    auto prog = parse("BEGIN { x = a && b }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Logical_Or) {
    auto prog = parse("BEGIN { x = a || b }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Ternary) {
    auto prog = parse("BEGIN { x = a ? b : c }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Regex_Match) {
    auto prog = parse("BEGIN { x = $0 ~ /test/ }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Regex_Not_Match) {
    auto prog = parse("BEGIN { x = $0 !~ /test/ }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Array_In) {
    auto prog = parse("BEGIN { x = \"key\" in arr }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Concatenation) {
    auto prog = parse("BEGIN { x = \"hello\" \" \" \"world\" }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Unary_Minus) {
    auto prog = parse("BEGIN { x = -5 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Unary_Not) {
    auto prog = parse("BEGIN { x = !a }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Increment) {
    auto prog = parse("BEGIN { x++ }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Decrement) {
    auto prog = parse("BEGIN { x-- }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Pre_Increment) {
    auto prog = parse("BEGIN { ++x }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Pre_Decrement) {
    auto prog = parse("BEGIN { --x }");
    ASSERT_TRUE(prog != nullptr);
}

// ============================================================================
// Assignment
// ============================================================================

TEST(Parser_Simple_Assignment) {
    auto prog = parse("BEGIN { x = 1 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Plus_Assign) {
    auto prog = parse("BEGIN { x += 1 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Minus_Assign) {
    auto prog = parse("BEGIN { x -= 1 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Star_Assign) {
    auto prog = parse("BEGIN { x *= 2 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Slash_Assign) {
    auto prog = parse("BEGIN { x /= 2 }");
    ASSERT_TRUE(prog != nullptr);
}

// ============================================================================
// Array Access
// ============================================================================

TEST(Parser_Array_Single_Index) {
    auto prog = parse("BEGIN { arr[1] = 10 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Array_String_Index) {
    auto prog = parse("BEGIN { arr[\"key\"] = \"value\" }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Array_Multi_Index) {
    auto prog = parse("BEGIN { arr[1, 2] = 10 }");
    ASSERT_TRUE(prog != nullptr);
}

// ============================================================================
// Field Access
// ============================================================================

TEST(Parser_Field_Number) {
    auto prog = parse("{ print $1 }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Field_Expression) {
    auto prog = parse("{ print $(NF-1) }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Field_Zero) {
    auto prog = parse("{ print $0 }");
    ASSERT_TRUE(prog != nullptr);
}

// ============================================================================
// Function Calls
// ============================================================================

TEST(Parser_Builtin_Function) {
    auto prog = parse("BEGIN { x = length(\"test\") }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Builtin_No_Args) {
    auto prog = parse("BEGIN { srand() }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Builtin_Multiple_Args) {
    auto prog = parse("BEGIN { x = substr(\"hello\", 1, 3) }");
    ASSERT_TRUE(prog != nullptr);
}

// ============================================================================
// Output Redirection
// ============================================================================

TEST(Parser_Print_To_File) {
    auto prog = parse("BEGIN { print \"hello\" > \"file.txt\" }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Print_Append) {
    auto prog = parse("BEGIN { print \"hello\" >> \"file.txt\" }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Print_To_Pipe) {
    auto prog = parse("BEGIN { print \"hello\" | \"cat\" }");
    ASSERT_TRUE(prog != nullptr);
}

// ============================================================================
// Getline
// ============================================================================

TEST(Parser_Getline_Simple) {
    auto prog = parse("{ getline }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Getline_Into_Var) {
    auto prog = parse("{ getline x }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Getline_From_File) {
    auto prog = parse("{ getline < \"file.txt\" }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Getline_From_Pipe) {
    auto prog = parse("{ \"cmd\" | getline x }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Getline_From_Pipe_No_Var) {
    auto prog = parse("{ \"echo hello\" | getline }");
    ASSERT_TRUE(prog != nullptr);
}

TEST(Parser_Getline_From_Coprocess) {
    auto prog = parse("{ \"cmd\" |& getline x }");
    ASSERT_TRUE(prog != nullptr);
}

// ============================================================================
// Complex Programs
// ============================================================================

TEST(Parser_Word_Count) {
    auto prog = parse(R"(
        BEGIN { words = 0 }
        { words += NF }
        END { print words }
    )");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(3));
}

TEST(Parser_Factorial_Function) {
    auto prog = parse(R"(
        function factorial(n) {
            if (n <= 1) return 1
            return n * factorial(n - 1)
        }
        BEGIN { print factorial(5) }
    )");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->functions.size(), static_cast<size_t>(1));
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(1));
}

TEST(Parser_Multiple_Patterns) {
    auto prog = parse(R"(
        /^#/ { next }
        /error/ { errors++ }
        /warning/ { warnings++ }
        END { print errors, warnings }
    )");
    ASSERT_TRUE(prog != nullptr);
    ASSERT_EQ(prog->rules.size(), static_cast<size_t>(4));
}

// ============================================================================
// Error Handling
// ============================================================================

TEST(Parser_Error_Unclosed_Brace) {
    auto prog = parse("BEGIN { print 1");
    // Parser should detect error
    ASSERT_TRUE(prog != nullptr);
    // Error is shown in parser.had_error() - we only check that it doesn't crash
}

TEST(Parser_Error_Missing_Paren) {
    auto prog = parse("BEGIN { if (x print }");
    ASSERT_TRUE(prog != nullptr);
}
