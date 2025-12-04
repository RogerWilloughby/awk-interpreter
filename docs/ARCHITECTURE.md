# AWK Interpreter Architecture

This document describes the internal architecture of the AWK interpreter.

## Overview

The interpreter follows a classic compiler pipeline:

```
Source Code → Lexer → Parser → AST → Interpreter → Output
                                ↑
                            Environment
```

## Pipeline Stages

### 1. Lexical Analysis (Lexer)

**File:** `src/lexer.cpp`, `include/awk/lexer.hpp`

The lexer converts source text into a stream of tokens.

```
Source: { print $1, $2 }

Tokens: LBRACE  PRINT  DOLLAR  NUMBER(1)  COMMA  DOLLAR  NUMBER(2)  RBRACE
```

Key features:
- **Context-sensitive tokenization**: Distinguishes `/` (division) from `/regex/` based on context
- **Keyword recognition**: `BEGIN`, `END`, `if`, `while`, `for`, `function`, etc.
- **String processing**: Handles escape sequences (`\n`, `\t`, `\xHH`)
- **Number parsing**: Integer, floating-point, hex (`0x`), and octal (`0`)

The lexer maintains state to correctly handle AWK's ambiguous syntax:
- After operators like `~` or `!~`, `/` starts a regex
- After values like numbers or identifiers, `/` is division

### 2. Parsing (Parser)

**File:** `src/parser.cpp`, `include/awk/parser.hpp`

The parser performs recursive descent parsing to build an Abstract Syntax Tree (AST).

```
                    Program
                       │
        ┌──────────────┼──────────────┐
        │              │              │
    Functions       Rules          Rules
        │              │              │
   FunctionDef     Pattern+Action   Pattern+Action
        │              │              │
    StmtPtr        Pattern      BlockStmt
                                    │
                              vector<StmtPtr>
```

Key features:
- **Operator precedence**: Correctly handles AWK's 15+ precedence levels
- **Pattern types**: BEGIN, END, BEGINFILE, ENDFILE, regex, expression, range
- **Namespace support** (gawk): `@namespace` directive
- **Include support** (gawk): `@include` directive with cycle detection

Parser expression precedence (lowest to highest):
1. Assignment (`=`, `+=`, `-=`, etc.)
2. Ternary (`?:`)
3. Logical OR (`||`)
4. Logical AND (`&&`)
5. Array membership (`in`)
6. Regex match (`~`, `!~`)
7. Comparison (`<`, `<=`, `>`, `>=`, `==`, `!=`)
8. Concatenation (implicit)
9. Addition (`+`, `-`)
10. Multiplication (`*`, `/`, `%`)
11. Power (`^`)
12. Unary (`!`, `-`, `+`, `++`, `--`)
13. Field access (`$`)
14. Primary (literals, variables, calls)

### 3. Abstract Syntax Tree (AST)

**File:** `include/awk/ast.hpp`

The AST represents the program structure using C++ classes.

#### Expression Types

| Class | Purpose | Example |
|-------|---------|---------|
| `LiteralExpr` | Numbers and strings | `42`, `"hello"` |
| `RegexExpr` | Regex patterns | `/[0-9]+/` |
| `VariableExpr` | Variable access | `x`, `NR` |
| `FieldExpr` | Field access | `$1`, `$NF` |
| `ArrayAccessExpr` | Array element | `arr[i]`, `arr[i,j]` |
| `BinaryExpr` | Binary operators | `a + b` |
| `UnaryExpr` | Unary operators | `!x`, `++i` |
| `TernaryExpr` | Conditional | `a ? b : c` |
| `AssignExpr` | Assignment | `x = 5` |
| `CallExpr` | Function call | `length(s)` |
| `MatchExpr` | Regex match | `s ~ /pat/` |
| `ConcatExpr` | String concatenation | `a b c` |
| `GetlineExpr` | Getline variants | `getline x < file` |
| `InExpr` | Array membership | `k in arr` |

#### Statement Types

| Class | Purpose | Example |
|-------|---------|---------|
| `BlockStmt` | Statement block | `{ stmt; stmt; }` |
| `IfStmt` | Conditional | `if (cond) stmt` |
| `WhileStmt` | While loop | `while (cond) stmt` |
| `DoWhileStmt` | Do-while loop | `do stmt while (cond)` |
| `ForStmt` | For loop | `for (;;) stmt` |
| `ForInStmt` | For-in loop | `for (k in arr) stmt` |
| `SwitchStmt` | Switch (gawk) | `switch (x) { ... }` |
| `PrintStmt` | Print output | `print x, y` |
| `PrintfStmt` | Formatted output | `printf "%d", x` |
| `ExprStmt` | Expression | `x++` |
| `DeleteStmt` | Delete array | `delete arr[k]` |
| `BreakStmt` | Break loop | `break` |
| `ContinueStmt` | Continue loop | `continue` |
| `NextStmt` | Next record | `next` |
| `NextfileStmt` | Next file | `nextfile` |
| `ExitStmt` | Exit program | `exit 1` |
| `ReturnStmt` | Return value | `return x` |

### 4. Execution (Interpreter)

**File:** `src/interpreter.cpp`, `src/interpreter_*.cpp`

The interpreter walks the AST and executes it.

#### Execution Flow

```
1. Initialize environment (FS, RS, OFS, etc.)
2. Register built-in functions
3. Execute BEGIN rules
4. For each input file:
   a. Execute BEGINFILE rules
   b. For each record:
      - Parse fields
      - Match patterns
      - Execute matching actions
   c. Execute ENDFILE rules
5. Execute END rules
6. Cleanup (close files, pipes)
```

#### Control Flow

Control flow is implemented using C++ exceptions:

| Exception | Statement | Effect |
|-----------|-----------|--------|
| `BreakException` | `break` | Exit innermost loop |
| `ContinueException` | `continue` | Next iteration |
| `NextException` | `next` | Next record |
| `NextfileException` | `nextfile` | Next file |
| `ReturnException` | `return` | Return from function |
| `ExitException` | `exit` | End program |

#### Built-in Functions

Built-in functions are organized by category in separate files:

| File | Category | Functions |
|------|----------|-----------|
| `interpreter_builtins_string.cpp` | String | `length`, `substr`, `index`, `split`, `sub`, `gsub`, `gensub`, `match`, `tolower`, `toupper`, `sprintf` |
| `interpreter_builtins_math.cpp` | Math | `sin`, `cos`, `sqrt`, `log`, `exp`, `int`, `rand`, `srand` |
| `interpreter_builtins_io.cpp` | I/O | `getline`, `close`, `fflush`, `system` |
| `interpreter_builtins_misc.cpp` | Misc | `systime`, `mktime`, `strftime`, bitwise ops, `typeof` |

### 5. Environment

**File:** `src/environment.cpp`, `include/awk/environment.hpp`

The environment manages:

- **Variables**: Global and local variable storage
- **Special variables**: `FS`, `RS`, `OFS`, `ORS`, `NR`, `NF`, `FILENAME`, etc.
- **Arrays**: Associative arrays with SUBSEP support
- **Function table**: User-defined and built-in functions

```cpp
class Environment {
    // Special variables (direct access for performance)
    AWKValue& FS();
    AWKValue& RS();
    AWKValue& OFS();
    AWKValue& NR();
    AWKValue& NF();
    // ...

    // General variable access
    AWKValue& get_variable(const std::string& name);
    void set_variable(const std::string& name, AWKValue value);

    // Arrays
    AWKValue& get_array_element(const std::string& name, const std::string& key);
    bool array_element_exists(const std::string& name, const std::string& key);
    void delete_array_element(const std::string& name, const std::string& key);
};
```

### 6. Value System

**File:** `src/value.cpp`, `include/awk/value.hpp`

AWK values are dynamically typed, supporting both string and numeric representations.

```cpp
class AWKValue {
    enum class Type { UNINITIALIZED, NUMBER, STRING, STRNUM };

    // Implicit conversions
    double to_number() const;
    std::string to_string() const;
    bool to_bool() const;

    // Type checking
    bool is_number() const;
    bool is_string() const;
};
```

Key behaviors:
- Uninitialized values are `""` (string) or `0` (number)
- String-to-number conversion: Leading numeric portion, or `0`
- Number-to-string conversion: Uses `CONVFMT` (default `"%.6g"`)
- Boolean: `0` and `""` are false, everything else is true

---

## Key Components

### Regex Cache

**File:** `src/regex_cache.cpp`

For performance, compiled regex patterns are cached:

```cpp
class RegexCache {
    static constexpr size_t MAX_CACHE_SIZE = 64;

    const std::regex& get(const std::string& pattern,
                          std::regex_constants::syntax_option_type flags);
};
```

### Field Parsing

Field splitting is a critical operation. The interpreter supports:

1. **Single-character FS**: Direct string splitting
2. **Multi-character FS**: Regex-based splitting
3. **FPAT** (gawk): Pattern matching for fields

```cpp
void Interpreter::parse_fields() {
    if (fpat_mode) {
        // Match FPAT pattern to extract fields
    } else if (fs.length() == 1) {
        // Fast single-character split
    } else {
        // Regex-based split
    }
}
```

### I/O Management

The interpreter manages multiple I/O channels:

```cpp
// Output files (print > "file")
std::unordered_map<std::string, std::unique_ptr<std::ofstream>> output_files_;

// Input files (getline < "file")
std::unordered_map<std::string, std::unique_ptr<std::ifstream>> input_files_;

// Output pipes (print | "cmd")
std::unordered_map<std::string, std::unique_ptr<PipeOStream>> output_pipes_;

// Input pipes ("cmd" | getline)
std::unordered_map<std::string, FILE*> input_pipes_;

// Coprocesses (print |& "cmd"; "cmd" |& getline) - gawk extension
std::unordered_map<std::string, std::unique_ptr<Coprocess>> coprocesses_;
```

---

## Performance Optimizations

### 1. Special Variable Caching

Frequently accessed special variables (`FS`, `RS`, `OFS`, `SUBSEP`) are cached to avoid repeated string conversions.

### 2. Regex Caching

Compiled regex patterns are cached with LRU eviction.

### 3. Field Lazy Evaluation

Fields are only parsed when accessed, not on every record read.

### 4. String Pre-allocation

String operations pre-allocate buffers based on expected size.

---

## Directory Structure

```
awk/
├── include/
│   ├── awk.hpp                 # Main public header
│   └── awk/
│       ├── ast.hpp             # AST node definitions
│       ├── environment.hpp     # Variable/array storage
│       ├── interpreter.hpp     # Interpreter class
│       ├── lexer.hpp           # Lexer class
│       ├── parser.hpp          # Parser class
│       ├── token.hpp           # Token types
│       └── value.hpp           # AWKValue class
├── src/
│   ├── main.cpp                # CLI entry point
│   ├── lexer.cpp               # Lexer implementation
│   ├── parser.cpp              # Parser implementation
│   ├── interpreter.cpp         # Core interpreter
│   ├── interpreter_eval.cpp    # Expression evaluation
│   ├── interpreter_exec.cpp    # Statement execution
│   ├── interpreter_getline.cpp # Getline variants
│   ├── interpreter_coprocess.cpp # Coprocess support
│   ├── interpreter_builtins_*.cpp # Built-in functions
│   ├── environment.cpp         # Environment implementation
│   ├── value.cpp               # AWKValue implementation
│   ├── regex_cache.cpp         # Regex caching
│   └── i18n.cpp                # Internationalization
├── tests/
│   ├── lexer_test.cpp          # Lexer unit tests
│   ├── parser_test.cpp         # Parser unit tests
│   └── interpreter_test.cpp    # Interpreter unit tests
└── integration_tests/
    ├── scripts/                # Test AWK scripts
    ├── input/                  # Test input files
    ├── expected/               # Expected outputs
    └── run_tests.sh            # Test runner
```

---

## Extending the Interpreter

### Adding a Built-in Function

1. Add declaration in `interpreter.hpp` (in `register_*_builtins` section)
2. Implement in appropriate `interpreter_builtins_*.cpp`
3. Register in `register_builtins()` function
4. Add tests in `interpreter_test.cpp`

Example:
```cpp
// In interpreter_builtins_string.cpp
void Interpreter::register_string_builtins() {
    env_.register_builtin("myfunction", [this](std::vector<AWKValue>& args) {
        // Implementation
        return AWKValue(result);
    });
}
```

### Adding a New AST Node

1. Define struct in `ast.hpp`
2. Add case in parser where the construct can appear
3. Add evaluation method in `interpreter_eval.cpp` or execution in `interpreter_exec.cpp`
4. Add tests

### Adding a New Statement Type

1. Define struct in `ast.hpp` (inherit from `Stmt`)
2. Parse in `parser.cpp` in `statement()` method
3. Implement `execute()` overload in `interpreter_exec.cpp`
4. Add tests
