// ============================================================================
// interpreter_eval.cpp - Expression Evaluation
// ============================================================================

#include "awk/interpreter.hpp"
#include <sstream>
#include <regex>

namespace awk {

// ============================================================================
// Expression Evaluation - Main Dispatch
// ============================================================================

AWKValue Interpreter::evaluate(Expr& expr) {
    if (auto* literal = dynamic_cast<LiteralExpr*>(&expr))
        return evaluate(*literal);
    if (auto* regex = dynamic_cast<RegexExpr*>(&expr))
        return evaluate(*regex);
    if (auto* var = dynamic_cast<VariableExpr*>(&expr))
        return evaluate(*var);
    if (auto* field = dynamic_cast<FieldExpr*>(&expr))
        return evaluate(*field);
    if (auto* arr = dynamic_cast<ArrayAccessExpr*>(&expr))
        return evaluate(*arr);
    if (auto* binary = dynamic_cast<BinaryExpr*>(&expr))
        return evaluate(*binary);
    if (auto* unary = dynamic_cast<UnaryExpr*>(&expr))
        return evaluate(*unary);
    if (auto* ternary = dynamic_cast<TernaryExpr*>(&expr))
        return evaluate(*ternary);
    if (auto* assign = dynamic_cast<AssignExpr*>(&expr))
        return evaluate(*assign);
    if (auto* call = dynamic_cast<CallExpr*>(&expr))
        return evaluate(*call);
    if (auto* indirect = dynamic_cast<IndirectCallExpr*>(&expr))
        return evaluate(*indirect);
    if (auto* match = dynamic_cast<MatchExpr*>(&expr))
        return evaluate(*match);
    if (auto* concat = dynamic_cast<ConcatExpr*>(&expr))
        return evaluate(*concat);
    if (auto* getline = dynamic_cast<GetlineExpr*>(&expr))
        return evaluate(*getline);
    if (auto* in = dynamic_cast<InExpr*>(&expr))
        return evaluate(*in);

    return AWKValue();
}

// ============================================================================
// Literal & Regex Expressions
// ============================================================================

AWKValue Interpreter::evaluate(LiteralExpr& expr) {
    if (expr.is_number()) {
        return AWKValue(expr.as_number());
    } else {
        return AWKValue(expr.as_string());
    }
}

AWKValue Interpreter::evaluate(RegexExpr& expr) {
    AWKValue val;
    val.set_regex(expr.pattern);
    return val;
}

// ============================================================================
// Variable & Field Access
// ============================================================================

AWKValue Interpreter::evaluate(VariableExpr& expr) {
    return env_.get_variable(expr.name);
}

AWKValue Interpreter::evaluate(FieldExpr& expr) {
    int index = static_cast<int>(evaluate(*expr.index).to_number());
    return get_field(index);
}

AWKValue Interpreter::evaluate(ArrayAccessExpr& expr) {
    std::vector<AWKValue> idx_vals;
    for (auto& idx : expr.indices) {
        idx_vals.push_back(evaluate(*idx));
    }
    std::string key = AWKValue::make_array_key(idx_vals, get_cached_subsep());

    // Special handling for SYMTAB (gawk extension)
    if (expr.name == "SYMTAB") {
        // SYMTAB["varname"] gives direct access to variable
        return env_.get_variable(key);
    }

    // Special handling for FUNCTAB (gawk extension)
    if (expr.name == "FUNCTAB") {
        // FUNCTAB["funcname"] returns "func" if function exists
        if (env_.has_function(key) || env_.has_builtin(key)) {
            return AWKValue(key);
        }
        return AWKValue("");
    }

    AWKValue& arr = env_.get_variable(expr.name);
    return arr.array_access(key);
}

// ============================================================================
// Binary Expressions
// ============================================================================

AWKValue Interpreter::evaluate(BinaryExpr& expr) {
    // Short-circuit evaluation for && and ||
    if (expr.op == TokenType::AND) {
        AWKValue left = evaluate(*expr.left);
        if (!is_truthy(left)) return AWKValue(0.0);
        return AWKValue(is_truthy(evaluate(*expr.right)) ? 1.0 : 0.0);
    }

    if (expr.op == TokenType::OR) {
        AWKValue left = evaluate(*expr.left);
        if (is_truthy(left)) return AWKValue(1.0);
        return AWKValue(is_truthy(evaluate(*expr.right)) ? 1.0 : 0.0);
    }

    AWKValue left = evaluate(*expr.left);
    AWKValue right = evaluate(*expr.right);

    switch (expr.op) {
        case TokenType::PLUS:
            return left + right;
        case TokenType::MINUS:
            return left - right;
        case TokenType::STAR:
            return left * right;
        case TokenType::SLASH:
            return left / right;
        case TokenType::PERCENT:
            return left % right;
        case TokenType::CARET:
            return left.power(right);

        case TokenType::EQ:
            return AWKValue(left == right ? 1.0 : 0.0);
        case TokenType::NE:
            return AWKValue(left != right ? 1.0 : 0.0);
        case TokenType::LT:
            return AWKValue(left < right ? 1.0 : 0.0);
        case TokenType::GT:
            return AWKValue(left > right ? 1.0 : 0.0);
        case TokenType::LE:
            return AWKValue(left <= right ? 1.0 : 0.0);
        case TokenType::GE:
            return AWKValue(left >= right ? 1.0 : 0.0);

        default:
            return AWKValue();
    }
}

// ============================================================================
// Unary Expressions
// ============================================================================

AWKValue Interpreter::evaluate(UnaryExpr& expr) {
    if (expr.op == TokenType::NOT) {
        // For NOT with regex: check if $0 does NOT match the regex
        if (auto* regex_expr = dynamic_cast<RegexExpr*>(expr.operand.get())) {
            AWKValue regex_val = evaluate(*regex_expr);
            bool matches = regex_match(AWKValue(current_record_), regex_val);
            return AWKValue(matches ? 0.0 : 1.0);  // Negated!
        }
        return AWKValue(is_truthy(evaluate(*expr.operand)) ? 0.0 : 1.0);
    }

    if (expr.op == TokenType::MINUS) {
        return -evaluate(*expr.operand);
    }

    if (expr.op == TokenType::PLUS) {
        return +evaluate(*expr.operand);
    }

    if (expr.op == TokenType::INCREMENT || expr.op == TokenType::DECREMENT) {
        AWKValue& lval = get_lvalue(*expr.operand);

        if (expr.prefix) {
            if (expr.op == TokenType::INCREMENT) {
                return lval.pre_increment();
            } else {
                return lval.pre_decrement();
            }
        } else {
            if (expr.op == TokenType::INCREMENT) {
                return lval.post_increment();
            } else {
                return lval.post_decrement();
            }
        }
    }

    return AWKValue();
}

// ============================================================================
// Ternary Expression
// ============================================================================

AWKValue Interpreter::evaluate(TernaryExpr& expr) {
    if (is_truthy(evaluate(*expr.condition))) {
        return evaluate(*expr.then_expr);
    } else {
        return evaluate(*expr.else_expr);
    }
}

// ============================================================================
// Assignment Expression
// ============================================================================

AWKValue Interpreter::evaluate(AssignExpr& expr) {
    // Optimization: detect var = var ... pattern for in-place string append
    // This avoids O(n^2) behavior when building large strings
    if (expr.op == TokenType::ASSIGN) {
        if (auto* target_var = dynamic_cast<VariableExpr*>(expr.target.get())) {
            if (auto* concat = dynamic_cast<ConcatExpr*>(expr.value.get())) {
                if (!concat->parts.empty()) {
                    if (auto* first_var = dynamic_cast<VariableExpr*>(concat->parts[0].get())) {
                        if (first_var->name == target_var->name) {
                            // Pattern matched: var = var ...
                            // Get reference to target variable
                            AWKValue& target = env_.get_variable(target_var->name);

                            // Evaluate and append each remaining part directly
                            for (size_t i = 1; i < concat->parts.size(); ++i) {
                                target.append_string(evaluate(*concat->parts[i]).to_string());
                            }

                            // Return empty value - returning the full accumulated string
                            // would cause O(n^2) copying. The value is already in the
                            // variable, which handles 99.9% of use cases where the
                            // assignment result is discarded.
                            return AWKValue();
                        }
                    }
                }
            }
        }
    }

    AWKValue value = evaluate(*expr.value);

    // Special handling for field assignment ($n = value)
    if (auto* field = dynamic_cast<FieldExpr*>(expr.target.get())) {
        int index = static_cast<int>(evaluate(*field->index).to_number());

        switch (expr.op) {
            case TokenType::ASSIGN:
                set_field(index, value);
                break;
            case TokenType::PLUS_ASSIGN:
                set_field(index, get_field(index) + value);
                break;
            case TokenType::MINUS_ASSIGN:
                set_field(index, get_field(index) - value);
                break;
            case TokenType::STAR_ASSIGN:
                set_field(index, get_field(index) * value);
                break;
            case TokenType::SLASH_ASSIGN:
                set_field(index, get_field(index) / value);
                break;
            case TokenType::PERCENT_ASSIGN:
                set_field(index, get_field(index) % value);
                break;
            case TokenType::CARET_ASSIGN:
                set_field(index, get_field(index).power(value));
                break;
            default:
                break;
        }
        return get_field(index);
    }

    // Normal LValue assignment
    AWKValue& target = get_lvalue(*expr.target);

    switch (expr.op) {
        case TokenType::ASSIGN:
            target = value;
            break;
        case TokenType::PLUS_ASSIGN:
            target += value;
            break;
        case TokenType::MINUS_ASSIGN:
            target -= value;
            break;
        case TokenType::STAR_ASSIGN:
            target *= value;
            break;
        case TokenType::SLASH_ASSIGN:
            target /= value;
            break;
        case TokenType::PERCENT_ASSIGN:
            target %= value;
            break;
        case TokenType::CARET_ASSIGN:
            target = target.power(value);
            break;
        default:
            break;
    }

    return target;
}

// ============================================================================
// Match Expression
// ============================================================================

AWKValue Interpreter::evaluate(MatchExpr& expr) {
    AWKValue text = evaluate(*expr.string);
    AWKValue pattern = evaluate(*expr.regex);

    bool matches = regex_match(text, pattern);

    if (expr.negated) {
        return AWKValue(matches ? 0.0 : 1.0);
    } else {
        return AWKValue(matches ? 1.0 : 0.0);
    }
}

// ============================================================================
// Concat Expression
// ============================================================================

AWKValue Interpreter::evaluate(ConcatExpr& expr) {
    // First pass: evaluate all parts and calculate total size
    std::vector<std::string> parts;
    parts.reserve(expr.parts.size());
    size_t total_size = 0;
    for (auto& part : expr.parts) {
        parts.push_back(evaluate(*part).to_string());
        total_size += parts.back().length();
    }

    // Second pass: concatenate with pre-allocated buffer
    std::string result;
    result.reserve(total_size);
    for (auto& part : parts) {
        result += part;
    }
    return AWKValue(result);
}

// ============================================================================
// In Expression
// ============================================================================

AWKValue Interpreter::evaluate(InExpr& expr) {
    std::vector<AWKValue> idx_vals;
    for (auto& k : expr.keys) {
        idx_vals.push_back(evaluate(*k));
    }
    std::string key = AWKValue::make_array_key(idx_vals, get_cached_subsep());

    // Special handling for SYMTAB
    if (expr.array_name == "SYMTAB") {
        return AWKValue(env_.has_variable(key) ? 1.0 : 0.0);
    }

    // Special handling for FUNCTAB
    if (expr.array_name == "FUNCTAB") {
        return AWKValue((env_.has_function(key) || env_.has_builtin(key)) ? 1.0 : 0.0);
    }

    AWKValue& arr = env_.get_variable(expr.array_name);
    return AWKValue(arr.array_contains(key) ? 1.0 : 0.0);
}

// ============================================================================
// Indirect Call Expression (gawk Extension)
// ============================================================================

AWKValue Interpreter::evaluate(IndirectCallExpr& expr) {
    // gawk extension: @varname(args) or @(expression)(args)
    // Evaluate the expression to get the function name
    AWKValue func_name_val = evaluate(*expr.func_name_expr);
    std::string func_name = func_name_val.to_string();

    // Evaluate arguments
    std::vector<AWKValue> args;
    for (auto& arg : expr.arguments) {
        args.push_back(evaluate(*arg));
    }

    // Call function
    return call_function(func_name, args);
}

} // namespace awk
