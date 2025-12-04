#include "awk/parser.hpp"
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace awk {

Parser::Parser(Lexer& lexer)
    : lexer_(lexer) {
    advance();  // Load first token
}

Parser::Parser(Lexer& lexer, std::set<std::string>& included_files,
               const std::string& current_file, const std::string& base_path)
    : lexer_(lexer), included_files_(&included_files),
      current_file_(current_file), base_path_(base_path) {
    advance();
}

std::set<std::string>& Parser::get_included_files() {
    return included_files_ ? *included_files_ : local_included_files_;
}

std::string Parser::read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Parser::get_directory(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos == std::string::npos) {
        return ".";
    }
    return filepath.substr(0, pos);
}

std::string Parser::normalize_path(const std::string& path) {
    // Simple normalization: backslashes to slashes, remove double slashes
    std::string result;
    result.reserve(path.size());
    char prev = '\0';
    for (char c : path) {
        if (c == '\\') c = '/';
        if (c == '/' && prev == '/') continue;
        result += c;
        prev = c;
    }
    // Remove trailing slash
    while (result.size() > 1 && result.back() == '/') {
        result.pop_back();
    }
    return result;
}

std::string Parser::resolve_include_path(const std::string& filename) const {
    // If absolute path, use directly
    if (!filename.empty() && (filename[0] == '/' ||
        (filename.size() > 1 && filename[1] == ':'))) {
        return normalize_path(filename);
    }

    // Relative path: search relative to base directory
    std::string path;
    if (!base_path_.empty()) {
        path = base_path_ + "/" + filename;
    } else {
        path = filename;
    }
    return normalize_path(path);
}

std::unique_ptr<Program> Parser::parse_file(const std::string& filename) {
    std::string source = read_file(filename);
    std::string base = get_directory(filename);
    std::string normalized = normalize_path(filename);

    Lexer lexer(source);
    std::set<std::string> included;
    included.insert(normalized);

    Parser parser(lexer, included, normalized, base);
    return parser.parse();
}

std::unique_ptr<Program> Parser::parse_string(const std::string& source,
                                               const std::string& base_path) {
    Lexer lexer(source);
    std::set<std::string> included;

    Parser parser(lexer, included, "", base_path);
    return parser.parse();
}

// ============================================================================
// Token Management
// ============================================================================

void Parser::advance() {
    previous_ = current_;

    while (true) {
        current_ = lexer_.next_token();
        if (current_.type != TokenType::ERROR) break;

        error_at_current(current_.lexeme);
    }
}

bool Parser::check(TokenType type) const {
    return current_.type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        Token t = current_;
        advance();
        return t;
    }

    error_at_current(message);
    return Token(TokenType::ERROR, message, current_.line, current_.column);
}

bool Parser::is_at_end() const {
    return current_.type == TokenType::END_OF_FILE;
}

bool Parser::peek_next_is(TokenType type) {
    Token next = lexer_.peek_token();
    return next.type == type;
}

void Parser::skip_newlines() {
    while (match(TokenType::NEWLINE)) {}
}

void Parser::skip_optional_newlines() {
    while (check(TokenType::NEWLINE)) {
        advance();
    }
}

bool Parser::is_statement_terminator() const {
    return check(TokenType::NEWLINE) ||
           check(TokenType::SEMICOLON) ||
           check(TokenType::RBRACE) ||
           is_at_end();
}

void Parser::consume_statement_terminator() {
    if (check(TokenType::NEWLINE) || check(TokenType::SEMICOLON)) {
        advance();
        skip_newlines();
    }
}

// ============================================================================
// Error Handling
// ============================================================================

void Parser::error(const std::string& message) {
    error_at_current(message);
}

void Parser::error_at_current(const std::string& message) {
    if (panic_mode_) return;
    panic_mode_ = true;
    had_error_ = true;

    std::ostringstream oss;
    oss << "[Line " << current_.line << ":" << current_.column << "] Error";

    if (current_.type == TokenType::END_OF_FILE) {
        oss << " at end";
    } else if (current_.type != TokenType::ERROR) {
        oss << " at '" << current_.lexeme << "'";
    }

    oss << ": " << message;
    errors_.push_back(oss.str());
}

void Parser::synchronize() {
    panic_mode_ = false;

    while (!is_at_end()) {
        // Continue after newline or semicolon
        if (previous_.type == TokenType::NEWLINE ||
            previous_.type == TokenType::SEMICOLON) {
            return;
        }

        // Also at certain keywords
        switch (current_.type) {
            case TokenType::FUNCTION:
            case TokenType::BEGIN_KW:
            case TokenType::END_KW:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::FOR:
            case TokenType::RETURN:
            case TokenType::PRINT:
            case TokenType::PRINTF:
            case TokenType::AT_INCLUDE:  // gawk Extension
                return;
            default:
                break;
        }

        advance();
    }
}

// ============================================================================
// Program Structure
// ============================================================================

std::unique_ptr<Program> Parser::parse() {
    auto prog = std::make_unique<Program>();

    skip_newlines();

    while (!is_at_end()) {
        try {
            // @include directive? (gawk extension)
            if (check(TokenType::AT_INCLUDE)) {
                process_include(prog);
            }
            // @namespace directive? (gawk extension)
            else if (check(TokenType::AT_NAMESPACE)) {
                process_namespace();
            }
            // Function definition?
            else if (check(TokenType::FUNCTION)) {
                auto func = function_definition();
                if (func) {
                    prog->functions.push_back(std::move(func));
                }
            } else {
                // Rule (Pattern { Action })
                auto r = rule();
                if (r) {
                    prog->rules.push_back(std::move(r));
                }
            }

            skip_newlines();
        } catch (...) {
            synchronize();
        }
    }

    return prog;
}

void Parser::process_include(std::unique_ptr<Program>& prog) {
    advance();  // Consume @include

    // Filename can be "filename" or <filename>
    std::string filename;

    if (check(TokenType::STRING)) {
        filename = std::get<std::string>(current_.literal);
        advance();
    } else if (check(TokenType::LT)) {
        // <filename> format
        advance();  // Consume <

        // Collect characters until >
        // Since the lexer doesn't support this directly, treat it as identifier
        if (check(TokenType::IDENTIFIER)) {
            filename = current_.lexeme;
            advance();
        } else {
            error("Expected filename after '<' in @include");
            return;
        }

        if (!match(TokenType::GT)) {
            error("Expected '>' after filename in @include");
            return;
        }
    } else {
        error("Expected filename after @include");
        return;
    }

    // Resolve path
    std::string resolved_path = resolve_include_path(filename);

    // Cycle detection
    auto& included = get_included_files();
    if (included.count(resolved_path)) {
        // File was already included - silently ignore (like gawk)
        consume_statement_terminator();
        return;
    }

    // Mark as included
    included.insert(resolved_path);

    // Read file
    std::string source;
    try {
        source = read_file(resolved_path);
    } catch (const std::exception& e) {
        error(std::string("@include: ") + e.what());
        return;
    }

    // Create new parser for the included file
    Lexer include_lexer(source);
    std::string include_base = get_directory(resolved_path);
    Parser include_parser(include_lexer, included, resolved_path, include_base);

    // Parse included file
    auto included_prog = include_parser.parse();

    // Take over errors
    if (include_parser.had_error()) {
        had_error_ = true;
        for (const auto& err : include_parser.errors()) {
            errors_.push_back(err);
        }
    }

    // Take over functions and rules from included file
    if (included_prog) {
        for (auto& func : included_prog->functions) {
            prog->functions.push_back(std::move(func));
        }
        for (auto& rule : included_prog->rules) {
            prog->rules.push_back(std::move(rule));
        }
    }

    consume_statement_terminator();
}

void Parser::process_namespace() {
    advance();  // Consume @namespace

    // Namespace name must be a string
    if (!check(TokenType::STRING)) {
        error("Expected namespace name as string after @namespace");
        return;
    }

    std::string ns_name = std::get<std::string>(current_.literal);
    advance();

    // Special namespace "awk" means return to global namespace
    if (ns_name == "awk") {
        current_namespace_.clear();
    } else {
        current_namespace_ = ns_name;
    }

    consume_statement_terminator();
}

std::string Parser::qualify_name(const std::string& name) const {
    // If already qualified (contains ::), don't qualify again
    if (name.find("::") != std::string::npos) {
        return name;
    }

    // If no namespace active, name unchanged
    if (current_namespace_.empty()) {
        return name;
    }

    // Qualify name with namespace
    return current_namespace_ + "::" + name;
}

std::unique_ptr<FunctionDef> Parser::function_definition() {
    size_t line = current_.line;
    size_t col = current_.column;

    consume(TokenType::FUNCTION, "Expected 'function'");

    Token name_token = consume(TokenType::IDENTIFIER, "Expected function name");
    std::string name = name_token.lexeme;

    // Qualified name with namespace::name? (gawk extension)
    if (match(TokenType::COLON_COLON)) {
        Token qual_name = consume(TokenType::IDENTIFIER, "Expected function name after '::'");
        name = name + "::" + qual_name.lexeme;
    } else {
        // Automatically qualify with current namespace
        name = qualify_name(name);
    }

    consume(TokenType::LPAREN, "Expected '(' after function name");

    std::vector<std::string> params;

    if (!check(TokenType::RPAREN)) {
        do {
            Token param = consume(TokenType::IDENTIFIER, "Expected parameter name");
            params.push_back(param.lexeme);
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expected ')' after parameters");

    skip_optional_newlines();

    StmtPtr body = block();

    auto func = std::make_unique<FunctionDef>(name, std::move(params), std::move(body));
    func->line = line;
    func->column = col;

    return func;
}

std::unique_ptr<Rule> Parser::rule() {
    Pattern pattern = parse_pattern();

    skip_optional_newlines();

    StmtPtr action = nullptr;

    // Action is optional for BEGIN/END, but also for other patterns
    if (check(TokenType::LBRACE)) {
        action = block();
    } else if (pattern.type != PatternType::BEGIN &&
               pattern.type != PatternType::END &&
               pattern.type != PatternType::BEGINFILE &&
               pattern.type != PatternType::ENDFILE) {
        // No block = implicit { print $0 }
        // We set action to nullptr, the interpreter handles this
    }

    return std::make_unique<Rule>(std::move(pattern), std::move(action));
}

Pattern Parser::parse_pattern() {
    // Special patterns
    if (match(TokenType::BEGIN_KW)) {
        return Pattern::begin();
    }
    if (match(TokenType::END_KW)) {
        return Pattern::end();
    }
    if (match(TokenType::BEGINFILE_KW)) {
        return Pattern::begin_file();
    }
    if (match(TokenType::ENDFILE_KW)) {
        return Pattern::end_file();
    }

    // Empty pattern (only action block)?
    if (check(TokenType::LBRACE)) {
        return Pattern::empty();
    }

    // Negated regex pattern !/regex/ ?
    if (check(TokenType::NOT)) {
        // Look ahead if a regex comes after NOT
        // For this we need to peek the lexer in regex context
        lexer_.expect_regex();
        Token next = lexer_.peek_token();

        if (next.type == TokenType::REGEX) {
            advance();  // Consume NOT, current_ is now REGEX
            auto regex = std::make_unique<RegexExpr>(std::get<std::string>(current_.literal));
            advance();  // Consume REGEX token

            // Create UnaryExpr with NOT around the regex
            auto negated = std::make_unique<UnaryExpr>(TokenType::NOT, std::move(regex));
            return Pattern::expression(std::move(negated));
        }
        // No regex after NOT - fallthrough to normal expression parsing
    }

    // Regex pattern?
    if (check(TokenType::REGEX)) {
        // Regex was recognized - current_ contains the REGEX token
        auto regex = std::make_unique<RegexExpr>(std::get<std::string>(current_.literal));
        advance();  // Consume REGEX token
        ExprPtr expr = std::move(regex);

        // Range-Pattern?
        skip_optional_newlines();
        if (match(TokenType::COMMA)) {
            skip_optional_newlines();
            ExprPtr end_expr = expression();
            return Pattern::range(std::move(expr), std::move(end_expr));
        }

        return Pattern::regex(std::move(expr));
    }

    // Expression-Pattern
    ExprPtr expr = expression();

    // Range-Pattern?
    skip_optional_newlines();
    if (match(TokenType::COMMA)) {
        skip_optional_newlines();
        ExprPtr end_expr = expression();
        return Pattern::range(std::move(expr), std::move(end_expr));
    }

    return Pattern::expression(std::move(expr));
}

// ============================================================================
// Statements
// ============================================================================

StmtPtr Parser::statement() {
    skip_optional_newlines();

    if (check(TokenType::LBRACE)) return block();
    if (match(TokenType::IF)) return if_statement();
    if (match(TokenType::WHILE)) return while_statement();
    if (match(TokenType::DO)) return do_while_statement();
    if (match(TokenType::FOR)) return for_statement();
    if (match(TokenType::SWITCH)) return switch_statement();
    if (check(TokenType::PRINT)) return print_statement();
    if (check(TokenType::PRINTF)) return printf_statement();
    if (match(TokenType::DELETE)) return delete_statement();

    if (match(TokenType::BREAK)) {
        consume_statement_terminator();
        return std::make_unique<BreakStmt>();
    }
    if (match(TokenType::CONTINUE)) {
        consume_statement_terminator();
        return std::make_unique<ContinueStmt>();
    }
    if (match(TokenType::NEXT)) {
        consume_statement_terminator();
        return std::make_unique<NextStmt>();
    }
    if (match(TokenType::NEXTFILE)) {
        consume_statement_terminator();
        return std::make_unique<NextfileStmt>();
    }
    if (match(TokenType::EXIT)) {
        ExprPtr status = nullptr;
        if (!is_statement_terminator()) {
            status = expression();
        }
        consume_statement_terminator();
        return std::make_unique<ExitStmt>(std::move(status));
    }
    if (match(TokenType::RETURN)) {
        ExprPtr value = nullptr;
        if (!is_statement_terminator()) {
            value = expression();
        }
        consume_statement_terminator();
        return std::make_unique<ReturnStmt>(std::move(value));
    }

    return expression_statement();
}

StmtPtr Parser::block() {
    consume(TokenType::LBRACE, "Expected '{'");

    auto block_stmt = std::make_unique<BlockStmt>();

    skip_newlines();

    while (!check(TokenType::RBRACE) && !is_at_end()) {
        auto stmt = statement();
        if (stmt) {
            block_stmt->statements.push_back(std::move(stmt));
        }
        skip_newlines();
    }

    consume(TokenType::RBRACE, "Expected '}'");

    return block_stmt;
}

StmtPtr Parser::if_statement() {
    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    ExprPtr condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    skip_optional_newlines();
    StmtPtr then_branch = statement();

    StmtPtr else_branch = nullptr;
    skip_optional_newlines();
    if (match(TokenType::ELSE)) {
        skip_optional_newlines();
        else_branch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition),
                                     std::move(then_branch),
                                     std::move(else_branch));
}

StmtPtr Parser::while_statement() {
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    ExprPtr condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    skip_optional_newlines();
    StmtPtr body = statement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

StmtPtr Parser::do_while_statement() {
    skip_optional_newlines();
    StmtPtr body = statement();

    skip_optional_newlines();
    consume(TokenType::WHILE, "Expected 'while' after do body");
    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    ExprPtr condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    consume_statement_terminator();

    return std::make_unique<DoWhileStmt>(std::move(body), std::move(condition));
}

StmtPtr Parser::for_statement() {
    consume(TokenType::LPAREN, "Expected '(' after 'for'");

    // Check if it's a for-in
    // for (var in array)
    if (check(TokenType::IDENTIFIER)) {
        Token var_token = current_;
        advance();

        if (match(TokenType::IN)) {
            // For-in
            Token array_token = consume(TokenType::IDENTIFIER, "Expected array name");
            consume(TokenType::RPAREN, "Expected ')' after for-in");

            skip_optional_newlines();
            StmtPtr body = statement();

            return std::make_unique<ForInStmt>(var_token.lexeme,
                                                array_token.lexeme,
                                                std::move(body));
        } else {
            // Back to normal for
            // We need to treat the just-read identifier as an expression
            // This is somewhat awkward...
            // Simpler: start expression parsing with the identifier

            // We create a variable expression
            auto var_expr = std::make_unique<VariableExpr>(var_token.lexeme);

            // Check if it's an assignment
            StmtPtr init = nullptr;
            if (current_.is_assignment_op()) {
                TokenType op = current_.type;
                advance();
                ExprPtr value = expression();
                auto assign = std::make_unique<AssignExpr>(std::move(var_expr), op, std::move(value));
                init = std::make_unique<ExprStmt>(std::move(assign));
            } else {
                // Could have more operators
                // For simplicity: jumping back and re-parsing is difficult
                // We treat it as expression statement
                init = std::make_unique<ExprStmt>(std::move(var_expr));
            }

            consume(TokenType::SEMICOLON, "Expected ';' after for init");

            // Condition
            ExprPtr condition = nullptr;
            if (!check(TokenType::SEMICOLON)) {
                condition = expression();
            }
            consume(TokenType::SEMICOLON, "Expected ';' after for condition");

            // Update
            ExprPtr update = nullptr;
            if (!check(TokenType::RPAREN)) {
                update = expression();
            }
            consume(TokenType::RPAREN, "Expected ')' after for clauses");

            skip_optional_newlines();
            StmtPtr body = statement();

            return std::make_unique<ForStmt>(std::move(init),
                                              std::move(condition),
                                              std::move(update),
                                              std::move(body));
        }
    }

    // Normal C-style for
    StmtPtr init = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        init = simple_statement();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for init");

    ExprPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for condition");

    ExprPtr update = nullptr;
    if (!check(TokenType::RPAREN)) {
        update = expression();
    }
    consume(TokenType::RPAREN, "Expected ')' after for clauses");

    skip_optional_newlines();
    StmtPtr body = statement();

    return std::make_unique<ForStmt>(std::move(init),
                                      std::move(condition),
                                      std::move(update),
                                      std::move(body));
}

StmtPtr Parser::switch_statement() {
    consume(TokenType::LPAREN, "Expected '(' after 'switch'");
    ExprPtr expr = expression();
    consume(TokenType::RPAREN, "Expected ')' after switch expression");

    skip_optional_newlines();
    consume(TokenType::LBRACE, "Expected '{' before switch cases");
    skip_newlines();

    auto switch_stmt = std::make_unique<SwitchStmt>(std::move(expr));

    while (!check(TokenType::RBRACE) && !is_at_end()) {
        if (match(TokenType::CASE)) {
            ExprPtr case_expr = expression();
            consume(TokenType::COLON, "Expected ':' after case expression");
            skip_newlines();

            // Collect statements until next case/default/}
            std::vector<StmtPtr> case_stmts;
            while (!check(TokenType::CASE) &&
                   !check(TokenType::DEFAULT) &&
                   !check(TokenType::RBRACE) &&
                   !is_at_end()) {
                auto stmt = statement();
                if (stmt) {
                    case_stmts.push_back(std::move(stmt));
                }
                skip_newlines();
            }

            auto case_body = std::make_unique<BlockStmt>(std::move(case_stmts));
            switch_stmt->cases.emplace_back(std::move(case_expr), std::move(case_body));

        } else if (match(TokenType::DEFAULT)) {
            consume(TokenType::COLON, "Expected ':' after 'default'");
            skip_newlines();

            std::vector<StmtPtr> default_stmts;
            while (!check(TokenType::CASE) &&
                   !check(TokenType::RBRACE) &&
                   !is_at_end()) {
                auto stmt = statement();
                if (stmt) {
                    default_stmts.push_back(std::move(stmt));
                }
                skip_newlines();
            }

            switch_stmt->default_case = std::make_unique<BlockStmt>(std::move(default_stmts));
        } else {
            error("Expected 'case' or 'default'");
            break;
        }
    }

    consume(TokenType::RBRACE, "Expected '}' after switch cases");

    return switch_stmt;
}

StmtPtr Parser::print_statement() {
    advance();  // Consume 'print'

    auto print_stmt = std::make_unique<PrintStmt>();

    // Parse arguments (can be separated by comma or space)
    // In AWK "print a b" = print a " " b (concatenation with OFS)
    // and "print a, b" = print with OFS in between

    if (!is_statement_terminator() &&
        !check(TokenType::GT) &&
        !check(TokenType::APPEND) &&
        !check(TokenType::PIPE) &&
        !check(TokenType::PIPE_BOTH)) {

        // First argument
        print_stmt->arguments.push_back(expression());

        // More arguments with comma
        while (match(TokenType::COMMA)) {
            skip_optional_newlines();
            print_stmt->arguments.push_back(expression());
        }
    }

    // Output-Redirect?
    auto [redirect, redirect_type] = parse_output_redirect();
    if (redirect) {
        print_stmt->output_redirect = std::move(redirect);
        print_stmt->redirect_type = redirect_type;
    }

    consume_statement_terminator();

    return print_stmt;
}

StmtPtr Parser::printf_statement() {
    advance();  // Consume 'printf'

    ExprPtr format = expression();
    auto printf_stmt = std::make_unique<PrintfStmt>(std::move(format));

    // Arguments
    while (match(TokenType::COMMA)) {
        skip_optional_newlines();
        printf_stmt->arguments.push_back(expression());
    }

    // Output-Redirect?
    auto [redirect, redirect_type] = parse_output_redirect();
    if (redirect) {
        printf_stmt->output_redirect = std::move(redirect);
        printf_stmt->redirect_type = redirect_type;
    }

    consume_statement_terminator();

    return printf_stmt;
}

StmtPtr Parser::delete_statement() {
    Token name_token = consume(TokenType::IDENTIFIER, "Expected array name after 'delete'");
    std::string name = name_token.lexeme;

    std::vector<ExprPtr> indices;

    if (match(TokenType::LBRACKET)) {
        // Delete specific elements
        if (!check(TokenType::RBRACKET)) {
            indices.push_back(expression());
            while (match(TokenType::COMMA)) {
                indices.push_back(expression());
            }
        }
        consume(TokenType::RBRACKET, "Expected ']' after delete indices");
    }
    // Without [] the entire array is deleted

    consume_statement_terminator();

    return std::make_unique<DeleteStmt>(std::move(name), std::move(indices));
}

StmtPtr Parser::expression_statement() {
    ExprPtr expr = expression();
    consume_statement_terminator();
    return std::make_unique<ExprStmt>(std::move(expr));
}

StmtPtr Parser::simple_statement() {
    ExprPtr expr = expression();
    return std::make_unique<ExprStmt>(std::move(expr));
}

// ============================================================================
// Expressions
// ============================================================================

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = ternary();

    if (current_.is_assignment_op()) {
        TokenType op = current_.type;
        advance();
        skip_optional_newlines();
        ExprPtr value = assignment();  // Right-associative

        // Check if LValue
        if (!is_lvalue(expr.get())) {
            error("Invalid assignment target");
        }

        return std::make_unique<AssignExpr>(std::move(expr), op, std::move(value));
    }

    return expr;
}

ExprPtr Parser::ternary() {
    ExprPtr expr = logical_or();

    if (match(TokenType::QUESTION)) {
        skip_optional_newlines();
        ExprPtr then_expr = expression();
        consume(TokenType::COLON, "Expected ':' in ternary expression");
        skip_optional_newlines();
        ExprPtr else_expr = ternary();

        return std::make_unique<TernaryExpr>(std::move(expr),
                                              std::move(then_expr),
                                              std::move(else_expr));
    }

    return expr;
}

ExprPtr Parser::logical_or() {
    ExprPtr expr = logical_and();

    while (match(TokenType::OR)) {
        skip_optional_newlines();
        ExprPtr right = logical_and();
        expr = std::make_unique<BinaryExpr>(std::move(expr), TokenType::OR, std::move(right));
    }

    return expr;
}

ExprPtr Parser::logical_and() {
    ExprPtr expr = array_membership();

    while (match(TokenType::AND)) {
        skip_optional_newlines();
        ExprPtr right = array_membership();
        expr = std::make_unique<BinaryExpr>(std::move(expr), TokenType::AND, std::move(right));
    }

    return expr;
}

ExprPtr Parser::array_membership() {
    ExprPtr expr = regex_match();

    // (key) in array oder (k1, k2) in array
    if (match(TokenType::IN)) {
        std::vector<ExprPtr> keys;
        keys.push_back(std::move(expr));

        Token array_token = consume(TokenType::IDENTIFIER, "Expected array name after 'in'");

        return std::make_unique<InExpr>(std::move(keys), array_token.lexeme);
    }

    return expr;
}

ExprPtr Parser::regex_match() {
    ExprPtr expr = comparison();

    while (match({TokenType::MATCH, TokenType::NOT_MATCH})) {
        TokenType op = previous_.type;
        bool negated = (op == TokenType::NOT_MATCH);

        skip_optional_newlines();

        // Regex can be a literal or an expression
        ExprPtr regex;
        if (check(TokenType::REGEX)) {
            // Regex literal was recognized (lexer knows to expect regex after ~)
            regex = std::make_unique<RegexExpr>(std::get<std::string>(current_.literal));
            advance();
        } else {
            // Dynamic regex (variable or expression)
            regex = comparison();
        }

        expr = std::make_unique<MatchExpr>(std::move(expr), std::move(regex), negated);
    }

    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = pipe_getline();

    while (match({TokenType::EQ, TokenType::NE, TokenType::LT,
                  TokenType::GT, TokenType::LE, TokenType::GE})) {
        TokenType op = previous_.type;
        skip_optional_newlines();
        ExprPtr right = concatenation();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::pipe_getline() {
    ExprPtr expr = concatenation();

    // Check for: cmd | getline [var] or cmd |& getline [var]
    // Only match if followed by GETLINE, otherwise leave the token for print redirect
    if ((check(TokenType::PIPE) || check(TokenType::PIPE_BOTH)) && peek_next_is(TokenType::GETLINE)) {
        bool coprocess = check(TokenType::PIPE_BOTH);
        advance();  // Consume | or |&
        advance();  // Consume getline

        auto getline_expr = std::make_unique<GetlineExpr>();
        getline_expr->command = std::move(expr);
        getline_expr->coprocess = coprocess;

        // Optional: Variable
        if (check(TokenType::IDENTIFIER)) {
            Token var_token = current_;
            advance();
            getline_expr->variable = std::make_unique<VariableExpr>(var_token.lexeme);
        }

        return getline_expr;
    }

    return expr;
}

ExprPtr Parser::concatenation() {
    ExprPtr expr = addition();

    // Concatenation by juxtaposition
    // This is tricky - we check if the next token could start an expression
    std::vector<ExprPtr> parts;
    parts.push_back(std::move(expr));

    while (true) {
        // Check if the next token could be an expression
        // NOT for operators, statement terminators, etc.
        if (check(TokenType::STRING) ||
            check(TokenType::NUMBER) ||
            check(TokenType::IDENTIFIER) ||
            check(TokenType::DOLLAR) ||
            check(TokenType::LPAREN) ||
            check(TokenType::NOT) ||
            check(TokenType::MINUS) ||
            check(TokenType::PLUS) ||
            check(TokenType::INCREMENT) ||
            check(TokenType::DECREMENT)) {

            // But not if an operator or terminator follows
            // after the current expression
            parts.push_back(addition());
        } else {
            break;
        }
    }

    if (parts.size() == 1) {
        return std::move(parts[0]);
    }

    return std::make_unique<ConcatExpr>(std::move(parts));
}

ExprPtr Parser::addition() {
    ExprPtr expr = multiplication();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        TokenType op = previous_.type;
        skip_optional_newlines();
        ExprPtr right = multiplication();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::multiplication() {
    ExprPtr expr = power();

    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        TokenType op = previous_.type;
        skip_optional_newlines();
        ExprPtr right = power();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::power() {
    ExprPtr expr = unary();

    // Right-associative
    if (match(TokenType::CARET)) {
        skip_optional_newlines();
        ExprPtr right = power();
        expr = std::make_unique<BinaryExpr>(std::move(expr), TokenType::CARET, std::move(right));
    }

    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::NOT, TokenType::MINUS, TokenType::PLUS})) {
        TokenType op = previous_.type;
        ExprPtr operand = unary();
        return std::make_unique<UnaryExpr>(op, std::move(operand), true);
    }

    if (match({TokenType::INCREMENT, TokenType::DECREMENT})) {
        TokenType op = previous_.type;
        ExprPtr operand = unary();
        return std::make_unique<UnaryExpr>(op, std::move(operand), true);
    }

    return postfix();
}

ExprPtr Parser::postfix() {
    ExprPtr expr = primary();

    while (match({TokenType::INCREMENT, TokenType::DECREMENT})) {
        TokenType op = previous_.type;
        expr = std::make_unique<UnaryExpr>(op, std::move(expr), false);
    }

    return expr;
}

ExprPtr Parser::primary() {
    // Numbers
    if (match(TokenType::NUMBER)) {
        return std::make_unique<LiteralExpr>(std::get<double>(previous_.literal));
    }

    // Strings
    if (match(TokenType::STRING)) {
        return std::make_unique<LiteralExpr>(std::get<std::string>(previous_.literal));
    }

    // Regex as literal
    if (check(TokenType::REGEX)) {
        auto regex = std::make_unique<RegexExpr>(std::get<std::string>(current_.literal));
        advance();
        return regex;
    }

    // Field access ($0, $1, $(expr))
    if (match(TokenType::DOLLAR)) {
        ExprPtr index = unary();  // $ binds strongly
        return std::make_unique<FieldExpr>(std::move(index));
    }

    // Getline
    if (check(TokenType::GETLINE)) {
        return getline_expression();
    }

    // Indirect function call (gawk extension: @varname(args))
    if (match(TokenType::AT)) {
        // After @ must follow an identifier or a parenthesized expression
        ExprPtr func_name_expr;

        if (match(TokenType::IDENTIFIER)) {
            // @varname - variable contains the function name
            func_name_expr = std::make_unique<VariableExpr>(previous_.lexeme);
        } else if (match(TokenType::LPAREN)) {
            // @(expression) - expression yields the function name
            func_name_expr = expression();
            consume(TokenType::RPAREN, "Expected ')' after indirect function expression");
        } else {
            error("Expected identifier or '(' after '@' for indirect function call");
            return std::make_unique<LiteralExpr>(0.0);
        }

        // Now an argument list must follow
        if (!check(TokenType::LPAREN)) {
            error("Expected '(' after indirect function name");
            return std::make_unique<LiteralExpr>(0.0);
        }

        consume(TokenType::LPAREN, "Expected '(' for indirect function call");
        std::vector<ExprPtr> args;

        if (!check(TokenType::RPAREN)) {
            do {
                skip_optional_newlines();
                args.push_back(expression());
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RPAREN, "Expected ')' after arguments");

        return std::make_unique<IndirectCallExpr>(std::move(func_name_expr), std::move(args));
    }

    // Identifier (variable, function, array)
    if (match(TokenType::IDENTIFIER)) {
        std::string name = previous_.lexeme;

        // Qualified name with namespace::name? (gawk extension)
        if (match(TokenType::COLON_COLON)) {
            Token qual_name = consume(TokenType::IDENTIFIER, "Expected identifier after '::'");
            name = name + "::" + qual_name.lexeme;
        } else {
            // Automatically qualify with current namespace (gawk extension)
            name = qualify_name(name);
        }

        // Function call?
        if (check(TokenType::LPAREN)) {
            return finish_call(name);
        }

        // Array access?
        if (check(TokenType::LBRACKET)) {
            return finish_array_access(name);
        }

        // Normal variable
        return std::make_unique<VariableExpr>(name);
    }

    // Grouped expression
    if (match(TokenType::LPAREN)) {
        skip_optional_newlines();

        // Could also be (k1, k2) in array
        ExprPtr expr = expression();

        // Check for comma for multi-key in-expression
        if (match(TokenType::COMMA)) {
            std::vector<ExprPtr> keys;
            keys.push_back(std::move(expr));

            do {
                skip_optional_newlines();
                keys.push_back(expression());
            } while (match(TokenType::COMMA));

            consume(TokenType::RPAREN, "Expected ')' after expression list");

            // Must be followed by 'in'
            if (match(TokenType::IN)) {
                Token array_token = consume(TokenType::IDENTIFIER, "Expected array name");
                return std::make_unique<InExpr>(std::move(keys), array_token.lexeme);
            } else {
                error("Expected 'in' after key list");
                return keys[0] ? std::move(keys[0]) : std::make_unique<LiteralExpr>(0.0);
            }
        }

        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    error("Expected expression, found '" + current_.lexeme + "'");
    return std::make_unique<LiteralExpr>(0.0);  // Fallback
}

ExprPtr Parser::getline_expression() {
    advance();  // Consume 'getline'

    auto getline_expr = std::make_unique<GetlineExpr>();

    // Optional: Variable
    if (check(TokenType::IDENTIFIER) && !check(TokenType::LT)) {
        // Check if this is really a variable or the next expression
        Token next = current_;
        advance();

        if (check(TokenType::LT) || is_statement_terminator() ||
            check(TokenType::PIPE) || check(TokenType::PIPE_BOTH)) {
            getline_expr->variable = std::make_unique<VariableExpr>(next.lexeme);
        } else {
            // No variable argument, put token back (simulated)
            // This is tricky without lookahead...
            // For now: we assume it is a variable
            getline_expr->variable = std::make_unique<VariableExpr>(next.lexeme);
        }
    }

    // Optional: < file
    if (match(TokenType::LT)) {
        getline_expr->file = expression();
    }

    return getline_expr;
}

ExprPtr Parser::finish_call(const std::string& name) {
    consume(TokenType::LPAREN, "Expected '(' after function name");

    std::vector<ExprPtr> args;

    if (!check(TokenType::RPAREN)) {
        do {
            skip_optional_newlines();
            args.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expected ')' after arguments");

    return std::make_unique<CallExpr>(name, std::move(args));
}

ExprPtr Parser::finish_array_access(const std::string& name) {
    consume(TokenType::LBRACKET, "Expected '[' for array access");

    std::vector<ExprPtr> indices;

    if (!check(TokenType::RBRACKET)) {
        indices.push_back(expression());
        while (match(TokenType::COMMA)) {
            indices.push_back(expression());
        }
    }

    consume(TokenType::RBRACKET, "Expected ']' after array index");

    return std::make_unique<ArrayAccessExpr>(name, std::move(indices));
}

std::pair<ExprPtr, RedirectType> Parser::parse_output_redirect() {
    if (match(TokenType::GT)) {
        ExprPtr target = expression();
        return {std::move(target), RedirectType::WRITE};
    }

    if (match(TokenType::APPEND)) {
        ExprPtr target = expression();
        return {std::move(target), RedirectType::APPEND};
    }

    // |& must be checked before | (gawk coprocess extension)
    if (match(TokenType::PIPE_BOTH)) {
        ExprPtr target = expression();
        return {std::move(target), RedirectType::PIPE_BOTH};
    }

    if (match(TokenType::PIPE)) {
        ExprPtr target = expression();
        return {std::move(target), RedirectType::PIPE};
    }

    return {nullptr, RedirectType::NONE};
}

bool Parser::is_lvalue(const Expr* expr) const {
    return dynamic_cast<const VariableExpr*>(expr) != nullptr ||
           dynamic_cast<const FieldExpr*>(expr) != nullptr ||
           dynamic_cast<const ArrayAccessExpr*>(expr) != nullptr;
}

} // namespace awk
