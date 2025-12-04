#include <awk.hpp>
#include <iostream>
#include <sstream>

int main() {
    std::cout << "Testing AWK interpreter..." << std::endl;

    // Test basic execution
    std::ostringstream output;
    awk::Lexer lexer("BEGIN { print \"Hello from AWK!\" }");
    awk::Parser parser(lexer);
    auto program = parser.parse();

    if (parser.had_error()) {
        std::cerr << "Parse error!" << std::endl;
        return 1;
    }

    awk::Interpreter interpreter;
    interpreter.set_output_stream(output);
    interpreter.run(*program, {});

    if (output.str() == "Hello from AWK!\n") {
        std::cout << "Test passed!" << std::endl;
        return 0;
    } else {
        std::cerr << "Test failed: unexpected output" << std::endl;
        return 1;
    }
}
