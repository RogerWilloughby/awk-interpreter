#include "awk/lexer.hpp"
#include "awk/parser.hpp"
#include "awk/interpreter.hpp"
#include "awk/platform.hpp"
#include "space_invaders.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>

void print_usage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " [options] 'program' [file ...]\n"
              << "       " << program_name << " [options] -f progfile [file ...]\n"
              << "\nOptions:\n"
              << "  -F fs         Set field separator to fs\n"
              << "  -v var=value  Assign value to variable before execution\n"
              << "  -f progfile   Read program from file\n"
              << "  -h, --help    Show this help message\n"
              << "  --version     Show version information\n";
}

void print_version() {
    std::cout << "awk 1.0.0\n"
              << "AWK implementation in C++\n"
              << "Based on POSIX AWK and GAWK extensions\n";
}

int main(int argc, char* argv[]) {
    std::string program_source;
    std::vector<std::string> input_files;
    std::vector<std::pair<std::string, std::string>> var_assignments;
    std::string field_separator;
    bool program_from_file = false;
    std::string program_file;

    // Parse arguments
    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }

        if (arg == "--version") {
            print_version();
            return 0;
        }

        if (arg == "-F") {
            if (i + 1 >= argc) {
                std::cerr << "awk: option -F requires an argument\n";
                return 1;
            }
            field_separator = argv[++i];
            ++i;
            continue;
        }

        if (arg.substr(0, 2) == "-F") {
            field_separator = arg.substr(2);
            ++i;
            continue;
        }

        if (arg == "-v") {
            if (i + 1 >= argc) {
                std::cerr << "awk: option -v requires an argument\n";
                return 1;
            }
            std::string assignment = argv[++i];
            size_t eq_pos = assignment.find('=');
            if (eq_pos == std::string::npos) {
                std::cerr << "awk: invalid -v argument: " << assignment << "\n";
                return 1;
            }
            var_assignments.emplace_back(
                assignment.substr(0, eq_pos),
                assignment.substr(eq_pos + 1)
            );
            ++i;
            continue;
        }

        if (arg == "-f") {
            if (i + 1 >= argc) {
                std::cerr << "awk: option -f requires an argument\n";
                return 1;
            }
            program_file = argv[++i];
            program_from_file = true;
            ++i;
            continue;
        }

        // Easter egg: Space Invaders game
        if (arg == "-undoc") {
            return run_space_invaders();
        }

        if (arg == "--") {
            ++i;
            break;
        }

        if (arg[0] == '-' && arg.length() > 1) {
            std::cerr << "awk: unknown option: " << arg << "\n";
            return 1;
        }

        // First non-option argument
        break;
    }

    // Determine program source
    if (program_from_file) {
        std::ifstream file(program_file);
        if (!file) {
            std::cerr << "awk: can't open file " << program_file << ": " << awk::safe_strerror(errno) << "\n";
            return 1;
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        program_source = ss.str();
    } else {
        if (i >= argc) {
            std::cerr << "awk: no program given\n";
            print_usage(argv[0]);
            return 1;
        }
        program_source = argv[i++];
    }

    // Remaining arguments are input files
    while (i < argc) {
        input_files.push_back(argv[i++]);
    }

    // Lexer and Parser
    awk::Lexer lexer(program_source);
    awk::Parser parser(lexer);

    auto program = parser.parse();

    if (parser.had_error()) {
        for (const auto& error : parser.errors()) {
            std::cerr << error << "\n";
        }
        return 1;
    }

    // Interpreter
    awk::Interpreter interpreter;

    // Set field separator
    if (!field_separator.empty()) {
        interpreter.environment().FS() = awk::AWKValue(field_separator);
    }

    // Variable assignments
    for (const auto& [name, value] : var_assignments) {
        // Check if value looks numeric
        char* end;
        double num = std::strtod(value.c_str(), &end);
        if (end != value.c_str() && *end == '\0') {
            interpreter.environment().set_variable(name, awk::AWKValue(num));
        } else {
            interpreter.environment().set_variable(name, awk::AWKValue(value));
        }
    }

    // Run the program
    try {
        interpreter.run(*program, input_files);
    } catch (const std::exception& e) {
        std::cerr << "awk: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
