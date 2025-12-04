#ifndef AWK_ENVIRONMENT_HPP
#define AWK_ENVIRONMENT_HPP

#include "value.hpp"
#include "ast.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>

namespace awk {

// Forward declaration
class Interpreter;

// Type for built-in functions
using BuiltinFunction = std::function<AWKValue(std::vector<AWKValue>&, Interpreter&)>;

// Runtime environment for AWK
class Environment {
public:
    Environment();

    // ========================================================================
    // Variable Management
    // ========================================================================

    // Get variable (creates if needed)
    AWKValue& get_variable(const std::string& name);

    // Set variable
    void set_variable(const std::string& name, AWKValue value);

    // Check if variable exists
    bool has_variable(const std::string& name) const;

    // Delete variable
    void delete_variable(const std::string& name);

    // ========================================================================
    // Scope Management for Functions
    // ========================================================================

    // Create new scope (for function calls)
    void push_scope();

    // Leave scope
    void pop_scope();

    // Get local variable
    AWKValue& get_local(const std::string& name);

    // Set local variable
    void set_local(const std::string& name, AWKValue value);

    // Check if in a function scope
    bool in_function_scope() const { return !scope_stack_.empty(); }

    // ========================================================================
    // Built-in Variables (Shortcuts)
    // ========================================================================

    // Field Separator
    AWKValue& FS() { return get_variable("FS"); }
    const AWKValue& FS() const { return const_cast<Environment*>(this)->FS(); }

    // Record Separator
    AWKValue& RS() { return get_variable("RS"); }
    const AWKValue& RS() const { return const_cast<Environment*>(this)->RS(); }

    // Output Field Separator
    AWKValue& OFS() { return get_variable("OFS"); }
    const AWKValue& OFS() const { return const_cast<Environment*>(this)->OFS(); }

    // Output Record Separator
    AWKValue& ORS() { return get_variable("ORS"); }
    const AWKValue& ORS() const { return const_cast<Environment*>(this)->ORS(); }

    // Number of Records (total)
    AWKValue& NR() { return get_variable("NR"); }
    const AWKValue& NR() const { return const_cast<Environment*>(this)->NR(); }

    // Number of Fields
    AWKValue& NF() { return get_variable("NF"); }
    const AWKValue& NF() const { return const_cast<Environment*>(this)->NF(); }

    // File Number of Records
    AWKValue& FNR() { return get_variable("FNR"); }
    const AWKValue& FNR() const { return const_cast<Environment*>(this)->FNR(); }

    // Current filename
    AWKValue& FILENAME() { return get_variable("FILENAME"); }
    const AWKValue& FILENAME() const { return const_cast<Environment*>(this)->FILENAME(); }

    // Array-Subscript-Separator
    AWKValue& SUBSEP() { return get_variable("SUBSEP"); }
    const AWKValue& SUBSEP() const { return const_cast<Environment*>(this)->SUBSEP(); }

    // Conversion format
    AWKValue& CONVFMT() { return get_variable("CONVFMT"); }
    const AWKValue& CONVFMT() const { return const_cast<Environment*>(this)->CONVFMT(); }

    // Output-Format
    AWKValue& OFMT() { return get_variable("OFMT"); }
    const AWKValue& OFMT() const { return const_cast<Environment*>(this)->OFMT(); }

    // Match position (from match())
    AWKValue& RSTART() { return get_variable("RSTART"); }
    const AWKValue& RSTART() const { return const_cast<Environment*>(this)->RSTART(); }

    // Match length (from match())
    AWKValue& RLENGTH() { return get_variable("RLENGTH"); }
    const AWKValue& RLENGTH() const { return const_cast<Environment*>(this)->RLENGTH(); }

    // Argument count
    AWKValue& ARGC() { return get_variable("ARGC"); }
    const AWKValue& ARGC() const { return const_cast<Environment*>(this)->ARGC(); }

    // Arguments (array)
    AWKValue& ARGV() { return get_variable("ARGV"); }
    const AWKValue& ARGV() const { return const_cast<Environment*>(this)->ARGV(); }

    // Environment variables (array)
    AWKValue& ENVIRON() { return get_variable("ENVIRON"); }
    const AWKValue& ENVIRON() const { return const_cast<Environment*>(this)->ENVIRON(); }

    // Case-Insensitive Matching (gawk Extension)
    AWKValue& IGNORECASE() { return get_variable("IGNORECASE"); }
    const AWKValue& IGNORECASE() const { return const_cast<Environment*>(this)->IGNORECASE(); }

    // Record Terminator (gawk extension) - the actual separator found
    AWKValue& RT() { return get_variable("RT"); }
    const AWKValue& RT() const { return const_cast<Environment*>(this)->RT(); }

    // Field Pattern (gawk extension) - alternative to FS
    AWKValue& FPAT() { return get_variable("FPAT"); }
    const AWKValue& FPAT() const { return const_cast<Environment*>(this)->FPAT(); }

    // Text Domain for i18n (gawk extension) - default is "messages"
    AWKValue& TEXTDOMAIN() { return get_variable("TEXTDOMAIN"); }
    const AWKValue& TEXTDOMAIN() const { return const_cast<Environment*>(this)->TEXTDOMAIN(); }

    // ========================================================================
    // Function Registry
    // ========================================================================

    // Register user-defined function
    void register_function(const std::string& name, FunctionDef* func);

    // Get function
    FunctionDef* get_function(const std::string& name);

    // Check if function exists
    bool has_function(const std::string& name) const;

    // All variable names (for SYMTAB iteration)
    std::vector<std::string> get_all_variable_names() const;

    // All function names (for FUNCTAB iteration)
    std::vector<std::string> get_all_function_names() const;

    // Register built-in function
    void register_builtin(const std::string& name, BuiltinFunction func);

    // Get built-in function
    BuiltinFunction get_builtin(const std::string& name);

    // Check if built-in exists
    bool has_builtin(const std::string& name) const;

    // ========================================================================
    // Initialization
    // ========================================================================

    // Initialize built-in variables
    void init_builtins();

    // Load environment variables
    void load_environ();

    // Set command-line arguments
    void set_argv(const std::vector<std::string>& args);

private:
    // Global variables
    std::unordered_map<std::string, AWKValue> globals_;

    // Scope stack for functions (local variables)
    std::vector<std::unordered_map<std::string, AWKValue>> scope_stack_;

    // User-defined functions
    std::unordered_map<std::string, FunctionDef*> user_functions_;

    // Built-in functions
    std::unordered_map<std::string, BuiltinFunction> builtin_functions_;

    // Check if a variable is a special built-in AWK variable
    // (used for namespace fallback lookup)
    static bool is_special_variable(const std::string& name);
};

} // namespace awk

#endif // AWK_ENVIRONMENT_HPP
