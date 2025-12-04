#include "awk/environment.hpp"
#include <cstdlib>
#include <unordered_set>

#ifdef _WIN32
// On Windows, use _environ from stdlib.h
#define environ _environ
#else
// On POSIX systems, environ is declared in unistd.h or available globally
extern char** environ;
#endif

namespace awk {

Environment::Environment() {
    init_builtins();
}

// ============================================================================
// Variable Management
// ============================================================================

AWKValue& Environment::get_variable(const std::string& name) {
    // First search in local scopes (from inner to outer)
    for (auto it = scope_stack_.rbegin(); it != scope_stack_.rend(); ++it) {
        auto var_it = it->find(name);
        if (var_it != it->end()) {
            return var_it->second;
        }
    }

    // Check if name is namespace-qualified (contains ::)
    size_t sep_pos = name.find("::");
    if (sep_pos != std::string::npos) {
        std::string unqualified = name.substr(sep_pos + 2);

        // Also check local scopes with unqualified name
        // (function parameters are stored without namespace prefix)
        for (auto it = scope_stack_.rbegin(); it != scope_stack_.rend(); ++it) {
            auto var_it = it->find(unqualified);
            if (var_it != it->end()) {
                return var_it->second;
            }
        }

        // Check if unqualified name is a special built-in variable
        auto it = globals_.find(unqualified);
        if (it != globals_.end()) {
            // Only fall back for special AWK variables (not user-defined)
            if (is_special_variable(unqualified)) {
                return it->second;
            }
        }
    }

    // Search in global variables (creates if needed)
    return globals_[name];
}

void Environment::set_variable(const std::string& name, AWKValue value) {
    // If in a scope, check if variable exists there
    for (auto it = scope_stack_.rbegin(); it != scope_stack_.rend(); ++it) {
        auto var_it = it->find(name);
        if (var_it != it->end()) {
            var_it->second = std::move(value);
            return;
        }
    }

    // Otherwise set globally
    globals_[name] = std::move(value);
}

bool Environment::has_variable(const std::string& name) const {
    // Search in scopes
    for (auto it = scope_stack_.rbegin(); it != scope_stack_.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }

    // Search in global variables
    return globals_.find(name) != globals_.end();
}

void Environment::delete_variable(const std::string& name) {
    // Only delete from global variables
    globals_.erase(name);
}

// ============================================================================
// Scope Management
// ============================================================================

void Environment::push_scope() {
    scope_stack_.emplace_back();
}

void Environment::pop_scope() {
    if (!scope_stack_.empty()) {
        scope_stack_.pop_back();
    }
}

AWKValue& Environment::get_local(const std::string& name) {
    if (scope_stack_.empty()) {
        return globals_[name];
    }
    return scope_stack_.back()[name];
}

void Environment::set_local(const std::string& name, AWKValue value) {
    if (scope_stack_.empty()) {
        globals_[name] = std::move(value);
    } else {
        scope_stack_.back()[name] = std::move(value);
    }
}

// ============================================================================
// Function Registry
// ============================================================================

void Environment::register_function(const std::string& name, FunctionDef* func) {
    user_functions_[name] = func;
}

FunctionDef* Environment::get_function(const std::string& name) {
    auto it = user_functions_.find(name);
    if (it != user_functions_.end()) {
        return it->second;
    }
    return nullptr;
}

bool Environment::has_function(const std::string& name) const {
    return user_functions_.find(name) != user_functions_.end();
}

std::vector<std::string> Environment::get_all_variable_names() const {
    std::vector<std::string> names;
    names.reserve(globals_.size());
    for (const auto& [name, value] : globals_) {
        names.push_back(name);
    }
    return names;
}

std::vector<std::string> Environment::get_all_function_names() const {
    std::vector<std::string> names;
    names.reserve(user_functions_.size() + builtin_functions_.size());
    for (const auto& [name, func] : user_functions_) {
        names.push_back(name);
    }
    for (const auto& [name, func] : builtin_functions_) {
        names.push_back(name);
    }
    return names;
}

void Environment::register_builtin(const std::string& name, BuiltinFunction func) {
    builtin_functions_[name] = std::move(func);
}

BuiltinFunction Environment::get_builtin(const std::string& name) {
    auto it = builtin_functions_.find(name);
    if (it != builtin_functions_.end()) {
        return it->second;
    }
    return nullptr;
}

bool Environment::has_builtin(const std::string& name) const {
    return builtin_functions_.find(name) != builtin_functions_.end();
}

// ============================================================================
// Initialization
// ============================================================================

void Environment::init_builtins() {
    // Field/Record Separators
    globals_["FS"] = AWKValue(" ");
    globals_["RS"] = AWKValue("\n");
    globals_["OFS"] = AWKValue(" ");
    globals_["ORS"] = AWKValue("\n");

    // Counters
    globals_["NR"] = AWKValue(0);
    globals_["NF"] = AWKValue(0);
    globals_["FNR"] = AWKValue(0);

    // Filename
    globals_["FILENAME"] = AWKValue("");

    // Array subscript separator (SUBSEP = 034 = 0x1C = FS)
    globals_["SUBSEP"] = AWKValue(std::string(1, '\034'));

    // Format strings
    globals_["CONVFMT"] = AWKValue("%.6g");
    globals_["OFMT"] = AWKValue("%.6g");

    // Match results
    globals_["RSTART"] = AWKValue(0);
    globals_["RLENGTH"] = AWKValue(0);

    // Case-insensitive matching (gawk extension)
    globals_["IGNORECASE"] = AWKValue(0);

    // Record terminator (gawk extension) - the actual separator found
    globals_["RT"] = AWKValue("");

    // Field pattern (gawk extension) - alternative to FS (empty = disabled)
    globals_["FPAT"] = AWKValue("");

    // Text domain for i18n (gawk extension) - default is "messages"
    globals_["TEXTDOMAIN"] = AWKValue("messages");

    // Arguments
    globals_["ARGC"] = AWKValue(0);
    // ARGV is created as array when needed

    // Load environment variables
    load_environ();
}

void Environment::load_environ() {
    AWKValue& env_array = globals_["ENVIRON"];

    if (environ != nullptr) {
        for (char** env = environ; *env != nullptr; ++env) {
            std::string entry(*env);
            size_t pos = entry.find('=');
            if (pos != std::string::npos) {
                std::string name = entry.substr(0, pos);
                std::string value = entry.substr(pos + 1);
                env_array.array_access(name) = AWKValue(value);
            }
        }
    }
}

void Environment::set_argv(const std::vector<std::string>& args) {
    globals_["ARGC"] = AWKValue(static_cast<double>(args.size()));

    AWKValue& argv_array = globals_["ARGV"];
    for (size_t i = 0; i < args.size(); ++i) {
        argv_array.array_access(std::to_string(i)) = AWKValue(args[i]);
    }
}

bool Environment::is_special_variable(const std::string& name) {
    // List of all special built-in AWK variables that should be accessible
    // without namespace qualification even when inside a namespace
    static const std::unordered_set<std::string> special_vars = {
        // Standard AWK variables
        "FS", "RS", "OFS", "ORS", "NR", "NF", "FNR",
        "FILENAME", "SUBSEP", "CONVFMT", "OFMT",
        "RSTART", "RLENGTH", "ARGC", "ARGV", "ENVIRON",
        // gawk extensions
        "IGNORECASE", "RT", "FPAT", "TEXTDOMAIN",
        "PROCINFO", "SYMTAB", "FUNCTAB"
    };
    return special_vars.count(name) > 0;
}

} // namespace awk
