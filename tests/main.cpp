// AWK Unit Tests - Main Entry Point
#include "test_framework.hpp"

// All test files are included here
#include "lexer_test.cpp"
#include "parser_test.cpp"
#include "interpreter_test.cpp"
#include "i18n_test.cpp"

int main() {
    return RUN_ALL_TESTS();
}
