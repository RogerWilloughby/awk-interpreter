# AWK Interpreter

A complete AWK interpreter implemented in C++17 with full POSIX AWK support and extensive gawk extensions.

## Features

- **100% POSIX AWK compliant** - All standard AWK features implemented
- **gawk extensions** - BEGINFILE/ENDFILE, switch/case, coprocesses, namespaces, and more
- **50+ built-in functions** - String, math, array, I/O, time, and bitwise operations
- **Cross-platform** - Builds on Windows (MSVC) and Linux/macOS (GCC/Clang)
- **Comprehensive test suite** - 632 unit tests + 32 integration tests

## Quick Start

```bash
# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run
./build/Release/awk 'BEGIN { print "Hello, World!" }'

# Process a file
./build/Release/awk -F',' '{ print $1, $3 }' data.csv

# Run a script
./build/Release/awk -f script.awk input.txt
```

## Example

```awk
# Word frequency counter
{
    for (i = 1; i <= NF; i++)
        words[tolower($i)]++
}
END {
    for (word in words)
        printf "%4d %s\n", words[word], word
}
```

## Documentation

- [Building](BUILDING.md) - Build instructions for all platforms
- [Usage](docs/USAGE.md) - Command-line options and usage
- [Features](docs/FEATURES.md) - Complete feature list and gawk compatibility
- [Examples](docs/EXAMPLES.md) - AWK script examples
- [Architecture](docs/ARCHITECTURE.md) - Interpreter design and internals
- [API](docs/API.md) - Embedding the interpreter in your C++ project
- [Contributing](docs/CONTRIBUTING.md) - How to contribute

## Supported Features

### Core AWK
- Patterns: BEGIN, END, regex, range, expression
- Fields: $0, $1...$NF, field assignment
- Operators: arithmetic, comparison, logical, regex match (~, !~)
- Control flow: if/else, while, do-while, for, for-in
- Arrays: associative arrays, multi-dimensional (SUBSEP)
- Functions: user-defined with local variables
- I/O: print, printf, getline, output redirection (>, >>, |)

### gawk Extensions
- BEGINFILE/ENDFILE patterns
- switch/case/default statement
- Coprocesses (|& operator)
- @namespace directive
- Indirect function calls (@var)
- Internationalization (i18n)
- Bitwise operations (and, or, xor, lshift, rshift, compl)
- IGNORECASE, RT, FPAT variables
- gensub(), patsplit(), asort(), asorti()

### Built-in Functions
| Category | Functions |
|----------|-----------|
| String | length, substr, index, split, sub, gsub, gensub, match, tolower, toupper, sprintf, strtonum |
| Math | sin, cos, tan, atan2, exp, log, sqrt, int, rand, srand |
| Array | asort, asorti, isarray |
| I/O | print, printf, getline, close, fflush, system |
| Time | systime, mktime, strftime |
| Type | typeof, isarray, mkbool |

## Test Results

```
Unit Tests:        632 passed, 0 failed
Integration Tests:  32 passed, 0 failed
gawk Compatibility: 31/32 tests produce identical output
```

## Requirements

- C++17 compatible compiler
- CMake 3.14+

## License

See [LICENSE](LICENSE) file for details.

## Acknowledgments

- GNU AWK (gawk) for the comprehensive AWK manual and reference implementation
- The One True Awk for the original AWK specification
