# Contributing to AWK Interpreter

Thank you for your interest in contributing to the AWK interpreter project.

## Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.14+
- Git

### Setup Development Environment

```bash
# Clone the repository
git clone <repository-url>
cd awk

# Configure with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Run tests
./build/Debug/awk_tests
```

---

## Code Style

### General Guidelines

- Use C++17 features appropriately
- Follow existing code formatting conventions
- Keep functions focused and reasonably sized
- Use meaningful variable and function names

### Naming Conventions

| Element | Style | Example |
|---------|-------|---------|
| Classes | PascalCase | `Interpreter`, `AWKValue` |
| Functions | snake_case | `parse_fields()`, `to_number()` |
| Variables | snake_case | `current_record`, `field_count` |
| Constants | UPPER_SNAKE_CASE | `MAX_CACHE_SIZE` |
| Member variables | trailing underscore | `env_`, `fields_` |
| Parameters | snake_case | `input_files`, `pattern` |

### Formatting

- 4 spaces for indentation (no tabs)
- Opening braces on same line for control structures
- Opening braces on same line for functions
- Maximum line length: 100 characters (soft limit)

```cpp
// Good
void Interpreter::execute(IfStmt& stmt) {
    if (is_truthy(evaluate(*stmt.condition))) {
        execute(*stmt.then_branch);
    } else if (stmt.else_branch) {
        execute(*stmt.else_branch);
    }
}

// Avoid
void Interpreter::execute(IfStmt& stmt)
{
    if(is_truthy(evaluate(*stmt.condition)))
    {
        execute(*stmt.then_branch);
    }
}
```

### Comments

- Use `//` for single-line comments
- Use `/* */` for multi-line comments when appropriate
- Document non-obvious behavior
- Keep comments up-to-date with code changes

```cpp
// Good: Explains why
// AWK uses 1-based indexing for fields, but 0 refers to entire record
if (index == 0) {
    return field0_;
}

// Avoid: States the obvious
// Increment i
i++;
```

---

## Project Structure

```
awk/
├── include/awk/          # Public headers
├── src/                  # Implementation files
├── tests/                # Unit tests
├── integration_tests/    # Integration tests
│   ├── scripts/          # Test AWK scripts
│   ├── input/            # Test input files
│   ├── expected/         # Expected outputs
│   └── run_tests.sh      # Test runner
├── docs/                 # Documentation
└── CMakeLists.txt        # Build configuration
```

---

## Making Changes

### Branch Naming

- `feature/description` - New features
- `fix/description` - Bug fixes
- `docs/description` - Documentation changes
- `refactor/description` - Code refactoring

### Commit Messages

Use clear, descriptive commit messages:

```
Add support for strftime() function

- Implement strftime() with common format specifiers
- Add %Y, %m, %d, %H, %M, %S format codes
- Add unit tests for strftime()
- Update FEATURES.md documentation
```

Format:
- First line: Brief summary (50 chars or less)
- Blank line
- Detailed description if needed (wrap at 72 chars)

---

## Testing

### Running Tests

```bash
# Unit tests
./build/Release/awk_tests

# Integration tests (Linux/macOS)
cd integration_tests
./run_tests.sh

# Integration tests (Windows)
cd integration_tests
run_tests.bat
```

### Writing Unit Tests

Add tests to the appropriate test file in `tests/`:

```cpp
// In tests/interpreter_test.cpp

TEST_F(InterpreterTest, MyNewFeature) {
    // Arrange
    std::string program = "BEGIN { print myfunction(5) }";

    // Act
    std::string output = run_program(program);

    // Assert
    EXPECT_EQ(output, "expected_result\n");
}
```

Test naming convention:
- `TestCategory_SpecificBehavior`
- Examples: `Lexer_ScansRegex`, `Parser_HandlesSwitch`, `Interpreter_SubstrFunction`

### Writing Integration Tests

1. Create AWK script in `integration_tests/scripts/`
2. Create input file in `integration_tests/input/` (if needed)
3. Create expected output in `integration_tests/expected/`
4. Add test to `run_tests.sh`

Example:
```bash
# In run_tests.sh
run_test "my_feature" "scripts/99_my_feature.awk" "input/my_data.txt"
```

### Test Coverage Goals

All new features should have:
- Unit tests for the happy path
- Unit tests for edge cases
- Unit tests for error conditions
- Integration tests for end-to-end behavior

---

## Adding New Features

### Adding a Built-in Function

1. **Choose the right file** based on category:
   - String functions → `interpreter_builtins_string.cpp`
   - Math functions → `interpreter_builtins_math.cpp`
   - I/O functions → `interpreter_builtins_io.cpp`
   - Other → `interpreter_builtins_misc.cpp`

2. **Register the function**:
```cpp
void Interpreter::register_string_builtins() {
    // ... existing functions ...

    env_.register_builtin("myfunction", [this](std::vector<AWKValue>& args) {
        if (args.size() < 1) {
            throw std::runtime_error("myfunction requires 1 argument");
        }
        // Implementation
        return AWKValue(result);
    });
}
```

3. **Add tests** in `tests/interpreter_test.cpp`

4. **Update documentation** in `docs/FEATURES.md`

### Adding a New Statement Type

1. **Define AST node** in `include/awk/ast.hpp`:
```cpp
struct MyStmt : Stmt {
    ExprPtr expression;
    // ... other members
};
```

2. **Parse in parser** in `src/parser.cpp`:
```cpp
StmtPtr Parser::statement() {
    // ... existing cases ...
    if (match(TokenType::MY_KEYWORD)) {
        return my_statement();
    }
}

StmtPtr Parser::my_statement() {
    // Parse the statement
}
```

3. **Execute in interpreter** in `src/interpreter_exec.cpp`:
```cpp
void Interpreter::execute(MyStmt& stmt) {
    // Implementation
}
```

4. **Add tests** and documentation

---

## Debugging Tips

### Enable Debug Output

```cpp
// Temporary debug output
std::cerr << "DEBUG: value=" << value.to_string() << "\n";
```

### Test Individual Components

```cpp
// Test lexer alone
awk::Lexer lexer("{ print $1 }");
while (!lexer.is_at_end()) {
    auto token = lexer.next_token();
    std::cout << token.type << ": " << token.lexeme << "\n";
}

// Test parser alone
auto program = parser.parse();
// Inspect program->rules, program->functions

// Test specific evaluation
AWKValue result = interpreter.evaluate(*expr);
```

### Use Integration Tests for Debugging

Create a minimal test case:
```awk
# debug_test.awk
BEGIN {
    # Minimal reproduction of issue
    x = problematic_expression
    print x
}
```

Run:
```bash
./build/Debug/awk -f debug_test.awk
```

---

## Pull Request Process

1. **Create a branch** from `main`
2. **Make your changes** following the guidelines above
3. **Write/update tests** for your changes
4. **Update documentation** as needed
5. **Run all tests** and ensure they pass
6. **Submit pull request** with clear description

### PR Description Template

```markdown
## Summary
Brief description of the changes.

## Changes
- List of specific changes made
- Another change

## Testing
- How the changes were tested
- New tests added

## Documentation
- Documentation updates made

## Related Issues
Closes #123 (if applicable)
```

### Review Criteria

PRs will be reviewed for:
- Correctness (does it work?)
- Test coverage (is it tested?)
- Code quality (is it readable and maintainable?)
- Documentation (is it documented?)
- Compatibility (does it maintain backward compatibility?)

---

## Reporting Issues

### Bug Reports

Include:
- AWK interpreter version
- Operating system
- Minimal reproduction case (AWK program + input)
- Expected behavior
- Actual behavior

### Feature Requests

Include:
- Description of the feature
- Use case / motivation
- Example of desired syntax/behavior
- Reference to AWK/gawk documentation if applicable

---

## Questions?

If you have questions about contributing:
- Check existing documentation
- Look at similar implementations in the codebase
- Open an issue for discussion
