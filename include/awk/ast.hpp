#ifndef AWK_AST_HPP
#define AWK_AST_HPP

#include "token.hpp"
#include <memory>
#include <vector>
#include <string>
#include <variant>

namespace awk {

// Forward declarations
struct Expr;
struct Stmt;

// Unique pointer aliases
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// ============================================================================
// Expressions
// ============================================================================

// Base struct for all expressions
struct Expr {
    size_t line = 0;
    size_t column = 0;

    virtual ~Expr() = default;

    // Visitor pattern support
    template<typename R, typename Visitor>
    R accept(Visitor& visitor);
};

// Literal value (number, string)
struct LiteralExpr : Expr {
    std::variant<double, std::string> value;

    explicit LiteralExpr(double num) : value(num) {}
    explicit LiteralExpr(std::string str) : value(std::move(str)) {}

    bool is_number() const { return std::holds_alternative<double>(value); }
    bool is_string() const { return std::holds_alternative<std::string>(value); }

    double as_number() const { return std::get<double>(value); }
    const std::string& as_string() const { return std::get<std::string>(value); }
};

// Regex-Literal
struct RegexExpr : Expr {
    std::string pattern;

    explicit RegexExpr(std::string pat) : pattern(std::move(pat)) {}
};

// Variable
struct VariableExpr : Expr {
    std::string name;

    explicit VariableExpr(std::string n) : name(std::move(n)) {}
};

// Field access ($0, $1, $NF, $(expr))
struct FieldExpr : Expr {
    ExprPtr index;

    explicit FieldExpr(ExprPtr idx) : index(std::move(idx)) {}
};

// Array access (arr[key] or arr[k1, k2, ...])
struct ArrayAccessExpr : Expr {
    std::string name;
    std::vector<ExprPtr> indices;

    ArrayAccessExpr(std::string n, std::vector<ExprPtr> idx)
        : name(std::move(n)), indices(std::move(idx)) {}
};

// Binary expression
struct BinaryExpr : Expr {
    ExprPtr left;
    TokenType op;
    ExprPtr right;

    BinaryExpr(ExprPtr l, TokenType o, ExprPtr r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

// Unary expression (!, -, +, ++, --)
struct UnaryExpr : Expr {
    TokenType op;
    ExprPtr operand;
    bool prefix;  // true for ++x, false for x++

    UnaryExpr(TokenType o, ExprPtr expr, bool pre = true)
        : op(o), operand(std::move(expr)), prefix(pre) {}
};

// Ternary expression (cond ? then : else)
struct TernaryExpr : Expr {
    ExprPtr condition;
    ExprPtr then_expr;
    ExprPtr else_expr;

    TernaryExpr(ExprPtr cond, ExprPtr then_e, ExprPtr else_e)
        : condition(std::move(cond))
        , then_expr(std::move(then_e))
        , else_expr(std::move(else_e)) {}
};

// Assignment (=, +=, -=, etc.)
struct AssignExpr : Expr {
    ExprPtr target;  // Variable, Field or ArrayAccess
    TokenType op;
    ExprPtr value;

    AssignExpr(ExprPtr tgt, TokenType o, ExprPtr val)
        : target(std::move(tgt)), op(o), value(std::move(val)) {}
};

// Function call
struct CallExpr : Expr {
    std::string function_name;
    std::vector<ExprPtr> arguments;

    CallExpr(std::string name, std::vector<ExprPtr> args)
        : function_name(std::move(name)), arguments(std::move(args)) {}
};

// Indirect function call (gawk extension: @varname(args))
struct IndirectCallExpr : Expr {
    ExprPtr func_name_expr;  // Expression yielding the function name
    std::vector<ExprPtr> arguments;

    IndirectCallExpr(ExprPtr name_expr, std::vector<ExprPtr> args)
        : func_name_expr(std::move(name_expr)), arguments(std::move(args)) {}
};

// Regex match (str ~ /regex/ or str !~ /regex/)
struct MatchExpr : Expr {
    ExprPtr string;
    ExprPtr regex;  // RegexExpr or dynamic expression
    bool negated;   // true for !~

    MatchExpr(ExprPtr str, ExprPtr re, bool neg = false)
        : string(std::move(str)), regex(std::move(re)), negated(neg) {}
};

// String concatenation (implicit by adjacency)
struct ConcatExpr : Expr {
    std::vector<ExprPtr> parts;

    explicit ConcatExpr(std::vector<ExprPtr> p) : parts(std::move(p)) {}
};

// Getline expression
struct GetlineExpr : Expr {
    ExprPtr variable;  // Optional: getline var
    ExprPtr file;      // Optional: < file
    ExprPtr command;   // Optional: command |
    bool coprocess;    // |&

    GetlineExpr()
        : variable(nullptr)
        , file(nullptr)
        , command(nullptr)
        , coprocess(false) {}
};

// In-Operator (key in array)
struct InExpr : Expr {
    std::vector<ExprPtr> keys;  // Kann mehrere Keys haben (k1, k2) in arr
    std::string array_name;

    InExpr(std::vector<ExprPtr> k, std::string arr)
        : keys(std::move(k)), array_name(std::move(arr)) {}
};

// ============================================================================
// Statements
// ============================================================================

// Base struct for all statements
struct Stmt {
    size_t line = 0;
    size_t column = 0;

    virtual ~Stmt() = default;

    template<typename R, typename Visitor>
    R accept(Visitor& visitor);
};

// Expression Statement
struct ExprStmt : Stmt {
    ExprPtr expression;

    explicit ExprStmt(ExprPtr expr) : expression(std::move(expr)) {}
};

// Redirect type for print/printf
enum class RedirectType {
    NONE,
    WRITE,      // >
    APPEND,     // >>
    PIPE,       // |
    PIPE_BOTH   // |& (coprocess - bidirectional pipe)
};

// Print Statement
struct PrintStmt : Stmt {
    std::vector<ExprPtr> arguments;
    ExprPtr output_redirect;
    RedirectType redirect_type = RedirectType::NONE;

    PrintStmt() = default;
    explicit PrintStmt(std::vector<ExprPtr> args) : arguments(std::move(args)) {}
};

// Printf Statement
struct PrintfStmt : Stmt {
    ExprPtr format;
    std::vector<ExprPtr> arguments;
    ExprPtr output_redirect;
    RedirectType redirect_type = RedirectType::NONE;

    explicit PrintfStmt(ExprPtr fmt) : format(std::move(fmt)) {}
};

// Block Statement
struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;

    BlockStmt() = default;
    explicit BlockStmt(std::vector<StmtPtr> stmts) : statements(std::move(stmts)) {}
};

// If Statement
struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;  // Optional

    IfStmt(ExprPtr cond, StmtPtr then_b, StmtPtr else_b = nullptr)
        : condition(std::move(cond))
        , then_branch(std::move(then_b))
        , else_branch(std::move(else_b)) {}
};

// While Statement
struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;

    WhileStmt(ExprPtr cond, StmtPtr b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

// Do-While Statement
struct DoWhileStmt : Stmt {
    StmtPtr body;
    ExprPtr condition;

    DoWhileStmt(StmtPtr b, ExprPtr cond)
        : body(std::move(b)), condition(std::move(cond)) {}
};

// For Statement (C-style)
struct ForStmt : Stmt {
    StmtPtr init;       // Optional
    ExprPtr condition;  // Optional
    ExprPtr update;     // Optional
    StmtPtr body;

    ForStmt(StmtPtr i, ExprPtr c, ExprPtr u, StmtPtr b)
        : init(std::move(i))
        , condition(std::move(c))
        , update(std::move(u))
        , body(std::move(b)) {}
};

// For-In Statement (Array-Iteration)
struct ForInStmt : Stmt {
    std::string variable;
    std::string array_name;
    StmtPtr body;

    ForInStmt(std::string var, std::string arr, StmtPtr b)
        : variable(std::move(var))
        , array_name(std::move(arr))
        , body(std::move(b)) {}
};

// Switch Statement (gawk)
struct SwitchStmt : Stmt {
    ExprPtr expression;
    std::vector<std::pair<ExprPtr, StmtPtr>> cases;  // case expr: stmt
    StmtPtr default_case;  // Optional

    explicit SwitchStmt(ExprPtr expr)
        : expression(std::move(expr)), default_case(nullptr) {}
};

// Control flow statements
struct BreakStmt : Stmt {};
struct ContinueStmt : Stmt {};
struct NextStmt : Stmt {};
struct NextfileStmt : Stmt {};

struct ExitStmt : Stmt {
    ExprPtr status;  // Optional

    ExitStmt() : status(nullptr) {}
    explicit ExitStmt(ExprPtr s) : status(std::move(s)) {}
};

struct ReturnStmt : Stmt {
    ExprPtr value;  // Optional

    ReturnStmt() : value(nullptr) {}
    explicit ReturnStmt(ExprPtr v) : value(std::move(v)) {}
};

// Delete Statement
struct DeleteStmt : Stmt {
    std::string array_name;
    std::vector<ExprPtr> indices;  // Empty = delete entire array

    DeleteStmt(std::string arr, std::vector<ExprPtr> idx)
        : array_name(std::move(arr)), indices(std::move(idx)) {}
};

// ============================================================================
// Program Structure
// ============================================================================

// Function definition
struct FunctionDef {
    std::string name;
    std::vector<std::string> parameters;
    StmtPtr body;
    size_t line = 0;
    size_t column = 0;

    FunctionDef(std::string n, std::vector<std::string> params, StmtPtr b)
        : name(std::move(n))
        , parameters(std::move(params))
        , body(std::move(b)) {}
};

// Pattern types
enum class PatternType {
    BEGIN,
    END,
    BEGINFILE,
    ENDFILE,
    EXPRESSION,  // Any expression
    REGEX,       // /pattern/
    RANGE,       // pat1, pat2
    EMPTY        // Empty pattern (matches everything)
};

// Pattern
struct Pattern {
    PatternType type = PatternType::EMPTY;
    ExprPtr expr;       // For EXPRESSION, REGEX
    ExprPtr range_end;  // For RANGE (the second pattern)
    bool range_active = false;  // Runtime state for range patterns

    Pattern() = default;

    static Pattern begin() {
        Pattern p;
        p.type = PatternType::BEGIN;
        return p;
    }

    static Pattern end() {
        Pattern p;
        p.type = PatternType::END;
        return p;
    }

    static Pattern begin_file() {
        Pattern p;
        p.type = PatternType::BEGINFILE;
        return p;
    }

    static Pattern end_file() {
        Pattern p;
        p.type = PatternType::ENDFILE;
        return p;
    }

    static Pattern empty() {
        Pattern p;
        p.type = PatternType::EMPTY;
        return p;
    }

    static Pattern expression(ExprPtr e) {
        Pattern p;
        p.type = PatternType::EXPRESSION;
        p.expr = std::move(e);
        return p;
    }

    static Pattern regex(ExprPtr e) {
        Pattern p;
        p.type = PatternType::REGEX;
        p.expr = std::move(e);
        return p;
    }

    static Pattern range(ExprPtr start, ExprPtr end_pat) {
        Pattern p;
        p.type = PatternType::RANGE;
        p.expr = std::move(start);
        p.range_end = std::move(end_pat);
        return p;
    }
};

// Rule (Pattern + Action)
struct Rule {
    Pattern pattern;
    StmtPtr action;  // nullptr = default action { print $0 }

    Rule(Pattern pat, StmtPtr act)
        : pattern(std::move(pat)), action(std::move(act)) {}
};

// Complete AWK program
struct Program {
    std::vector<std::unique_ptr<FunctionDef>> functions;
    std::vector<std::unique_ptr<Rule>> rules;
};

// ============================================================================
// Visitor Interface (optional, for type erasure)
// ============================================================================

// Helper class for visitor pattern with std::variant
template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template<typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace awk

#endif // AWK_AST_HPP
