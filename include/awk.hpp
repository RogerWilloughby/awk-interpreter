#ifndef AWK_HPP
#define AWK_HPP

// Main header for the AWK library
// Includes all public headers

#include "awk/token.hpp"
#include "awk/lexer.hpp"
#include "awk/ast.hpp"
#include "awk/parser.hpp"
#include "awk/value.hpp"
#include "awk/environment.hpp"
#include "awk/interpreter.hpp"

namespace awk {

// Convenience function: Run AWK program from string
inline int run(const std::string& program_source,
               const std::vector<std::string>& input_files = {}) {
    Lexer lexer(program_source);
    Parser parser(lexer);

    auto program = parser.parse();

    if (parser.had_error()) {
        for (const auto& error : parser.errors()) {
            std::cerr << error << "\n";
        }
        return 1;
    }

    Interpreter interpreter;
    interpreter.run(*program, input_files);

    return 0;
}

// Convenience function: Run AWK program from file
inline int run_file(const std::string& program_file,
                    const std::vector<std::string>& input_files = {}) {
    std::ifstream file(program_file);
    if (!file) {
        std::cerr << "awk: can't open file " << program_file << "\n";
        return 1;
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    return run(ss.str(), input_files);
}

} // namespace awk

#endif // AWK_HPP
