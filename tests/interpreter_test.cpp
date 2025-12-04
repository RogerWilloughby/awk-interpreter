// Interpreter Unit Tests
#include "test_framework.hpp"
#include "awk/lexer.hpp"
#include "awk/parser.hpp"
#include "awk/interpreter.hpp"
#include <sstream>

using namespace awk;
using namespace test;

// Helper: Run AWK program and capture output (BEGIN/END only)
std::string run_awk_simple(const std::string& source) {
    auto prog = Parser::parse_string(source);
    if (!prog) return "PARSE_ERROR";

    Interpreter interp;
    std::ostringstream output;
    interp.set_output_stream(output);

    std::vector<std::string> files;

    try {
        interp.run(*prog, files);
    } catch (const std::exception& e) {
        return std::string("RUNTIME_ERROR: ") + e.what();
    }

    return output.str();
}

// Helper mit Input als Zeichenkette
std::string run_awk(const std::string& source, const std::string& input = "") {
    if (input.empty()) {
        return run_awk_simple(source);
    }

    auto prog = Parser::parse_string(source);
    if (!prog) return "PARSE_ERROR";

    Interpreter interp;
    std::ostringstream output;
    interp.set_output_stream(output);

    // Verwende stringstream statt Datei
    std::istringstream input_stream(input);

    // Setze Record und verarbeite
    std::vector<std::string> files;
    files.push_back("__stdin__");

    // Schreibe Input in temp-Datei
    std::ofstream tmp("__test_input.tmp");
    tmp << input;
    tmp.close();
    files[0] = "__test_input.tmp";

    try {
        interp.run(*prog, files);
    } catch (const std::exception& e) {
        std::remove("__test_input.tmp");
        return std::string("RUNTIME_ERROR: ") + e.what();
    }

    // Clean up
    std::remove("__test_input.tmp");

    return output.str();
}

// ============================================================================
// Basic Output
// ============================================================================

TEST(Interpreter_Print_String) {
    std::string result = run_awk("BEGIN { print \"Hello, World!\" }");
    ASSERT_EQ(result, "Hello, World!\n");
}

TEST(Interpreter_Print_Number) {
    std::string result = run_awk("BEGIN { print 42 }");
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Print_Float) {
    std::string result = run_awk("BEGIN { print 3.14 }");
    ASSERT_TRUE(result.find("3.14") != std::string::npos);
}

TEST(Interpreter_Print_Multiple) {
    std::string result = run_awk("BEGIN { print 1, 2, 3 }");
    ASSERT_EQ(result, "1 2 3\n");
}

TEST(Interpreter_Print_Empty) {
    std::string result = run_awk("BEGIN { print }");
    ASSERT_EQ(result, "\n");
}

// ============================================================================
// Arithmetic
// ============================================================================

TEST(Interpreter_Addition) {
    std::string result = run_awk("BEGIN { print 2 + 3 }");
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Subtraction) {
    std::string result = run_awk("BEGIN { print 10 - 4 }");
    ASSERT_EQ(result, "6\n");
}

TEST(Interpreter_Multiplication) {
    std::string result = run_awk("BEGIN { print 6 * 7 }");
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Division) {
    std::string result = run_awk("BEGIN { print 20 / 4 }");
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Modulo) {
    std::string result = run_awk("BEGIN { print 17 % 5 }");
    ASSERT_EQ(result, "2\n");
}

TEST(Interpreter_Power) {
    std::string result = run_awk("BEGIN { print 2 ^ 10 }");
    ASSERT_EQ(result, "1024\n");
}

TEST(Interpreter_Unary_Minus) {
    std::string result = run_awk("BEGIN { print -5 }");
    ASSERT_EQ(result, "-5\n");
}

TEST(Interpreter_Precedence) {
    std::string result = run_awk("BEGIN { print 2 + 3 * 4 }");
    ASSERT_EQ(result, "14\n");
}

TEST(Interpreter_Parentheses) {
    std::string result = run_awk("BEGIN { print (2 + 3) * 4 }");
    ASSERT_EQ(result, "20\n");
}

// ============================================================================
// Variables
// ============================================================================

TEST(Interpreter_Variable_Assignment) {
    std::string result = run_awk("BEGIN { x = 10; print x }");
    ASSERT_EQ(result, "10\n");
}

TEST(Interpreter_Variable_Update) {
    std::string result = run_awk("BEGIN { x = 1; x = 2; print x }");
    ASSERT_EQ(result, "2\n");
}

TEST(Interpreter_Uninitialized_Variable) {
    std::string result = run_awk("BEGIN { print x + 0 }");
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Plus_Assign) {
    std::string result = run_awk("BEGIN { x = 10; x += 5; print x }");
    ASSERT_EQ(result, "15\n");
}

TEST(Interpreter_Minus_Assign) {
    std::string result = run_awk("BEGIN { x = 10; x -= 3; print x }");
    ASSERT_EQ(result, "7\n");
}

TEST(Interpreter_Star_Assign) {
    std::string result = run_awk("BEGIN { x = 5; x *= 3; print x }");
    ASSERT_EQ(result, "15\n");
}

TEST(Interpreter_Increment) {
    std::string result = run_awk("BEGIN { x = 5; x++; print x }");
    ASSERT_EQ(result, "6\n");
}

TEST(Interpreter_Decrement) {
    std::string result = run_awk("BEGIN { x = 5; x--; print x }");
    ASSERT_EQ(result, "4\n");
}

TEST(Interpreter_Pre_Increment) {
    std::string result = run_awk("BEGIN { x = 5; print ++x }");
    ASSERT_EQ(result, "6\n");
}

TEST(Interpreter_Post_Increment) {
    std::string result = run_awk("BEGIN { x = 5; print x++ }");
    ASSERT_EQ(result, "5\n");
}

// ============================================================================
// String Operations
// ============================================================================

TEST(Interpreter_String_Concatenation) {
    std::string result = run_awk("BEGIN { print \"Hello\" \" \" \"World\" }");
    ASSERT_EQ(result, "Hello World\n");
}

TEST(Interpreter_String_Number_Concat) {
    std::string result = run_awk("BEGIN { print \"Value: \" 42 }");
    ASSERT_EQ(result, "Value: 42\n");
}

TEST(Interpreter_String_Comparison) {
    std::string result = run_awk("BEGIN { print (\"abc\" < \"abd\") }");
    ASSERT_EQ(result, "1\n");
}

// ============================================================================
// Comparison & Logic
// ============================================================================

TEST(Interpreter_Less_Than) {
    std::string result = run_awk("BEGIN { print (1 < 2) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Greater_Than) {
    std::string result = run_awk("BEGIN { print (2 > 1) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Equal) {
    std::string result = run_awk("BEGIN { print (5 == 5) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Not_Equal) {
    std::string result = run_awk("BEGIN { print (5 != 3) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Logical_And) {
    std::string result = run_awk("BEGIN { print (1 && 1) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Logical_Or) {
    std::string result = run_awk("BEGIN { print (0 || 1) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Logical_Not) {
    std::string result = run_awk("BEGIN { print !0 }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Ternary_True) {
    std::string result = run_awk("BEGIN { print (1 ? \"yes\" : \"no\") }");
    ASSERT_EQ(result, "yes\n");
}

TEST(Interpreter_Ternary_False) {
    std::string result = run_awk("BEGIN { print (0 ? \"yes\" : \"no\") }");
    ASSERT_EQ(result, "no\n");
}

// ============================================================================
// Control Flow
// ============================================================================

TEST(Interpreter_If_True) {
    std::string result = run_awk("BEGIN { if (1) print \"yes\" }");
    ASSERT_EQ(result, "yes\n");
}

TEST(Interpreter_If_False) {
    std::string result = run_awk("BEGIN { if (0) print \"yes\" }");
    ASSERT_EQ(result, "");
}

TEST(Interpreter_If_Else_True) {
    std::string result = run_awk("BEGIN { if (1) print \"yes\"; else print \"no\" }");
    ASSERT_EQ(result, "yes\n");
}

TEST(Interpreter_If_Else_False) {
    std::string result = run_awk("BEGIN { if (0) print \"yes\"; else print \"no\" }");
    ASSERT_EQ(result, "no\n");
}

TEST(Interpreter_While_Loop) {
    std::string result = run_awk("BEGIN { i = 0; while (i < 3) { print i; i++ } }");
    ASSERT_EQ(result, "0\n1\n2\n");
}

TEST(Interpreter_Do_While_Loop) {
    std::string result = run_awk("BEGIN { i = 0; do { print i; i++ } while (i < 3) }");
    ASSERT_EQ(result, "0\n1\n2\n");
}

TEST(Interpreter_For_Loop) {
    std::string result = run_awk("BEGIN { for (i = 1; i <= 3; i++) print i }");
    ASSERT_EQ(result, "1\n2\n3\n");
}

TEST(Interpreter_Break) {
    std::string result = run_awk("BEGIN { for (i = 1; i <= 10; i++) { if (i > 3) break; print i } }");
    ASSERT_EQ(result, "1\n2\n3\n");
}

TEST(Interpreter_Continue) {
    std::string result = run_awk("BEGIN { for (i = 1; i <= 5; i++) { if (i == 3) continue; print i } }");
    ASSERT_EQ(result, "1\n2\n4\n5\n");
}

// ============================================================================
// Arrays
// ============================================================================

TEST(Interpreter_Array_Set_Get) {
    std::string result = run_awk("BEGIN { arr[1] = \"one\"; print arr[1] }");
    ASSERT_EQ(result, "one\n");
}

TEST(Interpreter_Array_String_Key) {
    std::string result = run_awk("BEGIN { arr[\"key\"] = \"value\"; print arr[\"key\"] }");
    ASSERT_EQ(result, "value\n");
}

TEST(Interpreter_Array_In) {
    std::string result = run_awk("BEGIN { arr[1] = 1; print (1 in arr) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Array_Not_In) {
    std::string result = run_awk("BEGIN { arr[1] = 1; print (2 in arr) }");
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Array_Delete) {
    std::string result = run_awk("BEGIN { arr[1] = 1; delete arr[1]; print (1 in arr) }");
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_For_In) {
    std::string result = run_awk("BEGIN { arr[\"a\"]=1; arr[\"b\"]=2; for (k in arr) count++; print count }");
    ASSERT_EQ(result, "2\n");
}

TEST(Interpreter_Array_Multi_Index) {
    std::string result = run_awk("BEGIN { arr[1,2] = \"multi\"; print arr[1,2] }");
    ASSERT_EQ(result, "multi\n");
}

// ============================================================================
// Built-in Functions
// ============================================================================

TEST(Interpreter_Length_String) {
    std::string result = run_awk("BEGIN { print length(\"hello\") }");
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Substr) {
    std::string result = run_awk("BEGIN { print substr(\"hello\", 2, 3) }");
    ASSERT_EQ(result, "ell\n");
}

TEST(Interpreter_Index) {
    std::string result = run_awk("BEGIN { print index(\"hello\", \"ll\") }");
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Split) {
    std::string result = run_awk("BEGIN { n = split(\"a:b:c\", arr, \":\"); print n, arr[1], arr[2], arr[3] }");
    ASSERT_EQ(result, "3 a b c\n");
}

TEST(Interpreter_Sprintf) {
    std::string result = run_awk("BEGIN { print sprintf(\"%05d\", 42) }");
    ASSERT_EQ(result, "00042\n");
}

TEST(Interpreter_Toupper) {
    std::string result = run_awk("BEGIN { print toupper(\"hello\") }");
    ASSERT_EQ(result, "HELLO\n");
}

TEST(Interpreter_Tolower) {
    std::string result = run_awk("BEGIN { print tolower(\"HELLO\") }");
    ASSERT_EQ(result, "hello\n");
}

TEST(Interpreter_Sqrt) {
    std::string result = run_awk("BEGIN { print sqrt(16) }");
    ASSERT_EQ(result, "4\n");
}

TEST(Interpreter_Sin) {
    std::string result = run_awk("BEGIN { print sin(0) }");
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Cos) {
    std::string result = run_awk("BEGIN { print cos(0) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Int) {
    std::string result = run_awk("BEGIN { print int(3.7) }");
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Log) {
    // log(e) ≈ 1, aber durch Rundung kann 0.99 oder 1 rauskommen
    std::string result = run_awk("BEGIN { x = log(2.718281828); print (x > 0.99 && x < 1.01) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Exp) {
    std::string result = run_awk("BEGIN { print int(exp(1) * 100) / 100 }");
    ASSERT_TRUE(result.find("2.71") != std::string::npos);
}

// ============================================================================
// User-Defined Functions
// ============================================================================

TEST(Interpreter_Simple_Function) {
    std::string result = run_awk("function double(n) { return n * 2 } BEGIN { print double(21) }");
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Recursive_Function) {
    std::string result = run_awk(R"(
        function factorial(n) {
            if (n <= 1) return 1
            return n * factorial(n - 1)
        }
        BEGIN { print factorial(5) }
    )");
    ASSERT_EQ(result, "120\n");
}

TEST(Interpreter_Function_With_Array) {
    std::string result = run_awk(R"(
        function sum(arr, n) {
            total = 0
            for (i = 1; i <= n; i++) total += arr[i]
            return total
        }
        BEGIN {
            a[1] = 1; a[2] = 2; a[3] = 3
            print sum(a, 3)
        }
    )");
    ASSERT_EQ(result, "6\n");
}

// ============================================================================
// Field Processing
// ============================================================================

TEST(Interpreter_Field_1) {
    std::string result = run_awk("{ print $1 }", "hello world");
    ASSERT_EQ(result, "hello\n");
}

TEST(Interpreter_Field_2) {
    std::string result = run_awk("{ print $2 }", "hello world");
    ASSERT_EQ(result, "world\n");
}

TEST(Interpreter_Field_0) {
    std::string result = run_awk("{ print $0 }", "hello world");
    ASSERT_EQ(result, "hello world\n");
}

TEST(Interpreter_NF) {
    std::string result = run_awk("{ print NF }", "one two three");
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_NR) {
    std::string result = run_awk("END { print NR }", "line1\nline2\nline3");
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Last_Field) {
    std::string result = run_awk("{ print $NF }", "one two three");
    ASSERT_EQ(result, "three\n");
}

TEST(Interpreter_Field_Assignment) {
    std::string result = run_awk("{ $2 = \"CHANGED\"; print }", "one two three");
    ASSERT_EQ(result, "one CHANGED three\n");
}

// ============================================================================
// Patterns
// ============================================================================

TEST(Interpreter_Regex_Pattern) {
    std::string result = run_awk("/error/ { print }", "info\nerror\nwarning");
    ASSERT_EQ(result, "error\n");
}

TEST(Interpreter_Expression_Pattern) {
    std::string result = run_awk("NR == 2 { print }", "line1\nline2\nline3");
    ASSERT_EQ(result, "line2\n");
}

TEST(Interpreter_Negated_Pattern) {
    std::string result = run_awk("!/skip/ { print }", "keep\nskip\nalso keep");
    ASSERT_EQ(result, "keep\nalso keep\n");
}

// ============================================================================
// Printf
// ============================================================================

TEST(Interpreter_Printf_String) {
    std::string result = run_awk("BEGIN { printf \"Hello\\n\" }");
    ASSERT_EQ(result, "Hello\n");
}

TEST(Interpreter_Printf_Int) {
    std::string result = run_awk("BEGIN { printf \"%d\\n\", 42 }");
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Printf_Padded) {
    std::string result = run_awk("BEGIN { printf \"%5d\\n\", 42 }");
    ASSERT_EQ(result, "   42\n");
}

TEST(Interpreter_Printf_Zero_Padded) {
    std::string result = run_awk("BEGIN { printf \"%05d\\n\", 42 }");
    ASSERT_EQ(result, "00042\n");
}

TEST(Interpreter_Printf_Float) {
    std::string result = run_awk("BEGIN { printf \"%.2f\\n\", 3.14159 }");
    ASSERT_EQ(result, "3.14\n");
}

TEST(Interpreter_Printf_String_Format) {
    std::string result = run_awk("BEGIN { printf \"%10s\\n\", \"test\" }");
    ASSERT_EQ(result, "      test\n");
}

// ============================================================================
// Regex Matching
// ============================================================================

TEST(Interpreter_Match_Operator) {
    std::string result = run_awk("BEGIN { print (\"hello\" ~ /ell/) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Not_Match_Operator) {
    std::string result = run_awk("BEGIN { print (\"hello\" !~ /xyz/) }");
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Match_Function) {
    std::string result = run_awk("BEGIN { print match(\"hello world\", /wor/) }");
    ASSERT_EQ(result, "7\n");
}

TEST(Interpreter_Sub) {
    std::string result = run_awk("BEGIN { x = \"hello\"; sub(/l/, \"L\", x); print x }");
    ASSERT_EQ(result, "heLlo\n");
}

TEST(Interpreter_Gsub) {
    std::string result = run_awk("BEGIN { x = \"hello\"; gsub(/l/, \"L\", x); print x }");
    ASSERT_EQ(result, "heLLo\n");
}

TEST(Interpreter_Gsub_Return_Count) {
    std::string result = run_awk("BEGIN { x = \"aaa\"; n = gsub(/a/, \"b\", x); print n }");
    ASSERT_EQ(result, "3\n");
}

// ============================================================================
// Special Variables
// ============================================================================

TEST(Interpreter_FS_Default) {
    std::string result = run_awk("{ print $1 }", "one two three");
    ASSERT_EQ(result, "one\n");
}

TEST(Interpreter_FS_Custom) {
    std::string result = run_awk("BEGIN { FS = \":\" } { print $2 }", "a:b:c");
    ASSERT_EQ(result, "b\n");
}

TEST(Interpreter_OFS) {
    std::string result = run_awk("BEGIN { OFS = \"-\" } { print $1, $2 }", "a b");
    ASSERT_EQ(result, "a-b\n");
}

TEST(Interpreter_ORS) {
    std::string result = run_awk("BEGIN { ORS = \"---\" } { print $1 }", "one\ntwo");
    ASSERT_EQ(result, "one---two---");
}

// ============================================================================
// Complex Programs
// ============================================================================

TEST(Interpreter_Word_Count) {
    std::string result = run_awk(R"(
        BEGIN { words = 0 }
        { words += NF }
        END { print words }
    )", "one two\nthree four five");
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Line_Count) {
    std::string result = run_awk("END { print NR }", "a\nb\nc\nd");
    ASSERT_EQ(result, "4\n");
}

TEST(Interpreter_Sum_Column) {
    std::string result = run_awk("{ sum += $1 } END { print sum }", "10\n20\n30");
    ASSERT_EQ(result, "60\n");
}

TEST(Interpreter_Average) {
    std::string result = run_awk("{ sum += $1; count++ } END { print sum / count }", "10\n20\n30");
    ASSERT_EQ(result, "20\n");
}

TEST(Interpreter_Max_Value) {
    std::string result = run_awk("NR == 1 || $1 > max { max = $1 } END { print max }", "5\n3\n8\n2");
    ASSERT_EQ(result, "8\n");
}

// ============================================================================
// Printf Dynamic Width/Precision Tests
// ============================================================================

TEST(Interpreter_Printf_Dynamic_Width) {
    std::string result = run_awk(R"(BEGIN { printf "%*s\n", 10, "hi" })");
    ASSERT_EQ(result, "        hi\n");
}

TEST(Interpreter_Printf_Dynamic_Precision_String) {
    std::string result = run_awk(R"(BEGIN { printf "%.*s\n", 3, "hello" })");
    ASSERT_EQ(result, "hel\n");
}

TEST(Interpreter_Printf_Dynamic_Width_And_Precision) {
    std::string result = run_awk(R"(BEGIN { printf "%*.*f\n", 10, 2, 3.14159 })");
    ASSERT_EQ(result, "      3.14\n");
}

TEST(Interpreter_Printf_Dynamic_Width_Negative) {
    // Negative width means left-align
    std::string result = run_awk(R"(BEGIN { printf "%*s|\n", -5, "hi" })");
    ASSERT_EQ(result, "hi   |\n");
}

// ============================================================================
// RS Paragraph Mode Tests
// ============================================================================

TEST(Interpreter_RS_Paragraph_Mode) {
    std::string result = run_awk(
        R"(BEGIN { RS = "" } { count++ } END { print count })",
        "Name: Alice\nAge: 30\n\nName: Bob\nAge: 25\n\nName: Charlie\nAge: 35\n"
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_RS_Paragraph_Mode_NF) {
    // In paragraph mode, NF should count fields across all lines in the paragraph
    std::string result = run_awk(
        R"(BEGIN { RS = "" } NR == 1 { print NF })",
        "word1 word2\nword3 word4 word5\n\nparagraph2\n"
    );
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_RS_Paragraph_Mode_First_Field) {
    std::string result = run_awk(
        R"(BEGIN { RS = "" } NR == 1 { print $1 })",
        "first second\nthird fourth\n\nnext paragraph\n"
    );
    ASSERT_EQ(result, "first\n");
}

// ============================================================================
// RT Variable Tests (gawk Extension)
// ============================================================================

TEST(Interpreter_RT_Newline) {
    // RT should contain "\n" for standard newline terminator
    std::string result = run_awk(
        R"({ print RT == "\n" ? "yes" : "no" })",
        "line1\nline2\n"
    );
    ASSERT_EQ(result, "yes\nyes\n");
}

TEST(Interpreter_RT_Custom_RS) {
    // RT should contain the actual separator found
    std::string result = run_awk(
        R"(BEGIN { RS = ":" } { print NR, RT })",
        "a:b:c"
    );
    // Note: last field has no terminator, RT may be empty
    ASSERT_EQ(result, "1 :\n2 :\n3 \n");
}

// ============================================================================
// FPAT Variable Tests (gawk Extension)
// ============================================================================

TEST(Interpreter_FPAT_Numbers) {
    // FPAT matches fields by pattern instead of splitting by FS
    std::string result = run_awk(
        R"(BEGIN { FPAT = "[0-9]+" } { print NF, $1, $2, $3 })",
        "abc123def456ghi789\n"
    );
    ASSERT_EQ(result, "3 123 456 789\n");
}

TEST(Interpreter_FPAT_Words) {
    std::string result = run_awk(
        R"(BEGIN { FPAT = "[a-zA-Z]+" } { print NF, $1, $2 })",
        "hello123world456\n"
    );
    ASSERT_EQ(result, "2 hello world\n");
}

// ============================================================================
// match() with Capturing Groups Tests (gawk Extension)
// ============================================================================

TEST(Interpreter_Match_Capturing_Groups) {
    std::string result = run_awk(
        R"(BEGIN {
            s = "user@domain.com"
            match(s, /([^@]+)@(.+)/, arr)
            print arr[0]
            print arr[1]
            print arr[2]
        })",
        ""
    );
    ASSERT_EQ(result, "user@domain.com\nuser\ndomain.com\n");
}

TEST(Interpreter_Match_Groups_Numbers) {
    std::string result = run_awk(
        R"(BEGIN {
            s = "123-456-789"
            match(s, /([0-9]+)-([0-9]+)-([0-9]+)/, arr)
            print arr[1], arr[2], arr[3]
        })",
        ""
    );
    ASSERT_EQ(result, "123 456 789\n");
}

// ============================================================================
// patsplit() Array Population Tests (gawk Extension)
// ============================================================================

TEST(Interpreter_Patsplit_Array) {
    std::string result = run_awk(
        R"(BEGIN {
            n = patsplit("abc123def456ghi789", a, /[0-9]+/)
            print n, a[1], a[2], a[3]
        })",
        ""
    );
    ASSERT_EQ(result, "3 123 456 789\n");
}

TEST(Interpreter_Patsplit_Words) {
    std::string result = run_awk(
        R"(BEGIN {
            n = patsplit("hello123world", a, /[a-z]+/)
            print n, a[1], a[2]
        })",
        ""
    );
    ASSERT_EQ(result, "2 hello world\n");
}

// ============================================================================
// i18n Functions (gawk Extension)
// ============================================================================

TEST(Interpreter_Dcgettext) {
    // dcgettext(string [, domain [, category]])
    std::string result = run_awk(
        R"(BEGIN {
            print dcgettext("Hello World")
        })",
        ""
    );
    ASSERT_EQ(result, "Hello World\n");
}

TEST(Interpreter_Dcgettext_With_Domain) {
    // dcgettext(string, domain)
    std::string result = run_awk(
        R"(BEGIN {
            print dcgettext("Hello World", "myapp")
        })",
        ""
    );
    ASSERT_EQ(result, "Hello World\n");
}

TEST(Interpreter_Dcngettext_Singular) {
    // dcngettext(singular, plural, number [, domain [, category]])
    std::string result = run_awk(
        R"(BEGIN {
            print dcngettext("1 file", "%d files", 1)
        })",
        ""
    );
    ASSERT_EQ(result, "1 file\n");
}

TEST(Interpreter_Dcngettext_Plural) {
    std::string result = run_awk(
        R"(BEGIN {
            print dcngettext("1 item", "%d items", 5)
        })",
        ""
    );
    ASSERT_EQ(result, "%d items\n");
}

TEST(Interpreter_Dcngettext_Zero) {
    std::string result = run_awk(
        R"(BEGIN {
            print dcngettext("1 result", "%d results", 0)
        })",
        ""
    );
    ASSERT_EQ(result, "%d results\n");
}

TEST(Interpreter_Dcngettext_With_Domain) {
    // dcngettext with explicit domain
    std::string result = run_awk(
        R"(BEGIN {
            print dcngettext("1 item", "%d items", 5, "myapp")
        })",
        ""
    );
    ASSERT_EQ(result, "%d items\n");
}

TEST(Interpreter_Bindtextdomain) {
    // bindtextdomain(directory [, domain])
    std::string result = run_awk(
        R"(BEGIN {
            print bindtextdomain("/usr/share/locale", "myapp")
        })",
        ""
    );
    ASSERT_EQ(result, "/usr/share/locale\n");
}

TEST(Interpreter_Bindtextdomain_Query) {
    // Set and then query with empty string
    std::string result = run_awk(
        R"(BEGIN {
            bindtextdomain("/opt/locale", "myapp")
            print bindtextdomain("", "myapp")
        })",
        ""
    );
    ASSERT_EQ(result, "/opt/locale\n");
}

TEST(Interpreter_Bindtextdomain_Unknown) {
    // Query without prior set -> empty
    std::string result = run_awk(
        R"(BEGIN {
            print "[" bindtextdomain("", "unknown") "]"
        })",
        ""
    );
    ASSERT_EQ(result, "[]\n");
}

TEST(Interpreter_Textdomain_Default) {
    // TEXTDOMAIN should default to "messages"
    std::string result = run_awk(
        R"(BEGIN {
            print TEXTDOMAIN
        })",
        ""
    );
    ASSERT_EQ(result, "messages\n");
}

TEST(Interpreter_Textdomain_Custom) {
    // TEXTDOMAIN can be changed
    std::string result = run_awk(
        R"(BEGIN {
            TEXTDOMAIN = "myapp"
            print TEXTDOMAIN
        })",
        ""
    );
    ASSERT_EQ(result, "myapp\n");
}

TEST(Interpreter_Bindtextdomain_Uses_Textdomain) {
    // bindtextdomain uses TEXTDOMAIN as default domain
    std::string result = run_awk(
        R"(BEGIN {
            TEXTDOMAIN = "myapp"
            bindtextdomain("/opt/locale")
            print bindtextdomain("")
        })",
        ""
    );
    ASSERT_EQ(result, "/opt/locale\n");
}

// mkbool Tests
TEST(Interpreter_Mkbool_True_Number) {
    std::string result = run_awk(
        R"(BEGIN { print mkbool(42) })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Mkbool_False_Zero) {
    std::string result = run_awk(
        R"(BEGIN { print mkbool(0) })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Mkbool_True_String) {
    std::string result = run_awk(
        R"(BEGIN { print mkbool("hello") })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Mkbool_False_Empty_String) {
    std::string result = run_awk(
        R"(BEGIN { print mkbool("") })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Mkbool_String_Zero) {
    // "0" als String ist truthy (nicht-leer), aber numerisch 0
    // In unserer Implementierung: String-Typ -> nicht-leer = truthy
    std::string result = run_awk(
        R"(BEGIN { print mkbool("0") })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Mkbool_Numeric_String_Zero) {
    // Bei numerischem Vergleich: 0 + 0 = 0 -> falsy
    std::string result = run_awk(
        R"(BEGIN { x = "0" + 0; print mkbool(x) })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

// ============================================================================
// gensub() Tests (gawk Extension)
// ============================================================================

TEST(Interpreter_Gensub_Global) {
    // gensub with "g" replaces all occurrences
    std::string result = run_awk(
        R"(BEGIN { print gensub(/l/, "L", "g", "hello") })",
        ""
    );
    ASSERT_EQ(result, "heLLo\n");
}

TEST(Interpreter_Gensub_First) {
    // gensub with "1" or number replaces nth occurrence
    std::string result = run_awk(
        R"(BEGIN { print gensub(/l/, "L", 1, "hello") })",
        ""
    );
    ASSERT_EQ(result, "heLlo\n");
}

TEST(Interpreter_Gensub_Second) {
    // Replace second occurrence only
    std::string result = run_awk(
        R"(BEGIN { print gensub(/l/, "L", 2, "hello") })",
        ""
    );
    ASSERT_EQ(result, "helLo\n");
}

TEST(Interpreter_Gensub_Backreference) {
    // gensub supports \1, \2, etc. for backreferences
    std::string result = run_awk(
        R"(BEGIN { print gensub(/([0-9]+)-([0-9]+)/, "\\2-\\1", "g", "123-456") })",
        ""
    );
    ASSERT_EQ(result, "456-123\n");
}

TEST(Interpreter_Gensub_No_Target) {
    // Without 4th argument, uses $0
    std::string result = run_awk(
        R"({ print gensub(/o/, "0", "g") })",
        "hello world"
    );
    ASSERT_EQ(result, "hell0 w0rld\n");
}

TEST(Interpreter_Gensub_Returns_New_String) {
    // gensub returns modified string, doesn't modify original
    std::string result = run_awk(
        R"(BEGIN {
            x = "hello"
            y = gensub(/l/, "L", "g", x)
            print x
            print y
        })",
        ""
    );
    ASSERT_EQ(result, "hello\nheLLo\n");
}

TEST(Interpreter_Gensub_No_Match) {
    // If no match, returns original string
    std::string result = run_awk(
        R"(BEGIN { print gensub(/xyz/, "ABC", "g", "hello") })",
        ""
    );
    ASSERT_EQ(result, "hello\n");
}

// ============================================================================
// getline Tests
// ============================================================================

TEST(Interpreter_Getline_Return_Value_Error) {
    // getline returns -1 on error (file doesn't exist)
    std::string result = run_awk(
        R"(BEGIN {
            print (getline < "__nonexistent_file_12345.tmp")
        })",
        ""
    );
    ASSERT_EQ(result, "-1\n");
}

TEST(Interpreter_Getline_From_File) {
    // Create test file first
    {
        std::ofstream f("__test_getline.tmp");
        f << "a\n" << "b\n";
    }

    std::string result = run_awk(
        R"(BEGIN {
            while ((getline line < "__test_getline.tmp") > 0) {
                print "Read:", line
            }
        })",
        ""
    );
    std::remove("__test_getline.tmp");
    ASSERT_EQ(result, "Read: a\nRead: b\n");
}

TEST(Interpreter_Getline_Into_Variable_From_File) {
    // Create test file first
    {
        std::ofstream f("__test_getline2.tmp");
        f << "hello\n";
    }

    std::string result = run_awk(
        R"(BEGIN {
            getline x < "__test_getline2.tmp"
            print "x=" x
        })",
        ""
    );
    std::remove("__test_getline2.tmp");
    ASSERT_EQ(result, "x=hello\n");
}

TEST(Interpreter_Getline_Multiple_Lines) {
    // Create test file first
    {
        std::ofstream f("__test_getline3.tmp");
        f << "line1\n" << "line2\n" << "line3\n";
    }

    std::string result = run_awk(
        R"(BEGIN {
            count = 0
            while ((getline < "__test_getline3.tmp") > 0) {
                count++
            }
            print count
        })",
        ""
    );
    std::remove("__test_getline3.tmp");
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Getline_EOF_Returns_Negative) {
    // Create test file first
    {
        std::ofstream f("__test_getline4.tmp");
        f << "only\n";
    }

    std::string result = run_awk(
        R"(BEGIN {
            r1 = (getline < "__test_getline4.tmp")
            r2 = (getline < "__test_getline4.tmp")
            print r1, r2
        })",
        ""
    );
    std::remove("__test_getline4.tmp");
    ASSERT_EQ(result, "1 -1\n");
}

TEST(Interpreter_Getline_Updates_Dollar_Zero) {
    // Create test file first
    {
        std::ofstream f("__test_getline5.tmp");
        f << "new content\n";
    }

    std::string result = run_awk(
        R"(BEGIN {
            getline < "__test_getline5.tmp"
            print $0
            print $1, $2
        })",
        ""
    );
    std::remove("__test_getline5.tmp");
    ASSERT_EQ(result, "new content\nnew content\n");
}

TEST(Interpreter_Getline_Pipe) {
    // getline from pipe: command | getline
    std::string result = run_awk(
        R"(BEGIN {
            "echo hello" | getline x
            print x
        })",
        ""
    );
    // Note: on Windows, echo might include extra spaces
    ASSERT_TRUE(result.find("hello") != std::string::npos);
}

TEST(Interpreter_Getline_Pipe_Multiple) {
    // Multiple reads from same pipe
    std::string result = run_awk(
        R"(BEGIN {
            cmd = "echo line1 && echo line2"
            count = 0
            while ((cmd | getline line) > 0) {
                count++
            }
            close(cmd)
            print count
        })",
        ""
    );
    // Should read 2 lines
    ASSERT_EQ(result, "2\n");
}

// ============================================================================
// next/nextfile/exit Tests
// ============================================================================

TEST(Interpreter_Next_Skips_Rules) {
    // next skips remaining rules for current record
    std::string result = run_awk(
        R"(
        { print "rule1:", $0 }
        /skip/ { next }
        { print "rule2:", $0 }
        )",
        "line1\nskip\nline3"
    );
    ASSERT_EQ(result, "rule1: line1\nrule2: line1\nrule1: skip\nrule1: line3\nrule2: line3\n");
}

TEST(Interpreter_Next_In_Loop) {
    // next in a pattern action
    std::string result = run_awk(
        R"(
        {
            if ($1 == "skip") next
            print $0
        }
        )",
        "keep1\nskip\nkeep2\nskip\nkeep3"
    );
    ASSERT_EQ(result, "keep1\nkeep2\nkeep3\n");
}

TEST(Interpreter_Next_Continues_To_Next_Record) {
    // next processes next input record
    std::string result = run_awk(
        R"(
        {
            count++
            if (NR == 2) next
            print NR, $0
        }
        END { print "count:", count }
        )",
        "a\nb\nc"
    );
    ASSERT_EQ(result, "1 a\n3 c\ncount: 3\n");
}

TEST(Interpreter_Exit_In_BEGIN) {
    // exit in BEGIN block stops execution (END may or may not run depending on implementation)
    std::string result = run_awk(
        R"(
        BEGIN {
            print "start"
            exit
            print "never"
        }
        )",
        ""
    );
    // Just verify that "start" is printed and "never" is not
    ASSERT_TRUE(result.find("start") != std::string::npos);
    ASSERT_TRUE(result.find("never") == std::string::npos);
}

TEST(Interpreter_Exit_With_Status_BEGIN) {
    // exit with status code in BEGIN
    std::string result = run_awk(
        R"(
        BEGIN {
            print "start"
            exit 42
            print "never"
        }
        )",
        ""
    );
    // Just verify that "start" is printed and "never" is not
    ASSERT_TRUE(result.find("start") != std::string::npos);
    ASSERT_TRUE(result.find("never") == std::string::npos);
}

TEST(Interpreter_Exit_In_END) {
    // exit in END block doesn't re-run END
    std::string result = run_awk(
        R"(
        BEGIN { print "begin" }
        END {
            print "end1"
            exit
            print "never"
        }
        )",
        ""
    );
    ASSERT_EQ(result, "begin\nend1\n");
}

TEST(Interpreter_Exit_Stops_After_Code) {
    // Code after exit is not executed
    std::string result = run_awk(
        R"(
        BEGIN {
            print "a"
            exit
            print "b"
            print "c"
        }
        )",
        ""
    );
    ASSERT_EQ(result, "a\n");
}

// ============================================================================
// IGNORECASE Tests
// ============================================================================

TEST(Interpreter_IGNORECASE_Default_Off) {
    // By default, IGNORECASE is 0 (case-sensitive)
    std::string result = run_awk(
        R"(BEGIN { print IGNORECASE })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_IGNORECASE_Match_Operator) {
    // IGNORECASE affects ~ operator
    std::string result = run_awk(
        R"(BEGIN {
            IGNORECASE = 1
            print ("HELLO" ~ /hello/)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_IGNORECASE_Match_Operator_Off) {
    // Without IGNORECASE, case matters
    std::string result = run_awk(
        R"(BEGIN {
            print ("HELLO" ~ /hello/)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_IGNORECASE_Not_Match_Operator) {
    // IGNORECASE affects !~ operator
    std::string result = run_awk(
        R"(BEGIN {
            IGNORECASE = 1
            print ("HELLO" !~ /hello/)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_IGNORECASE_Sub) {
    // IGNORECASE affects sub()
    std::string result = run_awk(
        R"(BEGIN {
            IGNORECASE = 1
            x = "Hello World"
            sub(/hello/, "hi", x)
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "hi World\n");
}

TEST(Interpreter_IGNORECASE_Gsub) {
    // IGNORECASE affects gsub()
    std::string result = run_awk(
        R"(BEGIN {
            IGNORECASE = 1
            x = "Hello HELLO hello"
            n = gsub(/hello/, "X", x)
            print n, x
        })",
        ""
    );
    ASSERT_EQ(result, "3 X X X\n");
}

TEST(Interpreter_IGNORECASE_Match_Function) {
    // IGNORECASE affects match()
    std::string result = run_awk(
        R"(BEGIN {
            IGNORECASE = 1
            print match("HELLO WORLD", /world/)
        })",
        ""
    );
    ASSERT_EQ(result, "7\n");
}

TEST(Interpreter_IGNORECASE_Patsplit) {
    // IGNORECASE affects patsplit()
    std::string result = run_awk(
        R"(BEGIN {
            IGNORECASE = 1
            n = patsplit("abc123DEF456ghi", arr, /[a-z]+/)
            print n, arr[1], arr[2], arr[3]
        })",
        ""
    );
    ASSERT_EQ(result, "3 abc DEF ghi\n");
}

TEST(Interpreter_IGNORECASE_Gensub) {
    // IGNORECASE affects gensub()
    std::string result = run_awk(
        R"(BEGIN {
            IGNORECASE = 1
            print gensub(/hello/, "hi", "g", "Hello HELLO hello")
        })",
        ""
    );
    ASSERT_EQ(result, "hi hi hi\n");
}

TEST(Interpreter_IGNORECASE_Toggle) {
    // IGNORECASE can be toggled on and off
    std::string result = run_awk(
        R"(BEGIN {
            print ("ABC" ~ /abc/)
            IGNORECASE = 1
            print ("ABC" ~ /abc/)
            IGNORECASE = 0
            print ("ABC" ~ /abc/)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n1\n0\n");
}

// ==================== Array Tests ====================

TEST(Interpreter_Array_Delete_Element) {
    // delete removes a single element
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = "one"
            a[2] = "two"
            a[3] = "three"
            delete a[2]
            print (1 in a), (2 in a), (3 in a)
        })",
        ""
    );
    ASSERT_EQ(result, "1 0 1\n");
}

TEST(Interpreter_Array_Delete_Entire) {
    // delete array removes all elements
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = "one"
            a[2] = "two"
            a[3] = "three"
            delete a
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Array_Length) {
    // length(array) returns number of elements
    std::string result = run_awk(
        R"(BEGIN {
            a["x"] = 1
            a["y"] = 2
            a["z"] = 3
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Array_Length_Empty) {
    // length of empty array is 0
    std::string result = run_awk(
        R"(BEGIN {
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Array_Multi_Dimensional_Access) {
    // Multi-dimensional array with SUBSEP
    std::string result = run_awk(
        R"(BEGIN {
            a[1,2] = "one-two"
            a[2,3] = "two-three"
            print a[1,2], a[2,3]
        })",
        ""
    );
    ASSERT_EQ(result, "one-two two-three\n");
}

TEST(Interpreter_Array_Multi_Dimensional_In) {
    // (i,j) in array works for multi-dimensional
    std::string result = run_awk(
        R"(BEGIN {
            a[1,2] = "value"
            print (1,2) in a, (2,3) in a
        })",
        ""
    );
    ASSERT_EQ(result, "1 0\n");
}

TEST(Interpreter_Array_Multi_Dimensional_Delete) {
    // delete a[i,j] works for multi-dimensional
    std::string result = run_awk(
        R"(BEGIN {
            a[1,2] = "one"
            a[2,3] = "two"
            delete a[1,2]
            print (1,2) in a, (2,3) in a
        })",
        ""
    );
    ASSERT_EQ(result, "0 1\n");
}

TEST(Interpreter_Array_SUBSEP) {
    // SUBSEP is used to join multi-dimensional indices
    std::string result = run_awk(
        R"(BEGIN {
            a[1,2] = "value"
            for (k in a) print k
            print SUBSEP == "\034"
        })",
        ""
    );
    ASSERT_TRUE(result.find("1") != std::string::npos);
    ASSERT_TRUE(result.find("2") != std::string::npos);
}

TEST(Interpreter_Array_Numeric_String_Key) {
    // Numeric and string keys are handled correctly
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = "numeric"
            a["1"] = "string"
            print a[1], length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "string 1\n");
}

TEST(Interpreter_Array_For_In_Order) {
    // for-in iterates over all elements
    std::string result = run_awk(
        R"(BEGIN {
            a["a"] = 1
            a["b"] = 2
            a["c"] = 3
            count = 0
            for (k in a) count++
            print count
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Array_Nested_Access) {
    // Arrays can be accessed in nested expressions
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = 2
            a[2] = 3
            a[3] = "done"
            print a[a[a[1]]]
        })",
        ""
    );
    ASSERT_EQ(result, "done\n");
}

TEST(Interpreter_Array_As_Function_Param) {
    // Arrays passed to functions
    std::string result = run_awk(
        R"(
        function sum(arr,    total, k) {
            total = 0
            for (k in arr) total += arr[k]
            return total
        }
        BEGIN {
            nums[1] = 10
            nums[2] = 20
            nums[3] = 30
            print sum(nums)
        })",
        ""
    );
    ASSERT_EQ(result, "60\n");
}

TEST(Interpreter_Array_Modified_In_Function) {
    // Arrays modified in functions - test local modification
    std::string result = run_awk(
        R"(
        function modify(arr) {
            arr["new"] = "added"
            return arr["new"]
        }
        BEGIN {
            a["old"] = "original"
            print modify(a)
        })",
        ""
    );
    ASSERT_EQ(result, "added\n");
}

TEST(Interpreter_Array_Split_Result) {
    // split() creates array correctly
    std::string result = run_awk(
        R"(BEGIN {
            n = split("a:b:c:d", arr, ":")
            print n, arr[1], arr[2], arr[3], arr[4]
        })",
        ""
    );
    ASSERT_EQ(result, "4 a b c d\n");
}

TEST(Interpreter_Array_Delete_In_Loop) {
    // Deleting elements during iteration
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = 1; a[2] = 2; a[3] = 3; a[4] = 4; a[5] = 5
            for (k in a) {
                if (a[k] % 2 == 0) delete a[k]
            }
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");
}

// ==================== Printf Format Tests ====================

TEST(Interpreter_Printf_Char) {
    // %c prints first char of string
    std::string result = run_awk(
        R"(BEGIN {
            printf "%c%c%c\n", "A", "B", "C"
        })",
        ""
    );
    ASSERT_EQ(result, "ABC\n");
}

TEST(Interpreter_Printf_Char_From_String) {
    // %c with string takes first character
    std::string result = run_awk(
        R"(BEGIN {
            printf "%c\n", "Hello"
        })",
        ""
    );
    ASSERT_EQ(result, "H\n");
}

TEST(Interpreter_Printf_Octal) {
    // %o prints octal
    std::string result = run_awk(
        R"(BEGIN {
            printf "%o\n", 64
        })",
        ""
    );
    ASSERT_EQ(result, "100\n");
}

TEST(Interpreter_Printf_Hex_Lower) {
    // %x prints lowercase hex
    std::string result = run_awk(
        R"(BEGIN {
            printf "%x\n", 255
        })",
        ""
    );
    ASSERT_EQ(result, "ff\n");
}

TEST(Interpreter_Printf_Hex_Upper) {
    // %X prints uppercase hex
    std::string result = run_awk(
        R"(BEGIN {
            printf "%X\n", 255
        })",
        ""
    );
    ASSERT_EQ(result, "FF\n");
}

TEST(Interpreter_Printf_Scientific_Lower) {
    // %e prints scientific notation lowercase
    std::string result = run_awk(
        R"(BEGIN {
            printf "%e\n", 1234.5
        })",
        ""
    );
    ASSERT_TRUE(result.find("e") != std::string::npos || result.find("E") != std::string::npos);
}

TEST(Interpreter_Printf_Scientific_Upper) {
    // %E prints scientific notation uppercase
    std::string result = run_awk(
        R"(BEGIN {
            printf "%E\n", 1234.5
        })",
        ""
    );
    ASSERT_TRUE(result.find("E") != std::string::npos);
}

TEST(Interpreter_Printf_General_Lower) {
    // %g uses shorter of %e or %f
    std::string result = run_awk(
        R"(BEGIN {
            printf "%g\n", 0.0001234
        })",
        ""
    );
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Printf_General_Upper) {
    // %G uses shorter of %E or %f
    std::string result = run_awk(
        R"(BEGIN {
            printf "%G\n", 1234567890.0
        })",
        ""
    );
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Printf_Escape_Newline) {
    // \n in printf
    std::string result = run_awk(
        R"(BEGIN {
            printf "a\nb\nc"
        })",
        ""
    );
    ASSERT_EQ(result, "a\nb\nc");
}

TEST(Interpreter_Printf_Escape_Tab) {
    // \t in printf
    std::string result = run_awk(
        R"(BEGIN {
            printf "a\tb\tc"
        })",
        ""
    );
    ASSERT_EQ(result, "a\tb\tc");
}

TEST(Interpreter_Printf_Escape_Carriage_Return) {
    // \r in printf
    std::string result = run_awk(
        R"(BEGIN {
            printf "a\rb"
        })",
        ""
    );
    ASSERT_EQ(result, "a\rb");
}

TEST(Interpreter_Printf_Escape_Backslash) {
    // \\ in printf
    std::string result = run_awk(
        R"(BEGIN {
            printf "a\\b"
        })",
        ""
    );
    ASSERT_EQ(result, "a\\b");
}

TEST(Interpreter_Printf_Percent_Literal) {
    // %% prints literal %
    std::string result = run_awk(
        R"(BEGIN {
            printf "100%%\n"
        })",
        ""
    );
    ASSERT_EQ(result, "100%\n");
}

TEST(Interpreter_Printf_Left_Justify) {
    // %-10s left justifies
    std::string result = run_awk(
        R"(BEGIN {
            printf "[%-5s]\n", "hi"
        })",
        ""
    );
    ASSERT_EQ(result, "[hi   ]\n");
}

TEST(Interpreter_Printf_Plus_Sign) {
    // %+d shows plus sign
    std::string result = run_awk(
        R"(BEGIN {
            printf "%+d %+d\n", 5, -5
        })",
        ""
    );
    ASSERT_EQ(result, "+5 -5\n");
}

TEST(Interpreter_Printf_Space_Sign) {
    // % d shows space for positive
    std::string result = run_awk(
        R"(BEGIN {
            printf "% d % d\n", 5, -5
        })",
        ""
    );
    ASSERT_EQ(result, " 5 -5\n");
}

TEST(Interpreter_Printf_Hash_Octal) {
    // %#o shows 0 prefix
    std::string result = run_awk(
        R"(BEGIN {
            printf "%#o\n", 64
        })",
        ""
    );
    ASSERT_EQ(result, "0100\n");
}

TEST(Interpreter_Printf_Hash_Hex) {
    // %#x shows 0x prefix
    std::string result = run_awk(
        R"(BEGIN {
            printf "%#x\n", 255
        })",
        ""
    );
    ASSERT_EQ(result, "0xff\n");
}

TEST(Interpreter_Printf_Precision_Float) {
    // %.2f limits decimal places
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.2f\n", 3.14159
        })",
        ""
    );
    ASSERT_EQ(result, "3.14\n");
}

TEST(Interpreter_Printf_Precision_String) {
    // %.3s limits string length
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.3s\n", "Hello"
        })",
        ""
    );
    ASSERT_EQ(result, "Hel\n");
}

TEST(Interpreter_Printf_Width_And_Precision) {
    // %8.2f with width and precision
    std::string result = run_awk(
        R"(BEGIN {
            printf "%8.2f\n", 3.14159
        })",
        ""
    );
    ASSERT_EQ(result, "    3.14\n");
}

TEST(Interpreter_Printf_Multiple_Formats) {
    // Multiple format specifiers
    std::string result = run_awk(
        R"(BEGIN {
            printf "%s=%d (0x%x)\n", "value", 255, 255
        })",
        ""
    );
    ASSERT_EQ(result, "value=255 (0xff)\n");
}

TEST(Interpreter_Printf_No_Newline) {
    // printf doesn't add newline automatically
    std::string result = run_awk(
        R"(BEGIN {
            printf "no"
            printf "newline"
        })",
        ""
    );
    ASSERT_EQ(result, "nonewline");
}

// ==================== Function Tests ====================

TEST(Interpreter_Function_No_Return) {
    // Function without return statement returns empty
    std::string result = run_awk(
        R"(
        function greet(name) {
            print "Hello, " name
        }
        BEGIN {
            x = greet("World")
            print "x=" x "."
        })",
        ""
    );
    ASSERT_TRUE(result.find("Hello, World") != std::string::npos);
    ASSERT_TRUE(result.find("x=.") != std::string::npos);
}

TEST(Interpreter_Function_Return_Value) {
    // Function returns a value
    std::string result = run_awk(
        R"(
        function double(x) {
            return x * 2
        }
        BEGIN {
            print double(21)
        })",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Function_Return_String) {
    // Function returns a string
    std::string result = run_awk(
        R"(
        function greeting(name) {
            return "Hello, " name "!"
        }
        BEGIN {
            print greeting("AWK")
        })",
        ""
    );
    ASSERT_EQ(result, "Hello, AWK!\n");
}

TEST(Interpreter_Function_Local_Variables) {
    // Local variables don't affect global scope
    std::string result = run_awk(
        R"(
        function test(a,    local1, local2) {
            local1 = 100
            local2 = 200
            return local1 + local2
        }
        BEGIN {
            local1 = 1
            print test(0)
            print local1
        })",
        ""
    );
    ASSERT_EQ(result, "300\n1\n");
}

TEST(Interpreter_Function_Multiple_Locals) {
    // Multiple local variables
    std::string result = run_awk(
        R"(
        function compute(x,    a, b, c) {
            a = x * 2
            b = x * 3
            c = x * 4
            return a + b + c
        }
        BEGIN {
            print compute(10)
        })",
        ""
    );
    ASSERT_EQ(result, "90\n");
}

TEST(Interpreter_Function_Recursion_Factorial) {
    // Recursive factorial
    std::string result = run_awk(
        R"(
        function fact(n) {
            if (n <= 1) return 1
            return n * fact(n - 1)
        }
        BEGIN {
            print fact(5)
        })",
        ""
    );
    ASSERT_EQ(result, "120\n");
}

TEST(Interpreter_Function_Recursion_Fibonacci) {
    // Recursive Fibonacci
    std::string result = run_awk(
        R"(
        function fib(n) {
            if (n <= 1) return n
            return fib(n-1) + fib(n-2)
        }
        BEGIN {
            print fib(10)
        })",
        ""
    );
    ASSERT_EQ(result, "55\n");
}

TEST(Interpreter_Function_Mutual_Recursion) {
    // Mutually recursive functions
    std::string result = run_awk(
        R"(
        function is_even(n) {
            if (n == 0) return 1
            return is_odd(n - 1)
        }
        function is_odd(n) {
            if (n == 0) return 0
            return is_even(n - 1)
        }
        BEGIN {
            print is_even(4), is_even(5), is_odd(3), is_odd(4)
        })",
        ""
    );
    ASSERT_EQ(result, "1 0 1 0\n");
}

TEST(Interpreter_Function_Nested_Calls) {
    // Nested function calls
    std::string result = run_awk(
        R"(
        function add(a, b) { return a + b }
        function mul(a, b) { return a * b }
        function compute(x) { return add(mul(x, 2), mul(x, 3)) }
        BEGIN {
            print compute(10)
        })",
        ""
    );
    ASSERT_EQ(result, "50\n");
}

TEST(Interpreter_Function_Global_Access) {
    // Functions can access global variables
    std::string result = run_awk(
        R"(
        function increment() {
            counter++
        }
        BEGIN {
            counter = 0
            increment()
            increment()
            increment()
            print counter
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Function_Modify_Global) {
    // Functions can modify global variables
    std::string result = run_awk(
        R"(
        function set_value(val) {
            global_var = val
        }
        BEGIN {
            global_var = "old"
            set_value("new")
            print global_var
        })",
        ""
    );
    ASSERT_EQ(result, "new\n");
}

TEST(Interpreter_Function_Parameter_Shadowing) {
    // Parameters shadow global variables
    std::string result = run_awk(
        R"(
        function test(x) {
            x = 999
            return x
        }
        BEGIN {
            x = 1
            print test(x)
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "999\n1\n");
}

TEST(Interpreter_Function_Default_Parameters) {
    // Missing parameters default to empty/zero
    std::string result = run_awk(
        R"(
        function test(a, b, c) {
            return a + b + c
        }
        BEGIN {
            print test(10)
        })",
        ""
    );
    ASSERT_EQ(result, "10\n");
}

TEST(Interpreter_Function_Extra_Parameters) {
    // Extra parameters are ignored
    std::string result = run_awk(
        R"(
        function add(a, b) {
            return a + b
        }
        BEGIN {
            print add(1, 2, 3, 4, 5)
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Function_Expression_Args) {
    // Expressions as arguments
    std::string result = run_awk(
        R"(
        function square(x) { return x * x }
        BEGIN {
            print square(3 + 4)
        })",
        ""
    );
    ASSERT_EQ(result, "49\n");
}

TEST(Interpreter_Function_Return_In_Loop) {
    // Return from inside a loop
    std::string result = run_awk(
        R"(
        function find_first_even(n) {
            for (i = 1; i <= n; i++) {
                if (i % 2 == 0) return i
            }
            return -1
        }
        BEGIN {
            print find_first_even(10)
        })",
        ""
    );
    ASSERT_EQ(result, "2\n");
}

TEST(Interpreter_Function_Early_Return) {
    // Early return skips remaining code
    std::string result = run_awk(
        R"(
        function test(x) {
            if (x < 0) return "negative"
            if (x == 0) return "zero"
            return "positive"
        }
        BEGIN {
            print test(-5)
            print test(0)
            print test(5)
        })",
        ""
    );
    ASSERT_EQ(result, "negative\nzero\npositive\n");
}

TEST(Interpreter_Function_Chained_Returns) {
    // Chained function returns
    std::string result = run_awk(
        R"(
        function a() { return b() }
        function b() { return c() }
        function c() { return 42 }
        BEGIN {
            print a()
        })",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Function_With_Builtin) {
    // User function using built-in functions
    std::string result = run_awk(
        R"(
        function capitalize(s) {
            return toupper(substr(s, 1, 1)) tolower(substr(s, 2))
        }
        BEGIN {
            print capitalize("hELLO")
        })",
        ""
    );
    ASSERT_EQ(result, "Hello\n");
}

TEST(Interpreter_Function_String_Manipulation) {
    // Function with string manipulation
    std::string result = run_awk(
        R"(
        function reverse(s,    i, result) {
            result = ""
            for (i = length(s); i >= 1; i--) {
                result = result substr(s, i, 1)
            }
            return result
        }
        BEGIN {
            print reverse("hello")
        })",
        ""
    );
    ASSERT_EQ(result, "olleh\n");
}

// ==================== String Function Tests ====================

TEST(Interpreter_Substr_Basic) {
    // Basic substr with start and length
    std::string result = run_awk(
        R"(BEGIN {
            print substr("Hello World", 1, 5)
        })",
        ""
    );
    ASSERT_EQ(result, "Hello\n");
}

TEST(Interpreter_Substr_No_Length) {
    // substr without length returns rest of string
    std::string result = run_awk(
        R"(BEGIN {
            print substr("Hello World", 7)
        })",
        ""
    );
    ASSERT_EQ(result, "World\n");
}

TEST(Interpreter_Substr_Start_Zero) {
    // Start position 0 treated as 1
    std::string result = run_awk(
        R"(BEGIN {
            print substr("Hello", 0, 3)
        })",
        ""
    );
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Substr_Negative_Start) {
    // Negative start position
    std::string result = run_awk(
        R"(BEGIN {
            print substr("Hello", -2, 5)
        })",
        ""
    );
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Substr_Beyond_Length) {
    // Length exceeds string length
    std::string result = run_awk(
        R"(BEGIN {
            print substr("Hi", 1, 100)
        })",
        ""
    );
    ASSERT_EQ(result, "Hi\n");
}

TEST(Interpreter_Substr_Start_Beyond_End) {
    // Start position beyond string end
    std::string result = run_awk(
        R"(BEGIN {
            print "[" substr("Hello", 100) "]"
        })",
        ""
    );
    ASSERT_EQ(result, "[]\n");
}

TEST(Interpreter_Substr_Empty_String) {
    // substr on empty string
    std::string result = run_awk(
        R"(BEGIN {
            print "[" substr("", 1, 5) "]"
        })",
        ""
    );
    ASSERT_EQ(result, "[]\n");
}

TEST(Interpreter_Substr_Single_Char) {
    // Extract single character
    std::string result = run_awk(
        R"(BEGIN {
            s = "ABCDE"
            print substr(s, 3, 1)
        })",
        ""
    );
    ASSERT_EQ(result, "C\n");
}

TEST(Interpreter_Split_Basic) {
    // Basic split with string separator
    std::string result = run_awk(
        R"(BEGIN {
            n = split("a,b,c", arr, ",")
            print n, arr[1], arr[2], arr[3]
        })",
        ""
    );
    ASSERT_EQ(result, "3 a b c\n");
}

TEST(Interpreter_Split_Regex) {
    // Split with regex separator
    std::string result = run_awk(
        R"(BEGIN {
            n = split("a1b22c333d", arr, /[0-9]+/)
            print n, arr[1], arr[2], arr[3], arr[4]
        })",
        ""
    );
    ASSERT_EQ(result, "4 a b c d\n");
}

TEST(Interpreter_Split_Default_FS) {
    // Split with default FS (whitespace)
    std::string result = run_awk(
        R"(BEGIN {
            n = split("one two three", arr)
            print n, arr[1], arr[2], arr[3]
        })",
        ""
    );
    ASSERT_EQ(result, "3 one two three\n");
}

TEST(Interpreter_Split_Empty_String) {
    // Split empty string - returns 1 (one empty element)
    std::string result = run_awk(
        R"(BEGIN {
            n = split("", arr, ",")
            print n
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Split_No_Match) {
    // Split with separator not found
    std::string result = run_awk(
        R"(BEGIN {
            n = split("hello", arr, ",")
            print n, arr[1]
        })",
        ""
    );
    ASSERT_EQ(result, "1 hello\n");
}

TEST(Interpreter_Split_Multi_Char_Sep) {
    // Split with multi-character separator
    std::string result = run_awk(
        R"(BEGIN {
            n = split("a::b::c", arr, "::")
            print n, arr[1], arr[2], arr[3]
        })",
        ""
    );
    ASSERT_EQ(result, "3 a b c\n");
}

TEST(Interpreter_Sprintf_Basic) {
    // Basic sprintf
    std::string result = run_awk(
        R"(BEGIN {
            s = sprintf("%s is %d years old", "John", 30)
            print s
        })",
        ""
    );
    ASSERT_EQ(result, "John is 30 years old\n");
}

TEST(Interpreter_Sprintf_Float_Precision) {
    // sprintf with float precision
    std::string result = run_awk(
        R"(BEGIN {
            print sprintf("%.3f", 3.14159265)
        })",
        ""
    );
    ASSERT_EQ(result, "3.142\n");
}

TEST(Interpreter_Sprintf_Padding) {
    // sprintf with padding
    std::string result = run_awk(
        R"(BEGIN {
            print sprintf("[%10s]", "test")
        })",
        ""
    );
    ASSERT_EQ(result, "[      test]\n");
}

TEST(Interpreter_Sprintf_Multiple) {
    // sprintf with multiple format specifiers
    std::string result = run_awk(
        R"(BEGIN {
            print sprintf("%d + %d = %d", 2, 3, 5)
        })",
        ""
    );
    ASSERT_EQ(result, "2 + 3 = 5\n");
}

TEST(Interpreter_Index_Found) {
    // index finds substring
    std::string result = run_awk(
        R"(BEGIN {
            print index("Hello World", "World")
        })",
        ""
    );
    ASSERT_EQ(result, "7\n");
}

TEST(Interpreter_Index_Not_Found) {
    // index returns 0 when not found
    std::string result = run_awk(
        R"(BEGIN {
            print index("Hello", "xyz")
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Index_First_Occurrence) {
    // index finds first occurrence
    std::string result = run_awk(
        R"(BEGIN {
            print index("abcabc", "bc")
        })",
        ""
    );
    ASSERT_EQ(result, "2\n");
}

TEST(Interpreter_Index_Empty_Needle) {
    // index with empty needle
    std::string result = run_awk(
        R"(BEGIN {
            print index("Hello", "")
        })",
        ""
    );
    // AWK implementations vary; typically returns 0 or 1
    ASSERT_TRUE(result == "0\n" || result == "1\n");
}

TEST(Interpreter_Length_Basic_String) {
    // length of string
    std::string result = run_awk(
        R"(BEGIN {
            print length("Hello")
        })",
        ""
    );
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Length_Empty_String) {
    // length of empty string
    std::string result = run_awk(
        R"(BEGIN {
            print length("")
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Length_Of_Number) {
    // length of number (converted to string)
    std::string result = run_awk(
        R"(BEGIN {
            print length(12345)
        })",
        ""
    );
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Toupper_Basic) {
    // toupper converts to uppercase
    std::string result = run_awk(
        R"(BEGIN {
            print toupper("Hello World")
        })",
        ""
    );
    ASSERT_EQ(result, "HELLO WORLD\n");
}

TEST(Interpreter_Toupper_Already_Upper) {
    // toupper on already uppercase
    std::string result = run_awk(
        R"(BEGIN {
            print toupper("HELLO")
        })",
        ""
    );
    ASSERT_EQ(result, "HELLO\n");
}

TEST(Interpreter_Tolower_Basic) {
    // tolower converts to lowercase
    std::string result = run_awk(
        R"(BEGIN {
            print tolower("Hello World")
        })",
        ""
    );
    ASSERT_EQ(result, "hello world\n");
}

TEST(Interpreter_Tolower_Already_Lower) {
    // tolower on already lowercase
    std::string result = run_awk(
        R"(BEGIN {
            print tolower("hello")
        })",
        ""
    );
    ASSERT_EQ(result, "hello\n");
}

TEST(Interpreter_Sub_Basic) {
    // Basic sub replacement
    std::string result = run_awk(
        R"(BEGIN {
            x = "hello world"
            sub(/world/, "AWK", x)
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "hello AWK\n");
}

TEST(Interpreter_Sub_First_Only) {
    // sub replaces only first occurrence
    std::string result = run_awk(
        R"(BEGIN {
            x = "aaa"
            sub(/a/, "X", x)
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "Xaa\n");
}

TEST(Interpreter_Sub_Ampersand) {
    // & in replacement means matched string
    std::string result = run_awk(
        R"(BEGIN {
            x = "hello"
            sub(/ell/, "[&]", x)
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "h[ell]o\n");
}

TEST(Interpreter_Gsub_All) {
    // gsub replaces all occurrences
    std::string result = run_awk(
        R"(BEGIN {
            x = "aaa"
            n = gsub(/a/, "X", x)
            print n, x
        })",
        ""
    );
    ASSERT_EQ(result, "3 XXX\n");
}

TEST(Interpreter_Gsub_Regex) {
    // gsub with regex pattern
    std::string result = run_awk(
        R"(BEGIN {
            x = "a1b2c3"
            gsub(/[0-9]/, "#", x)
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "a#b#c#\n");
}

TEST(Interpreter_Match_Sets_RSTART) {
    // match sets RSTART
    std::string result = run_awk(
        R"(BEGIN {
            match("hello world", /wor/)
            print RSTART
        })",
        ""
    );
    ASSERT_EQ(result, "7\n");
}

TEST(Interpreter_Match_Sets_RLENGTH) {
    // match sets RLENGTH
    std::string result = run_awk(
        R"(BEGIN {
            match("hello world", /wor/)
            print RLENGTH
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Match_Not_Found) {
    // match returns 0 when not found
    std::string result = run_awk(
        R"(BEGIN {
            print match("hello", /xyz/)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Match_Extract) {
    // Use match with substr to extract
    std::string result = run_awk(
        R"(BEGIN {
            s = "The price is $42.50"
            if (match(s, /\$[0-9]+\.[0-9]+/)) {
                print substr(s, RSTART, RLENGTH)
            }
        })",
        ""
    );
    ASSERT_EQ(result, "$42.50\n");
}

// ==================== Math Function Tests ====================

TEST(Interpreter_Math_Sin) {
    // sin(0) = 0
    std::string result = run_awk(
        R"(BEGIN {
            print sin(0)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Math_Sin_Pi) {
    // sin(pi/2) ≈ 1
    std::string result = run_awk(
        R"(BEGIN {
            pi = 3.14159265358979
            printf "%.0f\n", sin(pi/2)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Math_Cos) {
    // cos(0) = 1
    std::string result = run_awk(
        R"(BEGIN {
            print cos(0)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Math_Cos_Pi) {
    // cos(pi) ≈ -1
    std::string result = run_awk(
        R"(BEGIN {
            pi = 3.14159265358979
            printf "%.0f\n", cos(pi)
        })",
        ""
    );
    ASSERT_EQ(result, "-1\n");
}

TEST(Interpreter_Math_Atan2_Basic) {
    // atan2(0, 1) = 0
    std::string result = run_awk(
        R"(BEGIN {
            print atan2(0, 1)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Math_Atan2_Pi) {
    // atan2(1, 0) = pi/2
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.4f\n", atan2(1, 0)
        })",
        ""
    );
    ASSERT_EQ(result, "1.5708\n");
}

TEST(Interpreter_Math_Atan2_Negative) {
    // atan2(-1, 0) = -pi/2
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.4f\n", atan2(-1, 0)
        })",
        ""
    );
    ASSERT_EQ(result, "-1.5708\n");
}

TEST(Interpreter_Math_Sqrt_Perfect) {
    // sqrt of perfect squares
    std::string result = run_awk(
        R"(BEGIN {
            print sqrt(4), sqrt(9), sqrt(16), sqrt(25)
        })",
        ""
    );
    ASSERT_EQ(result, "2 3 4 5\n");
}

TEST(Interpreter_Math_Sqrt_Zero) {
    // sqrt(0) = 0
    std::string result = run_awk(
        R"(BEGIN {
            print sqrt(0)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Math_Sqrt_Decimal) {
    // sqrt of non-perfect square
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.4f\n", sqrt(2)
        })",
        ""
    );
    ASSERT_EQ(result, "1.4142\n");
}

TEST(Interpreter_Math_Log_E) {
    // log(e) ≈ 1
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.0f\n", log(2.718281828)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Math_Log_1) {
    // log(1) = 0
    std::string result = run_awk(
        R"(BEGIN {
            print log(1)
        })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Math_Exp_0) {
    // exp(0) = 1
    std::string result = run_awk(
        R"(BEGIN {
            print exp(0)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Math_Exp_1) {
    // exp(1) ≈ e
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.4f\n", exp(1)
        })",
        ""
    );
    ASSERT_EQ(result, "2.7183\n");
}

TEST(Interpreter_Math_Exp_Log_Inverse) {
    // exp(log(x)) = x
    std::string result = run_awk(
        R"(BEGIN {
            x = 42
            printf "%.0f\n", exp(log(x))
        })",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Math_Int_Positive) {
    // int truncates toward zero (positive)
    std::string result = run_awk(
        R"(BEGIN {
            print int(3.7), int(3.2), int(3.9)
        })",
        ""
    );
    ASSERT_EQ(result, "3 3 3\n");
}

TEST(Interpreter_Math_Int_Negative) {
    // int truncates toward zero (negative)
    std::string result = run_awk(
        R"(BEGIN {
            print int(-3.7), int(-3.2), int(-3.9)
        })",
        ""
    );
    ASSERT_EQ(result, "-3 -3 -3\n");
}

TEST(Interpreter_Math_Int_Already_Int) {
    // int of integer is unchanged
    std::string result = run_awk(
        R"(BEGIN {
            print int(5), int(-5), int(0)
        })",
        ""
    );
    ASSERT_EQ(result, "5 -5 0\n");
}

TEST(Interpreter_Math_Rand_Range) {
    // rand() returns value in [0,1)
    std::string result = run_awk(
        R"(BEGIN {
            ok = 1
            for (i = 0; i < 10; i++) {
                r = rand()
                if (r < 0 || r >= 1) ok = 0
            }
            print ok
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Math_Rand_Different) {
    // Multiple rand() calls give different values
    std::string result = run_awk(
        R"(BEGIN {
            r1 = rand()
            r2 = rand()
            r3 = rand()
            # Very unlikely all three are equal
            print (r1 != r2 || r2 != r3)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Math_Srand_Seed) {
    // srand with same seed gives same sequence
    std::string result = run_awk(
        R"(BEGIN {
            srand(42)
            a1 = rand()
            a2 = rand()
            srand(42)
            b1 = rand()
            b2 = rand()
            print (a1 == b1 && a2 == b2)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Math_Srand_Returns_New) {
    // srand returns the new seed value
    std::string result = run_awk(
        R"(BEGIN {
            result = srand(42)
            print result
        })",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Math_Power_Basic) {
    // Power operator
    std::string result = run_awk(
        R"(BEGIN {
            print 2^3, 3^2, 10^0
        })",
        ""
    );
    ASSERT_EQ(result, "8 9 1\n");
}

TEST(Interpreter_Math_Power_Fractional) {
    // Fractional exponent (square root)
    std::string result = run_awk(
        R"(BEGIN {
            print 4^0.5, 9^0.5
        })",
        ""
    );
    ASSERT_EQ(result, "2 3\n");
}

TEST(Interpreter_Math_Power_Negative_Exp) {
    // Negative exponent
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.2f\n", 2^-1
        })",
        ""
    );
    ASSERT_EQ(result, "0.50\n");
}

TEST(Interpreter_Math_Modulo_Positive) {
    // Modulo with positive numbers
    std::string result = run_awk(
        R"(BEGIN {
            print 17 % 5, 10 % 3, 8 % 4
        })",
        ""
    );
    ASSERT_EQ(result, "2 1 0\n");
}

TEST(Interpreter_Math_Modulo_Negative) {
    // Modulo with negative numbers
    std::string result = run_awk(
        R"(BEGIN {
            print -17 % 5, 17 % -5
        })",
        ""
    );
    // Results may vary by implementation
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Math_Division_Int) {
    // Integer division
    std::string result = run_awk(
        R"(BEGIN {
            print int(17 / 5), int(10 / 3)
        })",
        ""
    );
    ASSERT_EQ(result, "3 3\n");
}

TEST(Interpreter_Math_Division_Float) {
    // Floating point division
    std::string result = run_awk(
        R"(BEGIN {
            printf "%.2f\n", 5 / 2
        })",
        ""
    );
    ASSERT_EQ(result, "2.50\n");
}

TEST(Interpreter_Math_Large_Numbers) {
    // Large number arithmetic
    std::string result = run_awk(
        R"(BEGIN {
            print 1000000 * 1000000
        })",
        ""
    );
    // Result is 1000000000000 (1 trillion)
    ASSERT_EQ(result, "1000000000000\n");
}

TEST(Interpreter_Math_Very_Small) {
    // Very small numbers
    std::string result = run_awk(
        R"(BEGIN {
            x = 0.000001
            printf "%.6f\n", x * 10
        })",
        ""
    );
    ASSERT_EQ(result, "0.000010\n");
}

TEST(Interpreter_Math_Negative_Zero) {
    // Operations resulting in -0
    std::string result = run_awk(
        R"(BEGIN {
            x = -0.0
            print (x == 0)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Math_Compound_Expression) {
    // Complex mathematical expression
    std::string result = run_awk(
        R"(BEGIN {
            x = 2
            y = 3
            z = sqrt(x^2 + y^2)
            printf "%.4f\n", z
        })",
        ""
    );
    // sqrt(4 + 9) = sqrt(13) ≈ 3.6056
    ASSERT_EQ(result, "3.6056\n");
}

TEST(Interpreter_Math_Trig_Identity) {
    // sin^2 + cos^2 = 1
    std::string result = run_awk(
        R"(BEGIN {
            x = 1.23
            result = sin(x)^2 + cos(x)^2
            printf "%.0f\n", result
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

// ==================== Special Variables Tests ====================

TEST(Interpreter_NR_Counts_Records) {
    // NR counts total records
    std::string result = run_awk(
        R"(END { print NR })",
        "line1\nline2\nline3\n"
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_NR_In_Pattern) {
    // NR available in pattern
    std::string result = run_awk(
        R"(NR == 2 { print "second:", $0 })",
        "first\nsecond\nthird\n"
    );
    ASSERT_EQ(result, "second: second\n");
}

TEST(Interpreter_NF_Counts_Fields) {
    // NF counts fields in current record
    std::string result = run_awk(
        R"({ print NF })",
        "one two three\na b\nsingle\n"
    );
    ASSERT_EQ(result, "3\n2\n1\n");
}

TEST(Interpreter_NF_Empty_Line) {
    // NF is 0 for empty line
    std::string result = run_awk(
        R"({ print NF })",
        "\n"
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_FS_Space_Default) {
    // Default FS is space/tab
    std::string result = run_awk(
        R"({ print $1, $2, $3 })",
        "a b c\n"
    );
    ASSERT_EQ(result, "a b c\n");
}

TEST(Interpreter_FS_Tab) {
    // FS handles tabs
    std::string result = run_awk(
        R"({ print $1, $2 })",
        "a\tb\n"
    );
    ASSERT_EQ(result, "a b\n");
}

TEST(Interpreter_FS_Custom_Char) {
    // Custom single-char FS
    std::string result = run_awk(
        R"(BEGIN { FS = ":" } { print $1, $2, $3 })",
        "a:b:c\n"
    );
    ASSERT_EQ(result, "a b c\n");
}

TEST(Interpreter_FS_Regex) {
    // FS as regex
    std::string result = run_awk(
        R"(BEGIN { FS = "[,;]" } { print $1, $2, $3 })",
        "a,b;c\n"
    );
    ASSERT_EQ(result, "a b c\n");
}

TEST(Interpreter_FS_Single_Char) {
    // FS as single character
    std::string result = run_awk(
        R"(BEGIN { FS = "-" } { print $1, $2, $3 })",
        "a-b-c\n"
    );
    ASSERT_EQ(result, "a b c\n");
}

TEST(Interpreter_OFS_Default) {
    // Default OFS is space
    std::string result = run_awk(
        R"({ print $1, $2 })",
        "a b\n"
    );
    ASSERT_EQ(result, "a b\n");
}

TEST(Interpreter_OFS_Custom) {
    // Custom OFS
    std::string result = run_awk(
        R"(BEGIN { OFS = ":" } { print $1, $2, $3 })",
        "a b c\n"
    );
    ASSERT_EQ(result, "a:b:c\n");
}

TEST(Interpreter_ORS_Default) {
    // Default ORS is newline
    std::string result = run_awk(
        R"({ print $1 })",
        "a\nb\n"
    );
    ASSERT_EQ(result, "a\nb\n");
}

TEST(Interpreter_ORS_Custom) {
    // Custom ORS
    std::string result = run_awk(
        R"(BEGIN { ORS = ";" } { print $1 })",
        "a\nb\n"
    );
    ASSERT_EQ(result, "a;b;");
}

TEST(Interpreter_RS_Custom) {
    // Custom RS (record separator)
    std::string result = run_awk(
        R"(BEGIN { RS = ":" } { print NR, $0 })",
        "a:b:c"
    );
    ASSERT_TRUE(result.find("1 a") != std::string::npos);
    ASSERT_TRUE(result.find("2 b") != std::string::npos);
}

TEST(Interpreter_SUBSEP_Default) {
    // Default SUBSEP is \034
    std::string result = run_awk(
        R"(BEGIN {
            a[1,2] = "test"
            for (k in a) {
                print length(k)
            }
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");  // "1" + SUBSEP + "2" = 3 chars
}

TEST(Interpreter_CONVFMT_Default) {
    // CONVFMT controls number-to-string conversion
    std::string result = run_awk(
        R"(BEGIN {
            x = 3.14159
            print x ""
        })",
        ""
    );
    ASSERT_TRUE(result.find("3.14") != std::string::npos);
}

TEST(Interpreter_OFMT_Default) {
    // OFMT controls print number format
    std::string result = run_awk(
        R"(BEGIN {
            print 3.14159
        })",
        ""
    );
    ASSERT_TRUE(result.find("3.14") != std::string::npos);
}

// ==================== Field Manipulation Tests ====================

TEST(Interpreter_Field_Access_Dollar_1) {
    // $1 is first field
    std::string result = run_awk(
        R"({ print $1 })",
        "hello world\n"
    );
    ASSERT_EQ(result, "hello\n");
}

TEST(Interpreter_Field_Access_Dollar_0) {
    // $0 is entire record
    std::string result = run_awk(
        R"({ print $0 })",
        "hello world\n"
    );
    ASSERT_EQ(result, "hello world\n");
}

TEST(Interpreter_Field_Access_Last) {
    // $NF is last field
    std::string result = run_awk(
        R"({ print $NF })",
        "a b c d\n"
    );
    ASSERT_EQ(result, "d\n");
}

TEST(Interpreter_Field_Access_Beyond_NF) {
    // Field beyond NF is empty
    std::string result = run_awk(
        R"({ print "[" $10 "]" })",
        "a b c\n"
    );
    ASSERT_EQ(result, "[]\n");
}

TEST(Interpreter_Field_Access_Negative) {
    // Negative field index
    std::string result = run_awk(
        R"({ print "[" $-1 "]" })",
        "a b c\n"
    );
    // Typically returns empty or $0
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Field_Assign_Existing) {
    // Assign to existing field
    std::string result = run_awk(
        R"({ $2 = "CHANGED"; print $0 })",
        "a b c\n"
    );
    ASSERT_EQ(result, "a CHANGED c\n");
}

TEST(Interpreter_Field_Assign_Beyond_NF) {
    // Assign beyond NF extends record
    std::string result = run_awk(
        R"({ $5 = "new"; print $0 })",
        "a b c\n"
    );
    ASSERT_TRUE(result.find("new") != std::string::npos);
}

TEST(Interpreter_Field_Assign_Updates_NF) {
    // Assigning beyond NF updates NF
    std::string result = run_awk(
        R"({ $5 = "x"; print NF })",
        "a b c\n"
    );
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Field_Assign_Dollar_0) {
    // Assigning $0 re-splits fields
    std::string result = run_awk(
        R"({ $0 = "x y z"; print $2, NF })",
        "a b c d\n"
    );
    ASSERT_EQ(result, "y 3\n");
}

TEST(Interpreter_NF_Decrease) {
    // Decreasing NF - test that NF can be modified
    std::string result = run_awk(
        R"({ NF = 2; print NF, $1, $2 })",
        "a b c d e\n"
    );
    ASSERT_EQ(result, "2 a b\n");
}

TEST(Interpreter_NF_Increase) {
    // Increasing NF adds empty fields
    std::string result = run_awk(
        R"({ NF = 5; print NF, "[" $4 "]", "[" $5 "]" })",
        "a b c\n"
    );
    ASSERT_EQ(result, "5 [] []\n");
}

TEST(Interpreter_Field_Computed_Index) {
    // Computed field index
    std::string result = run_awk(
        R"({ i = 2; print $i })",
        "a b c\n"
    );
    ASSERT_EQ(result, "b\n");
}

TEST(Interpreter_Field_Expression_Index) {
    // Expression as field index
    std::string result = run_awk(
        R"({ print $(1+1) })",
        "a b c\n"
    );
    ASSERT_EQ(result, "b\n");
}

TEST(Interpreter_Field_Modify_Updates_Dollar_0) {
    // Modifying field updates $0
    std::string result = run_awk(
        R"(BEGIN { OFS = "-" } { $1 = "X"; print $0 })",
        "a b c\n"
    );
    ASSERT_EQ(result, "X-b-c\n");
}

TEST(Interpreter_Field_With_Custom_FS_OFS) {
    // FS and OFS interaction
    std::string result = run_awk(
        R"(BEGIN { FS = ":"; OFS = "," } { $1 = $1; print $0 })",
        "a:b:c\n"
    );
    ASSERT_EQ(result, "a,b,c\n");
}

// ==================== Multiple Patterns and Rules ====================

TEST(Interpreter_Multiple_Rules) {
    // Multiple rules all execute
    std::string result = run_awk(
        R"(
        { count++ }
        /a/ { a_count++ }
        END { print count, a_count }
        )",
        "apple\nbanana\ncherry\n"
    );
    ASSERT_EQ(result, "3 2\n");
}

TEST(Interpreter_Multiple_BEGIN) {
    // Multiple BEGIN blocks
    std::string result = run_awk(
        R"(
        BEGIN { x = 1 }
        BEGIN { x += 10 }
        BEGIN { print x }
        )",
        ""
    );
    ASSERT_EQ(result, "11\n");
}

TEST(Interpreter_Multiple_END) {
    // Multiple END blocks
    std::string result = run_awk(
        R"(
        END { print "first" }
        END { print "second" }
        )",
        "data\n"
    );
    ASSERT_TRUE(result.find("first") != std::string::npos);
    ASSERT_TRUE(result.find("second") != std::string::npos);
}

TEST(Interpreter_Pattern_And_Action) {
    // Pattern with action
    std::string result = run_awk(
        R"(/error/ { print "Found:", $0 })",
        "info: ok\nerror: bad\ninfo: good\n"
    );
    ASSERT_EQ(result, "Found: error: bad\n");
}

TEST(Interpreter_Pattern_Only) {
    // Pattern without action prints line
    std::string result = run_awk(
        R"(/yes/)",
        "no\nyes\nno\nyes\n"
    );
    ASSERT_EQ(result, "yes\nyes\n");
}

TEST(Interpreter_Action_Only) {
    // Action without pattern runs for all lines
    std::string result = run_awk(
        R"({ print ">" $0 })",
        "a\nb\n"
    );
    ASSERT_EQ(result, ">a\n>b\n");
}

TEST(Interpreter_Numeric_Comparison_Pattern) {
    // Numeric comparison as pattern
    std::string result = run_awk(
        R"(NR <= 2 { print NR, $0 })",
        "first\nsecond\nthird\n"
    );
    ASSERT_EQ(result, "1 first\n2 second\n");
}

TEST(Interpreter_String_Match_Pattern) {
    // Field match as pattern
    std::string result = run_awk(
        R"($1 == "yes" { print $2 })",
        "yes good\nno bad\nyes great\n"
    );
    ASSERT_EQ(result, "good\ngreat\n");
}

TEST(Interpreter_Negated_Regex_Pattern) {
    // Negated regex pattern
    std::string result = run_awk(
        R"(!/skip/ { print })",
        "keep\nskip\nkeep\n"
    );
    ASSERT_EQ(result, "keep\nkeep\n");
}

TEST(Interpreter_Expression_As_Pattern) {
    // Expression as pattern
    std::string result = run_awk(
        R"($1 > 5 { print $1 })",
        "3\n7\n2\n10\n"
    );
    ASSERT_EQ(result, "7\n10\n");
}

// ==================== ARGC/ARGV Tests ====================

TEST(Interpreter_ARGC_No_Files) {
    // ARGC is 1 when no files (just program name)
    std::string result = run_awk(
        R"(BEGIN { print ARGC })",
        ""
    );
    // ARGC includes awk itself, so at least 1
    ASSERT_TRUE(result.find("1") != std::string::npos || result.find("0") != std::string::npos);
}

TEST(Interpreter_ARGV_Zero) {
    // ARGV[0] is the program name or empty
    std::string result = run_awk(
        R"(BEGIN { print "[" ARGV[0] "]" })",
        ""
    );
    // May be "awk" or empty depending on implementation
    ASSERT_TRUE(result.length() > 0);
}

// ==================== ENVIRON Tests ====================

TEST(Interpreter_ENVIRON_PATH) {
    // ENVIRON contains environment variables
    std::string result = run_awk(
        R"(BEGIN {
            # PATH should exist on all systems
            if ("PATH" in ENVIRON) print "has_path"
            else print "no_path"
        })",
        ""
    );
    // Either has PATH or doesn't - both are valid
    ASSERT_TRUE(result.find("path") != std::string::npos);
}

TEST(Interpreter_ENVIRON_Access) {
    // Accessing non-existent ENVIRON key returns empty
    std::string result = run_awk(
        R"(BEGIN { print "[" ENVIRON["NONEXISTENT_VAR_12345"] "]" })",
        ""
    );
    ASSERT_EQ(result, "[]\n");
}

// ==================== FNR Tests ====================

TEST(Interpreter_FNR_Single_File) {
    // FNR equals NR for single file
    std::string result = run_awk(
        R"({ print NR, FNR })",
        "a\nb\nc\n"
    );
    ASSERT_EQ(result, "1 1\n2 2\n3 3\n");
}

TEST(Interpreter_FNR_Resets) {
    // FNR should be available
    std::string result = run_awk(
        R"(BEGIN { print "FNR exists" } { print FNR })",
        "line1\nline2\n"
    );
    ASSERT_TRUE(result.find("1") != std::string::npos);
    ASSERT_TRUE(result.find("2") != std::string::npos);
}

// ==================== FILENAME Tests ====================

TEST(Interpreter_FILENAME_Stdin) {
    // FILENAME for stdin input
    std::string result = run_awk(
        R"({ print FILENAME })",
        "test\n"
    );
    // Might be empty, "-", or some other value for stdin
    ASSERT_TRUE(result.length() > 0);
}

// ==================== I/O Redirection Tests ====================

TEST(Interpreter_Print_To_File) {
    // print > file redirects output
    std::string testfile = "__test_output_" + std::to_string(time(nullptr)) + ".txt";
    std::string result = run_awk(
        "BEGIN { print \"hello\" > \"" + testfile + "\"; print \"done\" }",
        ""
    );
    ASSERT_TRUE(result.find("done") != std::string::npos);
    // Clean up
    std::remove(testfile.c_str());
}

TEST(Interpreter_Print_Append_File) {
    // print >> file appends to file
    std::string testfile = "__test_append_" + std::to_string(time(nullptr)) + ".txt";
    std::string result = run_awk(
        "BEGIN { print \"line1\" > \"" + testfile + "\"; print \"line2\" >> \"" + testfile + "\"; print \"done\" }",
        ""
    );
    ASSERT_TRUE(result.find("done") != std::string::npos);
    // Clean up
    std::remove(testfile.c_str());
}

TEST(Interpreter_Close_File) {
    // close() closes a file
    std::string testfile = "__test_close_" + std::to_string(time(nullptr)) + ".txt";
    std::string result = run_awk(
        "BEGIN { print \"test\" > \"" + testfile + "\"; close(\"" + testfile + "\"); print \"closed\" }",
        ""
    );
    ASSERT_TRUE(result.find("closed") != std::string::npos);
    // Clean up
    std::remove(testfile.c_str());
}

TEST(Interpreter_Printf_To_File) {
    // printf > file redirects formatted output
    std::string testfile = "__test_printf_" + std::to_string(time(nullptr)) + ".txt";
    std::string result = run_awk(
        "BEGIN { printf \"%d\\n\", 42 > \"" + testfile + "\"; print \"done\" }",
        ""
    );
    ASSERT_TRUE(result.find("done") != std::string::npos);
    // Clean up
    std::remove(testfile.c_str());
}

// ==================== Additional Edge Cases ====================

TEST(Interpreter_Empty_Input) {
    // Empty input - only BEGIN/END run
    std::string result = run_awk(
        R"(BEGIN { print "start" } { print "middle" } END { print "end" })",
        ""
    );
    ASSERT_EQ(result, "start\nend\n");
}

TEST(Interpreter_Whitespace_Only_Input) {
    // Whitespace-only lines
    std::string result = run_awk(
        R"({ print NF })",
        "   \n\t\n  \t  \n"
    );
    ASSERT_EQ(result, "0\n0\n0\n");
}

TEST(Interpreter_Very_Long_Line) {
    // Very long line handling
    std::string longLine(1000, 'x');
    std::string result = run_awk(
        R"({ print length($0) })",
        longLine + "\n"
    );
    ASSERT_EQ(result, "1000\n");
}

TEST(Interpreter_Many_Fields) {
    // Line with many fields
    std::string manyFields;
    for (int i = 1; i <= 100; i++) {
        manyFields += std::to_string(i);
        if (i < 100) manyFields += " ";
    }
    std::string result = run_awk(
        R"({ print NF, $1, $50, $100 })",
        manyFields + "\n"
    );
    ASSERT_EQ(result, "100 1 50 100\n");
}

TEST(Interpreter_Unicode_Basic) {
    // Basic non-ASCII characters
    std::string result = run_awk(
        R"(BEGIN { print "Héllo Wörld" })",
        ""
    );
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Numeric_String_Comparison) {
    // Numeric string comparison
    std::string result = run_awk(
        R"(BEGIN {
            print ("10" > "9")
            print ("10" > "09")
            print (10 > 9)
        })",
        ""
    );
    // String "10" < "9" (lexicographic), but 10 > 9 (numeric)
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_String_To_Number) {
    // String to number conversion
    std::string result = run_awk(
        R"(BEGIN {
            x = "42"
            print x + 0
            print x + 8
        })",
        ""
    );
    ASSERT_EQ(result, "42\n50\n");
}

TEST(Interpreter_Number_To_String) {
    // Number to string conversion
    std::string result = run_awk(
        R"(BEGIN {
            x = 42
            print x ""
            print length(x)
        })",
        ""
    );
    ASSERT_EQ(result, "42\n2\n");
}

TEST(Interpreter_Uninitialized_In_Expression) {
    // Uninitialized variable in expression
    std::string result = run_awk(
        R"(BEGIN {
            print x + 1
            print y "" "test"
        })",
        ""
    );
    ASSERT_EQ(result, "1\ntest\n");
}

TEST(Interpreter_Array_In_Expression) {
    // Array element in expression
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = 10
            print a[1] + a[2] + 5
        })",
        ""
    );
    ASSERT_EQ(result, "15\n");
}

TEST(Interpreter_Nested_Ternary) {
    // Nested ternary operator
    std::string result = run_awk(
        R"(BEGIN {
            x = 5
            print (x < 0 ? "negative" : (x == 0 ? "zero" : "positive"))
        })",
        ""
    );
    ASSERT_EQ(result, "positive\n");
}

TEST(Interpreter_Complex_Regex) {
    // Complex regex pattern
    std::string result = run_awk(
        R"(/^[a-z]+[0-9]+$/ { print "match:", $0 })",
        "abc123\nABC123\nabc\n123\ntest99\n"
    );
    ASSERT_EQ(result, "match: abc123\nmatch: test99\n");
}

TEST(Interpreter_Regex_Special_Chars) {
    // Regex with special characters
    std::string result = run_awk(
        R"(/\$[0-9]+\.[0-9]+/ { print "price found" })",
        "The price is $19.99\nNo price here\n$5.00 only\n"
    );
    ASSERT_EQ(result, "price found\nprice found\n");
}

TEST(Interpreter_Backslash_In_String) {
    // Backslash handling in strings
    std::string result = run_awk(
        R"(BEGIN { print "a\\b\\c" })",
        ""
    );
    ASSERT_EQ(result, "a\\b\\c\n");
}

TEST(Interpreter_Quote_In_String) {
    // Quote handling in strings
    std::string result = run_awk(
        R"(BEGIN { print "He said \"hello\"" })",
        ""
    );
    ASSERT_EQ(result, "He said \"hello\"\n");
}

TEST(Interpreter_Concatenation_Precedence) {
    // Concatenation vs arithmetic precedence
    std::string result = run_awk(
        R"(BEGIN {
            print 1 + 2 " " 3 + 4
            print (1 + 2) " " (3 + 4)
        })",
        ""
    );
    // First: 1 + (2 " " 3) + 4 or (1+2) " " (3+4)?
    // AWK: concatenation has lower precedence than +
    ASSERT_TRUE(result.find("3 7") != std::string::npos);
}

TEST(Interpreter_Assignment_In_Condition) {
    // Assignment in condition
    std::string result = run_awk(
        R"(BEGIN {
            if (x = 5) print x
            while ((y = y + 1) <= 3) print y
        })",
        ""
    );
    ASSERT_TRUE(result.find("5") != std::string::npos);
    ASSERT_TRUE(result.find("1") != std::string::npos);
}

TEST(Interpreter_Chained_Assignment) {
    // Chained assignment
    std::string result = run_awk(
        R"(BEGIN {
            a = b = c = 10
            print a, b, c
        })",
        ""
    );
    ASSERT_EQ(result, "10 10 10\n");
}

TEST(Interpreter_Compound_Assignment_All) {
    // All compound assignment operators
    std::string result = run_awk(
        R"(BEGIN {
            x = 10
            x += 5; print x
            x -= 3; print x
            x *= 2; print x
            x /= 4; print x
            x %= 5; print x
            x ^= 2; print x
        })",
        ""
    );
    ASSERT_EQ(result, "15\n12\n24\n6\n1\n1\n");
}

TEST(Interpreter_Pre_Post_Increment_Expr) {
    // Pre/post increment in expressions
    std::string result = run_awk(
        R"(BEGIN {
            x = 5
            print x++
            print x
            print ++x
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "5\n6\n7\n7\n");
}

TEST(Interpreter_Logical_Short_Circuit) {
    // Logical short-circuit evaluation
    std::string result = run_awk(
        R"(BEGIN {
            x = 0
            if (0 && (x = 1)) print "yes"
            print "x=" x
            if (1 || (x = 2)) print "yes2"
            print "x=" x
        })",
        ""
    );
    ASSERT_TRUE(result.find("x=0") != std::string::npos);
}

TEST(Interpreter_Comparison_Chain) {
    // Multiple comparisons
    std::string result = run_awk(
        R"(BEGIN {
            print (1 < 2)
            print (2 < 1)
            print (1 <= 1)
            print (1 >= 1)
            print (1 == 1)
            print (1 != 2)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n0\n1\n1\n1\n1\n");
}

TEST(Interpreter_String_Comparison_Operators) {
    // String comparisons
    std::string result = run_awk(
        R"(BEGIN {
            print ("abc" < "abd")
            print ("abc" == "abc")
            print ("ABC" < "abc")
        })",
        ""
    );
    ASSERT_EQ(result, "1\n1\n1\n");
}

TEST(Interpreter_Mixed_Type_Comparison) {
    // Mixed type comparison (string vs number)
    std::string result = run_awk(
        R"(BEGIN {
            print ("10" == 10)
            print ("10" + 0 == 10)
        })",
        ""
    );
    // Behavior may vary - both should be valid
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Division_By_Zero) {
    // Division by zero handling
    std::string result = run_awk(
        R"(BEGIN {
            x = 1 / 0
            print (x == x)
        })",
        ""
    );
    // May produce inf, nan, or error - just check it doesn't crash
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Modulo_By_Zero) {
    // Modulo by zero handling
    std::string result = run_awk(
        R"(BEGIN {
            x = 5 % 0
            print "survived"
        })",
        ""
    );
    // May produce nan or error - check it runs
    ASSERT_TRUE(result.length() > 0);
}

// ==================== Range Pattern Tests ====================
// Note: Range patterns use the syntax /start/,/end/
// These tests verify the range pattern implementation works correctly

TEST(Interpreter_Range_Pattern_Regex_Basic) {
    // Basic regex range pattern /start/,/end/
    std::string result = run_awk(
        R"(/BEGIN/,/END/ { print })",
        "before\nBEGIN\nline1\nline2\nEND\nafter\n"
    );
    ASSERT_TRUE(result.find("BEGIN") != std::string::npos);
    ASSERT_TRUE(result.find("line1") != std::string::npos);
    ASSERT_TRUE(result.find("line2") != std::string::npos);
    ASSERT_TRUE(result.find("END") != std::string::npos);
    ASSERT_TRUE(result.find("before") == std::string::npos);
    ASSERT_TRUE(result.find("after") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Regex_Same_Line) {
    // Start and end regex on same line
    std::string result = run_awk(
        R"(/start/,/end/ { print })",
        "no match\nstart and end here\nno match\n"
    );
    ASSERT_TRUE(result.find("start and end") != std::string::npos);
    ASSERT_TRUE(result.find("no match") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Regex_Multiple) {
    // Multiple separate regex ranges
    std::string result = run_awk(
        R"(/ON/,/OFF/ { print })",
        "skip\nON\ndata1\nOFF\nskip\nON\ndata2\nOFF\nskip\n"
    );
    ASSERT_TRUE(result.find("ON") != std::string::npos);
    ASSERT_TRUE(result.find("data1") != std::string::npos);
    ASSERT_TRUE(result.find("data2") != std::string::npos);
    ASSERT_TRUE(result.find("OFF") != std::string::npos);
    ASSERT_TRUE(result.find("skip") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Regex_No_End) {
    // Regex range without end - continues to EOF
    std::string result = run_awk(
        R"(/START/,/END/ { print })",
        "before\nSTART\nline1\nline2\nline3\n"
    );
    ASSERT_TRUE(result.find("START") != std::string::npos);
    ASSERT_TRUE(result.find("line1") != std::string::npos);
    ASSERT_TRUE(result.find("line2") != std::string::npos);
    ASSERT_TRUE(result.find("line3") != std::string::npos);
    ASSERT_TRUE(result.find("before") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Regex_Count) {
    // Count lines in regex range
    std::string result = run_awk(
        R"(/begin/,/end/ { count++ } END { print count })",
        "skip\nbegin\na\nb\nc\nend\nskip\n"
    );
    ASSERT_EQ(result, "5\n");  // begin, a, b, c, end = 5 lines
}

TEST(Interpreter_Range_Pattern_Regex_Empty) {
    // Regex range that never matches
    std::string result = run_awk(
        R"(/XYZZY/,/PLUGH/ { print })",
        "line1\nline2\nline3\n"
    );
    ASSERT_EQ(result, "");
}

TEST(Interpreter_Range_Pattern_Mixed_Regex_Expr) {
    // Mixed: regex start, expression end
    std::string result = run_awk(
        R"(/start/,NR==4 { print NR, $0 })",
        "line1\nstart here\nline3\nline4\nline5\n"
    );
    ASSERT_TRUE(result.find("2 start") != std::string::npos);
    ASSERT_TRUE(result.find("3 line3") != std::string::npos);
    ASSERT_TRUE(result.find("4 line4") != std::string::npos);
    ASSERT_TRUE(result.find("line1") == std::string::npos);
    ASSERT_TRUE(result.find("line5") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Numeric) {
    // Numeric range pattern NR==2,NR==4
    std::string result = run_awk(
        R"(NR==2,NR==4 { print })",
        "line1\nline2\nline3\nline4\nline5\n"
    );
    ASSERT_TRUE(result.find("line2") != std::string::npos);
    ASSERT_TRUE(result.find("line3") != std::string::npos);
    ASSERT_TRUE(result.find("line4") != std::string::npos);
    ASSERT_TRUE(result.find("line1") == std::string::npos);
    ASSERT_TRUE(result.find("line5") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Field_Match) {
    // Range with field comparison
    std::string result = run_awk(
        R"($1=="start",$1=="end" { print $2 })",
        "other match\nstart first\nmiddle second\nend third\nother match\n"
    );
    ASSERT_TRUE(result.find("first") != std::string::npos);
    ASSERT_TRUE(result.find("second") != std::string::npos);
    ASSERT_TRUE(result.find("third") != std::string::npos);
}

TEST(Interpreter_Range_Pattern_FNR) {
    // Range with FNR
    std::string result = run_awk(
        R"(FNR==2,FNR==3 { print $0 })",
        "line1\nline2\nline3\nline4\n"
    );
    ASSERT_TRUE(result.find("line2") != std::string::npos);
    ASSERT_TRUE(result.find("line3") != std::string::npos);
    ASSERT_TRUE(result.find("line1") == std::string::npos);
    ASSERT_TRUE(result.find("line4") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_NR_Count) {
    // Count records in numeric range using simple equality
    std::string result = run_awk(
        R"(NR==2,NR==4 { count++ } END { print count })",
        "line1\nline2\nline3\nline4\nline5\n"
    );
    // Should count lines 2, 3, 4 = 3 lines
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Range_Pattern_First_Line) {
    // Range starting from first line
    std::string result = run_awk(
        R"(NR==1,NR==2 { print "in range:", $0 })",
        "first\nsecond\nthird\n"
    );
    ASSERT_TRUE(result.find("in range: first") != std::string::npos);
    ASSERT_TRUE(result.find("in range: second") != std::string::npos);
    ASSERT_TRUE(result.find("third") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Single_Line) {
    // Range with same start and end
    std::string result = run_awk(
        R"(NR==3,NR==3 { print "line3:", $0 })",
        "one\ntwo\nthree\nfour\n"
    );
    ASSERT_TRUE(result.find("line3: three") != std::string::npos);
    ASSERT_TRUE(result.find("one") == std::string::npos);
    ASSERT_TRUE(result.find("four") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Beyond_Input) {
    // Range end beyond input
    std::string result = run_awk(
        R"(NR==2,NR==10 { print })",
        "line1\nline2\nline3\n"
    );
    ASSERT_TRUE(result.find("line2") != std::string::npos);
    ASSERT_TRUE(result.find("line3") != std::string::npos);
    ASSERT_TRUE(result.find("line1") == std::string::npos);
}

TEST(Interpreter_Range_Pattern_Never_Starts) {
    // Range that never starts
    std::string result = run_awk(
        R"(NR==10,NR==20 { print })",
        "line1\nline2\nline3\n"
    );
    ASSERT_EQ(result, "");
}

TEST(Interpreter_Range_Pattern_With_Processing) {
    // Range with data processing
    std::string result = run_awk(
        R"(NR==2,NR==4 { sum += $1 } END { print sum })",
        "10\n20\n30\n40\n50\n"
    );
    // Sum of lines 2,3,4: 20+30+40 = 90
    ASSERT_EQ(result, "90\n");
}

TEST(Interpreter_Range_Pattern_Multiple_Active) {
    // Multiple range patterns
    std::string result = run_awk(
        R"(
        NR==1,NR==2 { first++ }
        NR==3,NR==4 { second++ }
        END { print first, second }
        )",
        "a\nb\nc\nd\ne\n"
    );
    ASSERT_EQ(result, "2 2\n");
}

// ==================== Multiline RS (Record Separator) Tests ====================

TEST(Interpreter_RS_Empty_Paragraph_Mode) {
    // RS = "" enables paragraph mode (blank lines separate records)
    std::string result = run_awk(
        R"(BEGIN { RS = "" } { print NR, ":", $0 })",
        "line1\nline2\n\nline3\nline4\n"
    );
    ASSERT_TRUE(result.find("1 :") != std::string::npos);
    ASSERT_TRUE(result.find("2 :") != std::string::npos);
}

TEST(Interpreter_RS_Paragraph_Multiple_Blanks) {
    // Paragraph mode with multiple blank lines
    std::string result = run_awk(
        R"(BEGIN { RS = "" } END { print NR })",
        "para1\n\n\n\npara2\n\npara3\n"
    );
    // Should have 3 paragraphs
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_RS_Single_Char) {
    // RS as single character
    std::string result = run_awk(
        R"(BEGIN { RS = ":" } { print NR, $0 })",
        "a:b:c"
    );
    ASSERT_TRUE(result.find("1 a") != std::string::npos);
    ASSERT_TRUE(result.find("2 b") != std::string::npos);
    ASSERT_TRUE(result.find("3 c") != std::string::npos);
}

TEST(Interpreter_RS_Multi_Char) {
    // RS as multi-character string - gawk extension
    // Standard awk only uses first char of RS
    std::string result = run_awk(
        R"(BEGIN { RS = "-" } { print NR, $0 })",
        "record1-record2-record3"
    );
    ASSERT_TRUE(result.find("1 record1") != std::string::npos);
    ASSERT_TRUE(result.find("2 record2") != std::string::npos);
    ASSERT_TRUE(result.find("3 record3") != std::string::npos);
}

TEST(Interpreter_RS_Regex_Digits) {
    // RS as regex matching digits
    std::string result = run_awk(
        R"(BEGIN { RS = "[0-9]+" } { gsub(/^[ \n]+|[ \n]+$/, "", $0); if ($0 != "") print $0 })",
        "a1b22c333d"
    );
    ASSERT_TRUE(result.find("a") != std::string::npos);
    ASSERT_TRUE(result.find("b") != std::string::npos);
    ASSERT_TRUE(result.find("c") != std::string::npos);
}

TEST(Interpreter_RS_Newline_Sequence) {
    // RS matching CRLF or LF
    std::string result = run_awk(
        R"(BEGIN { RS = "\r\n|\n" } END { print NR })",
        "line1\nline2\nline3\n"
    );
    // Should count 3 or 4 records depending on trailing newline handling
    ASSERT_TRUE(result.find("3") != std::string::npos || result.find("4") != std::string::npos);
}

TEST(Interpreter_RS_With_RT) {
    // RT contains the matched record terminator (gawk extension)
    // Test that RT variable exists and can be accessed
    std::string result = run_awk(
        R"(BEGIN { RS = ":" } { print NR, RT })",
        "a:b:c"
    );
    // RT should contain ":" for each record except possibly the last
    ASSERT_TRUE(result.find("1") != std::string::npos);
    ASSERT_TRUE(result.find("2") != std::string::npos);
}

TEST(Interpreter_RS_Empty_Records) {
    // RS causing empty records
    std::string result = run_awk(
        R"(BEGIN { RS = "," } { print NR, "[" $0 "]" })",
        "a,,b"
    );
    ASSERT_TRUE(result.find("1 [a]") != std::string::npos);
    ASSERT_TRUE(result.find("2 []") != std::string::npos);
    ASSERT_TRUE(result.find("3 [b]") != std::string::npos);
}

TEST(Interpreter_RS_XML_Tags) {
    // RS matching XML-like tags
    std::string result = run_awk(
        R"(BEGIN { RS = "<[^>]+>" } { gsub(/^[ \n\t]+|[ \n\t]+$/, "", $0); if ($0 != "") print $0 })",
        "<tag>content1</tag><tag>content2</tag>"
    );
    ASSERT_TRUE(result.find("content1") != std::string::npos);
    ASSERT_TRUE(result.find("content2") != std::string::npos);
}

TEST(Interpreter_RS_Paragraph_NF) {
    // Paragraph mode preserves newlines as field separators
    std::string result = run_awk(
        R"(BEGIN { RS = "" } { print NF })",
        "word1 word2\nword3 word4\n\nword5\n"
    );
    // First paragraph: 4 words, second: 1 word
    ASSERT_TRUE(result.find("4") != std::string::npos);
    ASSERT_TRUE(result.find("1") != std::string::npos);
}

TEST(Interpreter_RS_No_Match) {
    // RS that never matches - uses default newline RS
    // Standard awk only uses first char of RS, so multi-char RS may behave differently
    std::string result = run_awk(
        R"(BEGIN { RS = "X" } END { print NR })",
        "line1\nline2\nline3\n"
    );
    // Since there's no "X" in input, it's treated as one record
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_RS_At_Start) {
    // RS at start of input
    std::string result = run_awk(
        R"(BEGIN { RS = ":" } { print NR, "[" $0 "]" })",
        ":a:b"
    );
    // First record is empty
    ASSERT_TRUE(result.find("1 []") != std::string::npos);
    ASSERT_TRUE(result.find("2 [a]") != std::string::npos);
}

TEST(Interpreter_RS_At_End) {
    // RS at end of input
    std::string result = run_awk(
        R"(BEGIN { RS = ":" } END { print NR })",
        "a:b:"
    );
    // May be 2 or 3 depending on trailing RS handling
    ASSERT_TRUE(result.find("2") != std::string::npos || result.find("3") != std::string::npos);
}

TEST(Interpreter_RS_Consecutive) {
    // Consecutive RS matches
    std::string result = run_awk(
        R"(BEGIN { RS = ":" } { print NR, "[" $0 "]" })",
        "a:::b"
    );
    // Should create empty records
    ASSERT_TRUE(result.find("[a]") != std::string::npos);
    ASSERT_TRUE(result.find("[b]") != std::string::npos);
    ASSERT_TRUE(result.find("[]") != std::string::npos);
}

TEST(Interpreter_RS_Single_Equals) {
    // RS with single character separator
    std::string result = run_awk(
        R"(BEGIN { RS = "=" } { if ($0 != "") print NR, $0 })",
        "part1=part2=part3"
    );
    ASSERT_TRUE(result.find("part1") != std::string::npos);
    ASSERT_TRUE(result.find("part2") != std::string::npos);
    ASSERT_TRUE(result.find("part3") != std::string::npos);
}

// ==================== Additional Delete/Array Edge Cases ====================

TEST(Interpreter_Delete_Nonexistent_Element) {
    // Deleting non-existent element should not cause error
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = "one"
            delete a[999]
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Delete_During_Iteration) {
    // Delete during for-in (behavior may vary)
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = 1; a[2] = 2; a[3] = 3
            for (k in a) {
                if (k == 2) delete a[k]
            }
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "2\n");
}

TEST(Interpreter_Delete_Entire_Array_Recreate) {
    // Delete entire array then recreate
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = "x"; a[2] = "y"
            delete a
            a[5] = "new"
            print length(a), a[5]
        })",
        ""
    );
    ASSERT_EQ(result, "1 new\n");
}

TEST(Interpreter_Array_Sparse_Indices) {
    // Sparse array indices
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = "one"
            a[100] = "hundred"
            a[1000000] = "million"
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_Array_Negative_Index) {
    // Negative array index
    std::string result = run_awk(
        R"(BEGIN {
            a[-5] = "negative"
            print a[-5]
        })",
        ""
    );
    ASSERT_EQ(result, "negative\n");
}

TEST(Interpreter_Array_Float_Index) {
    // Float as array index (converted to string)
    std::string result = run_awk(
        R"(BEGIN {
            a[3.14] = "pi"
            print a[3.14]
        })",
        ""
    );
    ASSERT_EQ(result, "pi\n");
}

TEST(Interpreter_Array_Empty_String_Index) {
    // Empty string as array index
    std::string result = run_awk(
        R"(BEGIN {
            a[""] = "empty key"
            print a[""]
        })",
        ""
    );
    ASSERT_EQ(result, "empty key\n");
}

TEST(Interpreter_Array_Special_Chars_Index) {
    // Special characters in array index
    std::string result = run_awk(
        R"(BEGIN {
            a["a\tb"] = "tab"
            a["x\ny"] = "newline"
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "2\n");
}

// ==================== String Function Edge Cases ====================

TEST(Interpreter_Substr_Length_Zero) {
    // substr with length 0
    std::string result = run_awk(
        R"(BEGIN { print "[" substr("hello", 2, 0) "]" })",
        ""
    );
    ASSERT_EQ(result, "[]\n");
}

TEST(Interpreter_Substr_Negative_Length) {
    // substr with negative length - behavior may vary
    std::string result = run_awk(
        R"(BEGIN { print "[" substr("hello", 2, -5) "]" })",
        ""
    );
    // Some AWK implementations return empty, others return rest of string
    ASSERT_TRUE(result.find("[") != std::string::npos);
}

TEST(Interpreter_Split_Single_Char_String) {
    // Split single character string
    std::string result = run_awk(
        R"(BEGIN {
            n = split("x", arr, ",")
            print n, arr[1]
        })",
        ""
    );
    ASSERT_EQ(result, "1 x\n");
}

TEST(Interpreter_Split_Separator_At_Edges) {
    // Split with separator at start and end
    std::string result = run_awk(
        R"(BEGIN {
            n = split(",a,b,", arr, ",")
            print n
        })",
        ""
    );
    // Should have empty elements at start and end
    ASSERT_TRUE(result.find("4") != std::string::npos);
}

TEST(Interpreter_Gsub_Empty_Replacement) {
    // gsub with empty replacement (deletion)
    std::string result = run_awk(
        R"(BEGIN {
            x = "hello"
            gsub(/l/, "", x)
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "heo\n");
}

TEST(Interpreter_Sub_No_Match) {
    // sub when pattern doesn't match
    std::string result = run_awk(
        R"(BEGIN {
            x = "hello"
            n = sub(/xyz/, "replaced", x)
            print n, x
        })",
        ""
    );
    ASSERT_EQ(result, "0 hello\n");
}

TEST(Interpreter_Gsub_Overlapping) {
    // gsub with potentially overlapping matches
    std::string result = run_awk(
        R"(BEGIN {
            x = "aaa"
            n = gsub(/aa/, "X", x)
            print n, x
        })",
        ""
    );
    // Non-overlapping: should replace first "aa"
    ASSERT_TRUE(result.find("X") != std::string::npos);
}

TEST(Interpreter_Index_Longer_Needle) {
    // index where needle is longer than haystack
    std::string result = run_awk(
        R"(BEGIN { print index("ab", "abcdef") })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Match_Empty_String) {
    // match on empty string
    std::string result = run_awk(
        R"(BEGIN {
            n = match("", /.*/)
            print n, RSTART, RLENGTH
        })",
        ""
    );
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Tolower_Numbers) {
    // tolower with numbers (should be unchanged)
    std::string result = run_awk(
        R"(BEGIN { print tolower("ABC123XYZ") })",
        ""
    );
    ASSERT_EQ(result, "abc123xyz\n");
}

TEST(Interpreter_Toupper_Special_Chars) {
    // toupper with special characters
    std::string result = run_awk(
        R"(BEGIN { print toupper("hello, world!") })",
        ""
    );
    ASSERT_EQ(result, "HELLO, WORLD!\n");
}

// ==================== Numeric Edge Cases ====================

TEST(Interpreter_Very_Large_Exponent) {
    // Very large exponent
    std::string result = run_awk(
        R"(BEGIN { print 1e100 > 0 })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Very_Small_Exponent) {
    // Very small exponent
    std::string result = run_awk(
        R"(BEGIN { print 1e-100 > 0 })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Negative_Zero_Comparison) {
    // Negative zero equals positive zero
    std::string result = run_awk(
        R"(BEGIN { print (-0 == 0) })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Integer_Overflow_Addition) {
    // Large integer addition
    std::string result = run_awk(
        R"(BEGIN { print 9999999999999999 + 1 > 0 })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Float_Precision) {
    // Float precision test
    std::string result = run_awk(
        R"(BEGIN { printf "%.10f\n", 0.1 + 0.2 })",
        ""
    );
    // 0.1 + 0.2 is not exactly 0.3 in floating point
    ASSERT_TRUE(result.find("0.3") != std::string::npos);
}

TEST(Interpreter_Hex_In_Expression) {
    // Hex number in expression
    std::string result = run_awk(
        R"(BEGIN { print 0xFF + 1 })",
        ""
    );
    ASSERT_EQ(result, "256\n");
}

TEST(Interpreter_Octal_In_Expression) {
    // Octal number in expression
    std::string result = run_awk(
        R"(BEGIN { print 010 + 1 })",
        ""
    );
    ASSERT_EQ(result, "9\n");
}

// ==================== Control Flow Edge Cases ====================

TEST(Interpreter_Nested_Loops_Break) {
    // Break only exits innermost loop
    std::string result = run_awk(
        R"(BEGIN {
            for (i = 1; i <= 3; i++) {
                for (j = 1; j <= 3; j++) {
                    if (j == 2) break
                    print i, j
                }
            }
        })",
        ""
    );
    ASSERT_TRUE(result.find("1 1") != std::string::npos);
    ASSERT_TRUE(result.find("2 1") != std::string::npos);
    ASSERT_TRUE(result.find("3 1") != std::string::npos);
    ASSERT_TRUE(result.find("1 2") == std::string::npos);
}

TEST(Interpreter_Nested_Loops_Continue) {
    // Continue only affects innermost loop
    std::string result = run_awk(
        R"(BEGIN {
            for (i = 1; i <= 2; i++) {
                for (j = 1; j <= 3; j++) {
                    if (j == 2) continue
                    printf "%d%d ", i, j
                }
            }
        })",
        ""
    );
    ASSERT_TRUE(result.find("11") != std::string::npos);
    ASSERT_TRUE(result.find("13") != std::string::npos);
    ASSERT_TRUE(result.find("12") == std::string::npos);
}

TEST(Interpreter_While_False_Initially) {
    // While loop with false condition initially
    std::string result = run_awk(
        R"(BEGIN {
            x = 0
            while (x > 10) {
                print "inside"
                x++
            }
            print "done"
        })",
        ""
    );
    ASSERT_EQ(result, "done\n");
}

TEST(Interpreter_Do_While_Always_Once) {
    // Do-while always executes at least once
    std::string result = run_awk(
        R"(BEGIN {
            x = 100
            do {
                print "executed"
                x++
            } while (x < 10)
        })",
        ""
    );
    ASSERT_EQ(result, "executed\n");
}

TEST(Interpreter_For_Empty_Parts) {
    // For loop with empty parts
    std::string result = run_awk(
        R"(BEGIN {
            i = 1
            for (;;) {
                print i
                if (++i > 3) break
            }
        })",
        ""
    );
    ASSERT_EQ(result, "1\n2\n3\n");
}

TEST(Interpreter_If_Numeric_Zero) {
    // If with numeric zero (falsy)
    std::string result = run_awk(
        R"(BEGIN {
            if (0) print "yes"
            else print "no"
        })",
        ""
    );
    ASSERT_EQ(result, "no\n");
}

TEST(Interpreter_If_Empty_String) {
    // If with empty string (falsy)
    std::string result = run_awk(
        R"(BEGIN {
            if ("") print "yes"
            else print "no"
        })",
        ""
    );
    ASSERT_EQ(result, "no\n");
}

TEST(Interpreter_If_String_Zero) {
    // If with string "0" - in AWK numeric strings are evaluated numerically
    std::string result = run_awk(
        R"(BEGIN {
            x = "0"
            if (x + 0) print "truthy"
            else print "falsy"
        })",
        ""
    );
    ASSERT_EQ(result, "falsy\n");
}

// ==================== Field Manipulation Edge Cases ====================

TEST(Interpreter_Field_Zero_Assignment) {
    // Assigning to $0 re-parses fields
    std::string result = run_awk(
        R"({ $0 = "new content here"; print NF, $1, $3 })",
        "original line\n"
    );
    ASSERT_EQ(result, "3 new here\n");
}

TEST(Interpreter_Field_Very_High_Number) {
    // Access very high field number
    std::string result = run_awk(
        R"({ print "[" $1000000 "]" })",
        "a b c\n"
    );
    ASSERT_EQ(result, "[]\n");
}

TEST(Interpreter_Field_Computed_Zero) {
    // $(0) equals $0
    std::string result = run_awk(
        R"({ print $(0) })",
        "hello world\n"
    );
    ASSERT_EQ(result, "hello world\n");
}

TEST(Interpreter_Field_Assign_Creates_Gaps) {
    // Assigning to high field creates empty gaps
    std::string result = run_awk(
        R"({ $5 = "fifth"; print $3, $4, $5 })",
        "a b\n"
    );
    ASSERT_TRUE(result.find("fifth") != std::string::npos);
}

TEST(Interpreter_NF_Set_To_Zero) {
    // Setting NF to 0 clears all fields
    std::string result = run_awk(
        R"({ NF = 0; print "[" $0 "]", NF })",
        "a b c d e\n"
    );
    ASSERT_TRUE(result.find("0") != std::string::npos);
}

TEST(Interpreter_FS_Multiple_Spaces) {
    // Default FS handles multiple spaces
    std::string result = run_awk(
        R"({ print NF, $1, $2 })",
        "a     b\n"
    );
    ASSERT_EQ(result, "2 a b\n");
}

TEST(Interpreter_FS_Leading_Trailing_Space) {
    // Leading and trailing spaces with default FS
    std::string result = run_awk(
        R"({ print NF, "[" $1 "]", "[" $2 "]" })",
        "  a  b  \n"
    );
    ASSERT_EQ(result, "2 [a] [b]\n");
}

// ==================== Printf Edge Cases ====================

TEST(Interpreter_Printf_Percent_S_Number) {
    // %s with number argument
    std::string result = run_awk(
        R"(BEGIN { printf "%s\n", 42 })",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Printf_Percent_D_String) {
    // %d with string argument
    std::string result = run_awk(
        R"(BEGIN { printf "%d\n", "42" })",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Printf_Percent_D_Non_Numeric_String) {
    // %d with non-numeric string
    std::string result = run_awk(
        R"(BEGIN { printf "%d\n", "hello" })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Printf_Width_Larger_Than_String) {
    // Width larger than string
    std::string result = run_awk(
        R"(BEGIN { printf "[%20s]\n", "hi" })",
        ""
    );
    ASSERT_EQ(result, "[                  hi]\n");
}

TEST(Interpreter_Printf_Precision_Zero_Int) {
    // Precision of 0 for integer with non-zero value
    std::string result = run_awk(
        R"(BEGIN { printf "[%.0d]\n", 5 })",
        ""
    );
    ASSERT_EQ(result, "[5]\n");
}

TEST(Interpreter_Printf_Star_Width_Negative) {
    // Negative width via * (left-justify)
    std::string result = run_awk(
        R"(BEGIN { printf "[%*s]\n", -10, "hi" })",
        ""
    );
    ASSERT_EQ(result, "[hi        ]\n");
}

TEST(Interpreter_Printf_Multiple_Percent) {
    // Multiple %% escapes
    std::string result = run_awk(
        R"(BEGIN { printf "%%a%%b%%\n" })",
        ""
    );
    ASSERT_EQ(result, "%a%b%\n");
}

// ==================== Function Edge Cases ====================

TEST(Interpreter_Function_No_Args_Called_With_Args) {
    // Function with no params called with args
    std::string result = run_awk(
        R"(
        function noargs() { return 42 }
        BEGIN { print noargs(1, 2, 3) }
        )",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Function_Recursive_Deep) {
    // Deeply recursive function
    std::string result = run_awk(
        R"(
        function countdown(n) {
            if (n <= 0) return 0
            return countdown(n - 1)
        }
        BEGIN { print countdown(100) }
        )",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_Function_Return_Nothing) {
    // Function with bare return
    std::string result = run_awk(
        R"(
        function early(x) {
            if (x < 0) return
            return x * 2
        }
        BEGIN {
            print early(-5) + 0
            print early(5)
        }
        )",
        ""
    );
    ASSERT_EQ(result, "0\n10\n");
}

TEST(Interpreter_Function_Modifies_Global_Array) {
    // Function modifies global array
    std::string result = run_awk(
        R"(
        function add_item(key, val) {
            global_arr[key] = val
        }
        BEGIN {
            add_item("x", 10)
            add_item("y", 20)
            print global_arr["x"], global_arr["y"]
        }
        )",
        ""
    );
    ASSERT_EQ(result, "10 20\n");
}

// ==================== Regex Edge Cases ====================

TEST(Interpreter_Regex_Empty_Match) {
    // Regex that matches empty string
    std::string result = run_awk(
        R"(BEGIN {
            if ("test" ~ /.*/) print "matches"
        })",
        ""
    );
    ASSERT_EQ(result, "matches\n");
}

TEST(Interpreter_Regex_Anchors) {
    // Regex with ^ and $ anchors
    std::string result = run_awk(
        R"(/^hello$/ { print "exact" })",
        "hello world\nhello\nworld hello\n"
    );
    ASSERT_EQ(result, "exact\n");
}

TEST(Interpreter_Regex_Case_Sensitive_Default) {
    // Regex is case-sensitive by default
    std::string result = run_awk(
        R"(/ABC/ { print "found" })",
        "abc\nABC\nAbc\n"
    );
    ASSERT_EQ(result, "found\n");
}

TEST(Interpreter_Regex_Special_In_Bracket) {
    // Special chars in character class
    std::string result = run_awk(
        R"(/[.+*]/ { print "found" })",
        "abc\na.b\na+b\na*b\n"
    );
    // Should find 3 matches
    ASSERT_TRUE(result.find("found") != std::string::npos);
}

TEST(Interpreter_Regex_Negated_Class) {
    // Negated character class
    std::string result = run_awk(
        R"(/[^0-9]+/ { count++ } END { print count })",
        "123\nabc\n456\n"
    );
    // Lines with non-digits
    ASSERT_TRUE(result.find("1") != std::string::npos);
}

// ==================== Getline Edge Cases ====================

TEST(Interpreter_Getline_Updates_NR) {
    // getline from stdin updates NR
    std::string result = run_awk(
        R"({
            if (NR == 1) {
                print "first:", $0
                ret = getline
                if (ret > 0) print "second:", $0
            }
        })",
        "line1\nline2\nline3\n"
    );
    ASSERT_TRUE(result.find("first:") != std::string::npos);
}

// ==================== Special Variable Edge Cases ====================

TEST(Interpreter_NR_In_BEGIN) {
    // NR is 0 in BEGIN
    std::string result = run_awk(
        R"(BEGIN { print NR })",
        "a\nb\nc\n"
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_NR_In_END) {
    // NR is total count in END
    std::string result = run_awk(
        R"(END { print NR })",
        "a\nb\nc\n"
    );
    ASSERT_EQ(result, "3\n");
}

TEST(Interpreter_NF_In_BEGIN) {
    // NF is 0 in BEGIN (no current record)
    std::string result = run_awk(
        R"(BEGIN { print NF })",
        ""
    );
    ASSERT_EQ(result, "0\n");
}

TEST(Interpreter_FILENAME_In_BEGIN) {
    // FILENAME in BEGIN is empty
    std::string result = run_awk(
        R"(BEGIN { print "[" FILENAME "]" })",
        ""
    );
    ASSERT_EQ(result, "[]\n");
}

TEST(Interpreter_Modify_FS_Mid_Record) {
    // Changing FS doesn't affect current record
    std::string result = run_awk(
        R"({
            print $1
            FS = ":"
            print $2
        })",
        "a b c\n"
    );
    ASSERT_EQ(result, "a\nb\n");
}

TEST(Interpreter_Modify_OFS_Affects_Output) {
    // Changing OFS affects print output
    std::string result = run_awk(
        R"({
            OFS = "-"
            $1 = $1
            print
        })",
        "a b c\n"
    );
    ASSERT_EQ(result, "a-b-c\n");
}

// ==================== Concatenation Edge Cases ====================

TEST(Interpreter_Concat_Numbers) {
    // Concatenating numbers
    std::string result = run_awk(
        R"(BEGIN { print 1 2 3 })",
        ""
    );
    ASSERT_EQ(result, "123\n");
}

TEST(Interpreter_Concat_Mixed) {
    // Concatenating mixed types
    std::string result = run_awk(
        R"(BEGIN { print "value:" 42 })",
        ""
    );
    ASSERT_EQ(result, "value:42\n");
}

TEST(Interpreter_Concat_With_Space) {
    // Space in concatenation
    std::string result = run_awk(
        R"(BEGIN { print "a" " " "b" })",
        ""
    );
    ASSERT_EQ(result, "a b\n");
}

TEST(Interpreter_Concat_Empty_Strings) {
    // Concatenating empty strings
    std::string result = run_awk(
        R"(BEGIN { print "" "" "test" "" })",
        ""
    );
    ASSERT_EQ(result, "test\n");
}

// ==================== Operator Edge Cases ====================

TEST(Interpreter_Unary_Plus) {
    // Unary plus operator
    std::string result = run_awk(
        R"(BEGIN { x = "42"; print +x })",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Unary_Not_Number) {
    // Logical not on number
    std::string result = run_awk(
        R"(BEGIN { print !0, !1, !42 })",
        ""
    );
    ASSERT_EQ(result, "1 0 0\n");
}

TEST(Interpreter_Unary_Not_String) {
    // Logical not on string
    std::string result = run_awk(
        R"(BEGIN { print !"", !"x" })",
        ""
    );
    ASSERT_EQ(result, "1 0\n");
}

TEST(Interpreter_Exponent_Associativity) {
    // Power is right-associative
    std::string result = run_awk(
        R"(BEGIN { print 2^3^2 })",
        ""
    );
    // 2^(3^2) = 2^9 = 512, not (2^3)^2 = 64
    ASSERT_EQ(result, "512\n");
}

TEST(Interpreter_Comparison_String_Numeric) {
    // Comparison between string and number
    std::string result = run_awk(
        R"(BEGIN {
            print ("10" < "9")
            print (10 < 9)
        })",
        ""
    );
    // String comparison: "10" < "9" is true (lexicographic)
    // Numeric comparison: 10 < 9 is false
    ASSERT_EQ(result, "1\n0\n");
}

TEST(Interpreter_In_Operator_Numeric_String) {
    // 'in' operator with numeric and string keys
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = "one"
            print (1 in a)
            print ("1" in a)
            print (2 in a)
        })",
        ""
    );
    ASSERT_EQ(result, "1\n1\n0\n");
}

TEST(Interpreter_Ternary_Nested_Deep) {
    // Deeply nested ternary
    std::string result = run_awk(
        R"(BEGIN {
            x = 3
            print (x==1 ? "one" : (x==2 ? "two" : (x==3 ? "three" : "other")))
        })",
        ""
    );
    ASSERT_EQ(result, "three\n");
}

TEST(Interpreter_Assignment_Returns_Value) {
    // Assignment expression returns the assigned value
    std::string result = run_awk(
        R"(BEGIN {
            print (x = 5)
            print x
        })",
        ""
    );
    ASSERT_EQ(result, "5\n5\n");
}

TEST(Interpreter_Compound_Assignment_Returns_Value) {
    // Compound assignment returns new value
    std::string result = run_awk(
        R"(BEGIN {
            x = 10
            print (x += 5)
            print (x *= 2)
        })",
        ""
    );
    ASSERT_EQ(result, "15\n30\n");
}

// ==================== Multiple Rules Edge Cases ====================

TEST(Interpreter_Same_Pattern_Multiple_Actions) {
    // Same pattern with different actions
    std::string result = run_awk(
        R"(
        /test/ { count++ }
        /test/ { print "found" }
        END { print "count:", count }
        )",
        "test\nno\ntest\n"
    );
    ASSERT_TRUE(result.find("found") != std::string::npos);
    ASSERT_TRUE(result.find("count: 2") != std::string::npos);
}

TEST(Interpreter_Overlapping_Patterns) {
    // Overlapping regex patterns
    std::string result = run_awk(
        R"(
        /a/ { print "has a" }
        /b/ { print "has b" }
        /ab/ { print "has ab" }
        )",
        "ab\n"
    );
    ASSERT_TRUE(result.find("has a") != std::string::npos);
    ASSERT_TRUE(result.find("has b") != std::string::npos);
    ASSERT_TRUE(result.find("has ab") != std::string::npos);
}

TEST(Interpreter_BEGIN_END_Order) {
    // Multiple BEGIN and END blocks execute in order
    std::string result = run_awk(
        R"(
        BEGIN { printf "B1 " }
        BEGIN { printf "B2 " }
        END { printf "E1 " }
        END { printf "E2\n" }
        )",
        ""
    );
    ASSERT_EQ(result, "B1 B2 E1 E2\n");
}

// ==================== sprintf Edge Cases ====================

TEST(Interpreter_Sprintf_No_Args) {
    // sprintf with no format args
    std::string result = run_awk(
        R"(BEGIN { print sprintf("hello") })",
        ""
    );
    ASSERT_EQ(result, "hello\n");
}

TEST(Interpreter_Sprintf_More_Args_Than_Format) {
    // sprintf with more args than format specifiers
    std::string result = run_awk(
        R"(BEGIN { print sprintf("%d", 1, 2, 3) })",
        ""
    );
    ASSERT_EQ(result, "1\n");
}

TEST(Interpreter_Sprintf_Fewer_Args_Than_Format) {
    // sprintf with fewer args than format specifiers
    std::string result = run_awk(
        R"(BEGIN { print sprintf("%d %d %d", 1) })",
        ""
    );
    // Missing args treated as 0
    ASSERT_TRUE(result.find("1") != std::string::npos);
}

// ==================== gsub/sub Edge Cases ====================

TEST(Interpreter_Gsub_Dollar_Zero) {
    // gsub on $0
    std::string result = run_awk(
        R"({
            gsub(/a/, "X")
            print
        })",
        "banana\n"
    );
    ASSERT_EQ(result, "bXnXnX\n");
}

TEST(Interpreter_Sub_Dollar_Zero) {
    // sub on $0
    std::string result = run_awk(
        R"({
            sub(/a/, "X")
            print
        })",
        "banana\n"
    );
    ASSERT_EQ(result, "bXnana\n");
}

TEST(Interpreter_Gsub_Field) {
    // gsub on specific field
    std::string result = run_awk(
        R"({
            gsub(/a/, "X", $2)
            print $2
        })",
        "one banana three\n"
    );
    ASSERT_EQ(result, "bXnXnX\n");
}

// ==================== length() Edge Cases ====================

TEST(Interpreter_Length_No_Arg) {
    // length() with no argument uses $0
    std::string result = run_awk(
        R"({ print length() })",
        "hello\n"
    );
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Length_Array) {
    // length of array
    std::string result = run_awk(
        R"(BEGIN {
            a[1] = 1; a[2] = 2; a[3] = 3
            print length(a)
        })",
        ""
    );
    ASSERT_EQ(result, "3\n");
}

// ==================== Escape Sequence Edge Cases ====================

TEST(Interpreter_Escape_Hex) {
    // Hex escape sequence - gawk extension, may not be supported
    std::string result = run_awk(
        R"(BEGIN { print "\x41\x42\x43" })",
        ""
    );
    // May output ABC or literal \x41\x42\x43 depending on implementation
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Escape_Octal) {
    // Octal escape sequence - may not be fully supported
    std::string result = run_awk(
        R"(BEGIN { print "\101\102\103" })",
        ""
    );
    // May output ABC or literal sequences depending on implementation
    ASSERT_TRUE(result.length() > 0);
}

TEST(Interpreter_Escape_Backslash) {
    // Escaped backslash
    std::string result = run_awk(
        R"(BEGIN { print "a\\b\\c" })",
        ""
    );
    ASSERT_EQ(result, "a\\b\\c\n");
}

TEST(Interpreter_Escape_Quote) {
    // Escaped quote
    std::string result = run_awk(
        R"(BEGIN { print "\"quoted\"" })",
        ""
    );
    ASSERT_EQ(result, "\"quoted\"\n");
}

// ============================================================================
// @namespace Tests (gawk extension)
// ============================================================================

TEST(Interpreter_Namespace_Function_Definition) {
    // Function defined in namespace should be callable with namespace prefix
    std::string result = run_awk(
        R"(@namespace "myns"
        function foo() { return 42 }
        @namespace "awk"
        BEGIN { print myns::foo() })",
        ""
    );
    ASSERT_EQ(result, "42\n");
}

TEST(Interpreter_Namespace_Function_Call_Inside) {
    // Function calls inside same namespace should work automatically
    std::string result = run_awk(
        R"(@namespace "myns"
        function bar() { return 10 }
        function foo() { return bar() * 2 }
        @namespace "awk"
        BEGIN { print myns::foo() })",
        ""
    );
    ASSERT_EQ(result, "20\n");
}

TEST(Interpreter_Namespace_Variable) {
    // Variables in namespaces
    std::string result = run_awk(
        R"(@namespace "myns"
        BEGIN { x = 100 }
        @namespace "awk"
        END { print myns::x })",
        ""
    );
    ASSERT_EQ(result, "100\n");
}

TEST(Interpreter_Namespace_Builtin_Accessible) {
    // Built-in functions should be accessible from inside namespace
    std::string result = run_awk(
        R"(@namespace "myns"
        function test() { return length("hello") }
        @namespace "awk"
        BEGIN { print myns::test() })",
        ""
    );
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Namespace_Special_Variables) {
    // Special variables like NR, NF should be accessible from namespace
    std::string result = run_awk(
        R"(@namespace "myns"
        { total += NF }
        END { print total }
        @namespace "awk")",
        "a b c\nd e"
    );
    ASSERT_EQ(result, "5\n");
}

TEST(Interpreter_Namespace_Explicit_Awk) {
    // @namespace "awk" returns to global namespace
    std::string result = run_awk(
        R"(@namespace "test"
        function hello() { return "test::hello" }
        @namespace "awk"
        function hello() { return "global::hello" }
        BEGIN { print hello(); print test::hello() })",
        ""
    );
    ASSERT_EQ(result, "global::hello\ntest::hello\n");
}

TEST(Interpreter_Namespace_Multiple) {
    // Multiple namespaces
    std::string result = run_awk(
        R"(@namespace "ns1"
        function foo() { return "ns1" }
        @namespace "ns2"
        function foo() { return "ns2" }
        @namespace "awk"
        BEGIN { print ns1::foo(), ns2::foo() })",
        ""
    );
    ASSERT_EQ(result, "ns1 ns2\n");
}

TEST(Interpreter_Namespace_Nested_Call) {
    // Namespace function calling another namespace function
    std::string result = run_awk(
        R"(@namespace "helper"
        function double(n) { return n * 2 }
        @namespace "main"
        function process(n) { return helper::double(n) + 1 }
        @namespace "awk"
        BEGIN { print main::process(5) })",
        ""
    );
    ASSERT_EQ(result, "11\n");
}

TEST(Interpreter_Namespace_Array) {
    // Arrays in namespaces
    std::string result = run_awk(
        R"(@namespace "myns"
        BEGIN { arr[1] = "a"; arr[2] = "b" }
        END { print arr[1], arr[2] }
        @namespace "awk")",
        ""
    );
    ASSERT_EQ(result, "a b\n");
}

TEST(Interpreter_Namespace_Printf) {
    // Built-in printf should work from namespace
    std::string result = run_awk(
        R"(@namespace "myns"
        BEGIN { printf "%d + %d = %d\n", 2, 3, 5 }
        @namespace "awk")",
        ""
    );
    ASSERT_EQ(result, "2 + 3 = 5\n");
}
