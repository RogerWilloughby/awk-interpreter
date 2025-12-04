// ============================================================================
// interpreter_exec.cpp - Statement Execution
// ============================================================================

#include "awk/interpreter.hpp"
#include <sstream>

namespace awk {

// ============================================================================
// Statement Execution - Main Dispatch
// ============================================================================

void Interpreter::execute(Stmt& stmt) {
    // Dispatch based on type
    if (auto* block = dynamic_cast<BlockStmt*>(&stmt)) {
        execute(*block);
    } else if (auto* ifstmt = dynamic_cast<IfStmt*>(&stmt)) {
        execute(*ifstmt);
    } else if (auto* whilestmt = dynamic_cast<WhileStmt*>(&stmt)) {
        execute(*whilestmt);
    } else if (auto* dowhile = dynamic_cast<DoWhileStmt*>(&stmt)) {
        execute(*dowhile);
    } else if (auto* forstmt = dynamic_cast<ForStmt*>(&stmt)) {
        execute(*forstmt);
    } else if (auto* forin = dynamic_cast<ForInStmt*>(&stmt)) {
        execute(*forin);
    } else if (auto* switchstmt = dynamic_cast<SwitchStmt*>(&stmt)) {
        execute(*switchstmt);
    } else if (auto* print = dynamic_cast<PrintStmt*>(&stmt)) {
        execute(*print);
    } else if (auto* printf = dynamic_cast<PrintfStmt*>(&stmt)) {
        execute(*printf);
    } else if (auto* exprstmt = dynamic_cast<ExprStmt*>(&stmt)) {
        execute(*exprstmt);
    } else if (auto* del = dynamic_cast<DeleteStmt*>(&stmt)) {
        execute(*del);
    } else if (dynamic_cast<BreakStmt*>(&stmt)) {
        throw BreakException();
    } else if (dynamic_cast<ContinueStmt*>(&stmt)) {
        throw ContinueException();
    } else if (dynamic_cast<NextStmt*>(&stmt)) {
        throw NextException();
    } else if (dynamic_cast<NextfileStmt*>(&stmt)) {
        throw NextfileException();
    } else if (auto* exitstmt = dynamic_cast<ExitStmt*>(&stmt)) {
        int status = 0;
        if (exitstmt->status) {
            status = static_cast<int>(evaluate(*exitstmt->status).to_number());
        }
        throw ExitException(status);
    } else if (auto* ret = dynamic_cast<ReturnStmt*>(&stmt)) {
        AWKValue val;
        if (ret->value) {
            val = evaluate(*ret->value);
        }
        throw ReturnException(std::move(val));
    }
}

// ============================================================================
// Block Statement
// ============================================================================

void Interpreter::execute(BlockStmt& stmt) {
    for (auto& s : stmt.statements) {
        execute(*s);
    }
}

// ============================================================================
// Control Flow Statements
// ============================================================================

void Interpreter::execute(IfStmt& stmt) {
    if (is_truthy(evaluate(*stmt.condition))) {
        execute(*stmt.then_branch);
    } else if (stmt.else_branch) {
        execute(*stmt.else_branch);
    }
}

void Interpreter::execute(WhileStmt& stmt) {
    while (is_truthy(evaluate(*stmt.condition))) {
        try {
            execute(*stmt.body);
        } catch (const BreakException&) {
            break;
        } catch (const ContinueException&) {
            continue;
        }
    }
}

void Interpreter::execute(DoWhileStmt& stmt) {
    do {
        try {
            execute(*stmt.body);
        } catch (const BreakException&) {
            break;
        } catch (const ContinueException&) {
            continue;
        }
    } while (is_truthy(evaluate(*stmt.condition)));
}

void Interpreter::execute(ForStmt& stmt) {
    if (stmt.init) {
        execute(*stmt.init);
    }

    while (!stmt.condition || is_truthy(evaluate(*stmt.condition))) {
        try {
            execute(*stmt.body);
        } catch (const BreakException&) {
            break;
        } catch (const ContinueException&) {
            // Continue: execute update and continue
        }

        if (stmt.update) {
            evaluate(*stmt.update);
        }
    }
}

void Interpreter::execute(ForInStmt& stmt) {
    std::vector<std::string> keys;

    // Special handling for SYMTAB
    if (stmt.array_name == "SYMTAB") {
        keys = env_.get_all_variable_names();
    }
    // Special handling for FUNCTAB
    else if (stmt.array_name == "FUNCTAB") {
        keys = env_.get_all_function_names();
    }
    else {
        AWKValue& arr = env_.get_variable(stmt.array_name);
        if (!arr.is_array()) {
            return;  // Not an array, nothing to iterate
        }
        keys = arr.array_keys();
    }

    for (const auto& key : keys) {
        env_.set_variable(stmt.variable, AWKValue(key));

        try {
            execute(*stmt.body);
        } catch (const BreakException&) {
            break;
        } catch (const ContinueException&) {
            continue;
        }
    }
}

void Interpreter::execute(SwitchStmt& stmt) {
    AWKValue switch_val = evaluate(*stmt.expression);
    bool matched = false;

    for (auto& [case_expr, case_body] : stmt.cases) {
        if (!matched) {
            AWKValue case_val = evaluate(*case_expr);
            if (switch_val == case_val) {
                matched = true;
            }
        }

        if (matched) {
            try {
                execute(*case_body);
            } catch (const BreakException&) {
                return;  // break exits switch
            }
        }
    }

    if (!matched && stmt.default_case) {
        try {
            execute(*stmt.default_case);
        } catch (const BreakException&) {
            return;
        }
    }
}

// ============================================================================
// Print Statements
// ============================================================================

void Interpreter::execute(PrintStmt& stmt) {
    std::ostream* out = output_;

    if (stmt.output_redirect) {
        std::string target = evaluate(*stmt.output_redirect).to_string();
        out = get_output_stream(target, stmt.redirect_type);
    }

    if (stmt.arguments.empty()) {
        // print without arguments: print $0
        rebuild_record();  // Important: rebuild record from modified fields
        *out << current_record_;
    } else {
        // Cache OFS and OFMT for the loop to avoid repeated lookups
        const std::string& ofs = get_cached_ofs();
        const std::string& ofmt = get_cached_ofmt();

        bool first = true;
        for (auto& arg : stmt.arguments) {
            if (!first) {
                *out << ofs;
            }
            first = false;

            AWKValue val = evaluate(*arg);
            *out << val.to_string(ofmt);
        }
    }

    *out << get_cached_ors();
}

void Interpreter::execute(PrintfStmt& stmt) {
    std::ostream* out = output_;

    if (stmt.output_redirect) {
        std::string target = evaluate(*stmt.output_redirect).to_string();
        out = get_output_stream(target, stmt.redirect_type);
    }

    std::string format = evaluate(*stmt.format).to_string();
    std::vector<AWKValue> args;

    for (auto& arg : stmt.arguments) {
        args.push_back(evaluate(*arg));
    }

    *out << do_sprintf(format, args);
}

// ============================================================================
// Expression Statement
// ============================================================================

void Interpreter::execute(ExprStmt& stmt) {
    evaluate(*stmt.expression);
}

// ============================================================================
// Delete Statement
// ============================================================================

void Interpreter::execute(DeleteStmt& stmt) {
    AWKValue& arr = env_.get_variable(stmt.array_name);

    if (stmt.indices.empty()) {
        // Delete entire array
        arr.array_clear();
    } else {
        // Delete specific element
        std::vector<AWKValue> idx_vals;
        for (auto& idx : stmt.indices) {
            idx_vals.push_back(evaluate(*idx));
        }
        std::string key = AWKValue::make_array_key(idx_vals, get_cached_subsep());
        arr.array_delete(key);
    }
}

} // namespace awk
