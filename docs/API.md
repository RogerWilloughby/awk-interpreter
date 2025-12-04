# AWK Interpreter API

This document describes how to embed the AWK interpreter in your C++ application.

## Quick Start

### Include the Library

```cpp
#include <awk.hpp>
```

### Simple Execution

```cpp
#include <awk.hpp>

int main() {
    // Run AWK program from string
    int result = awk::run("BEGIN { print \"Hello, World!\" }");
    return result;
}
```

### Process Files

```cpp
#include <awk.hpp>

int main() {
    // Run AWK program on files
    std::vector<std::string> files = {"data1.txt", "data2.txt"};
    int result = awk::run("{ print NR, $0 }", files);
    return result;
}
```

### Run From Script File

```cpp
#include <awk.hpp>

int main() {
    std::vector<std::string> input_files = {"data.txt"};
    int result = awk::run_file("script.awk", input_files);
    return result;
}
```

---

## Core Classes

### Lexer

Converts source code into tokens.

```cpp
#include <awk/lexer.hpp>

awk::Lexer lexer(source_code);

while (!lexer.is_at_end()) {
    awk::Token token = lexer.next_token();
    // Process token...
}
```

**Methods:**

| Method | Description |
|--------|-------------|
| `Lexer(std::string source)` | Constructor |
| `Token next_token()` | Get next token |
| `Token peek_token()` | Peek without consuming |
| `bool is_at_end()` | Check if at end of source |
| `size_t current_line()` | Current line number |
| `size_t current_column()` | Current column number |

### Parser

Builds an Abstract Syntax Tree from tokens.

```cpp
#include <awk/lexer.hpp>
#include <awk/parser.hpp>

awk::Lexer lexer(source_code);
awk::Parser parser(lexer);

auto program = parser.parse();

if (parser.had_error()) {
    for (const auto& error : parser.errors()) {
        std::cerr << error << "\n";
    }
}
```

**Methods:**

| Method | Description |
|--------|-------------|
| `Parser(Lexer& lexer)` | Constructor |
| `std::unique_ptr<Program> parse()` | Parse entire program |
| `bool had_error()` | Check for parse errors |
| `const std::vector<std::string>& errors()` | Get error messages |

**Static Methods:**

| Method | Description |
|--------|-------------|
| `parse_file(const std::string& filename)` | Parse AWK file (with @include support) |
| `parse_string(const std::string& source, const std::string& base_path)` | Parse string (with @include support) |

### Interpreter

Executes the parsed program.

```cpp
#include <awk/interpreter.hpp>

awk::Interpreter interpreter;

// Optional: Configure before running
interpreter.environment().FS() = awk::AWKValue(",");

// Run program
std::vector<std::string> files = {"data.csv"};
interpreter.run(*program, files);
```

**Methods:**

| Method | Description |
|--------|-------------|
| `Interpreter()` | Constructor |
| `void run(Program& program, const std::vector<std::string>& files)` | Execute program |
| `Environment& environment()` | Access environment |
| `void set_output_stream(std::ostream& os)` | Redirect output |
| `void set_error_stream(std::ostream& os)` | Redirect errors |

### Environment

Manages variables, arrays, and functions.

```cpp
awk::Environment& env = interpreter.environment();

// Set variables
env.set_variable("x", awk::AWKValue(42));
env.set_variable("name", awk::AWKValue("John"));

// Get variables
awk::AWKValue& x = env.get_variable("x");

// Special variables
env.FS() = awk::AWKValue(",");
env.OFS() = awk::AWKValue("\t");
```

**Special Variable Access:**

| Method | Variable | Default |
|--------|----------|---------|
| `FS()` | Field separator | `" "` |
| `RS()` | Record separator | `"\n"` |
| `OFS()` | Output field separator | `" "` |
| `ORS()` | Output record separator | `"\n"` |
| `NR()` | Total record count | `0` |
| `NF()` | Field count | `0` |
| `FNR()` | File record count | `0` |
| `FILENAME()` | Current filename | `""` |
| `SUBSEP()` | Array subscript separator | `"\034"` |
| `CONVFMT()` | Number conversion format | `"%.6g"` |
| `OFMT()` | Output number format | `"%.6g"` |
| `IGNORECASE()` | Case-insensitive matching | `0` |

**Variable Methods:**

| Method | Description |
|--------|-------------|
| `AWKValue& get_variable(const std::string& name)` | Get/create variable |
| `void set_variable(const std::string& name, AWKValue value)` | Set variable |
| `bool has_variable(const std::string& name)` | Check existence |

**Array Methods:**

| Method | Description |
|--------|-------------|
| `AWKValue& get_array_element(const std::string& name, const std::string& key)` | Get/create element |
| `bool array_element_exists(const std::string& name, const std::string& key)` | Check existence |
| `void delete_array_element(const std::string& name, const std::string& key)` | Delete element |
| `void delete_array(const std::string& name)` | Delete entire array |
| `const std::unordered_map<std::string, AWKValue>& get_array(const std::string& name)` | Get array map |

### AWKValue

Represents AWK's dynamically-typed values.

```cpp
// Create values
awk::AWKValue num(42.5);
awk::AWKValue str("hello");
awk::AWKValue empty;  // Uninitialized

// Conversions
double d = num.to_number();      // 42.5
std::string s = num.to_string(); // "42.5"
bool b = num.to_bool();          // true

// Type checking
bool is_num = num.is_number();   // true
bool is_str = str.is_string();   // true
```

**Constructors:**

| Constructor | Description |
|-------------|-------------|
| `AWKValue()` | Uninitialized (0/"") |
| `AWKValue(double)` | Numeric value |
| `AWKValue(const std::string&)` | String value |
| `AWKValue(const char*)` | String value |

**Conversion Methods:**

| Method | Description |
|--------|-------------|
| `double to_number() const` | Convert to number |
| `std::string to_string() const` | Convert to string |
| `bool to_bool() const` | Convert to boolean |

**Type Methods:**

| Method | Description |
|--------|-------------|
| `bool is_number() const` | True if numeric |
| `bool is_string() const` | True if string |
| `bool is_uninitialized() const` | True if uninitialized |

---

## Advanced Usage

### Custom Output

Redirect AWK output to a string stream:

```cpp
#include <awk.hpp>
#include <sstream>

std::ostringstream output;

awk::Lexer lexer("BEGIN { print \"test\" }");
awk::Parser parser(lexer);
auto program = parser.parse();

awk::Interpreter interpreter;
interpreter.set_output_stream(output);
interpreter.run(*program, {});

std::string result = output.str();  // "test\n"
```

### Pre-set Variables

```cpp
awk::Interpreter interpreter;
awk::Environment& env = interpreter.environment();

// Set field separator
env.FS() = awk::AWKValue(",");

// Set custom variables
env.set_variable("threshold", awk::AWKValue(100));
env.set_variable("prefix", awk::AWKValue("LOG:"));

// Run with pre-configured environment
interpreter.run(*program, files);
```

### Access Results After Execution

```cpp
awk::Interpreter interpreter;
interpreter.run(*program, files);

// Access final variable values
awk::AWKValue& total = interpreter.environment().get_variable("total");
double sum = total.to_number();

// Access arrays
const auto& counts = interpreter.environment().get_array("counts");
for (const auto& [key, value] : counts) {
    std::cout << key << ": " << value.to_number() << "\n";
}
```

### Error Handling

```cpp
try {
    awk::Lexer lexer(source);
    awk::Parser parser(lexer);
    auto program = parser.parse();

    if (parser.had_error()) {
        // Syntax errors
        for (const auto& error : parser.errors()) {
            std::cerr << "Parse error: " << error << "\n";
        }
        return 1;
    }

    awk::Interpreter interpreter;
    interpreter.run(*program, files);

} catch (const std::exception& e) {
    // Runtime errors
    std::cerr << "Runtime error: " << e.what() << "\n";
    return 1;
}
```

### Multiple Program Executions

```cpp
// Parse once, run multiple times
awk::Lexer lexer(source);
awk::Parser parser(lexer);
auto program = parser.parse();

for (const auto& dataset : datasets) {
    awk::Interpreter interpreter;  // Fresh interpreter each time
    interpreter.run(*program, {dataset});
}
```

---

## Linking

### CMake

```cmake
# Add the AWK library as a subdirectory
add_subdirectory(awk)

# Link against the library
target_link_libraries(your_project PRIVATE awk_lib)
```

### Manual Compilation

```bash
# Compile with include path
g++ -std=c++17 -I/path/to/awk/include your_program.cpp -L/path/to/awk/build/Release -lawk_lib -o your_program
```

---

## Complete Example

```cpp
#include <awk.hpp>
#include <iostream>
#include <sstream>

int main() {
    // AWK program: Sum values by category
    const char* program_source = R"(
        BEGIN { FS = "," }
        NR > 1 {
            category = $1
            amount = $2
            totals[category] += amount
            count[category]++
        }
        END {
            for (cat in totals) {
                avg = totals[cat] / count[cat]
                printf "%s: total=%.2f, avg=%.2f\n", cat, totals[cat], avg
            }
        }
    )";

    // Sample data
    std::string data =
        "category,amount\n"
        "A,100\n"
        "B,200\n"
        "A,150\n"
        "B,250\n"
        "A,120\n";

    // Write data to temp file
    std::ofstream temp("temp_data.csv");
    temp << data;
    temp.close();

    // Parse program
    awk::Lexer lexer(program_source);
    awk::Parser parser(lexer);
    auto program = parser.parse();

    if (parser.had_error()) {
        for (const auto& error : parser.errors()) {
            std::cerr << error << "\n";
        }
        return 1;
    }

    // Capture output
    std::ostringstream output;

    // Run
    awk::Interpreter interpreter;
    interpreter.set_output_stream(output);
    interpreter.run(*program, {"temp_data.csv"});

    // Display results
    std::cout << output.str();

    // Cleanup
    std::remove("temp_data.csv");

    return 0;
}
```

**Output:**
```
A: total=370.00, avg=123.33
B: total=450.00, avg=225.00
```

---

## Thread Safety

The interpreter is **not thread-safe**. Each thread should use its own `Interpreter` instance. The `Lexer` and `Parser` can be used to parse programs once, and the resulting `Program` AST can be shared (read-only) across threads, with each thread creating its own `Interpreter` to execute it.

```cpp
auto program = parse_program_once();

// Each thread gets its own interpreter
std::thread t1([&program]() {
    awk::Interpreter interp;
    interp.run(*program, {"data1.txt"});
});

std::thread t2([&program]() {
    awk::Interpreter interp;
    interp.run(*program, {"data2.txt"});
});
```
