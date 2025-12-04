# AWK Interpreter User Manual

**Version 1.0**

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Getting Started](#2-getting-started)
3. [AWK Program Structure](#3-awk-program-structure)
4. [Patterns](#4-patterns)
5. [Actions](#5-actions)
6. [Variables](#6-variables)
7. [Operators](#7-operators)
8. [Control Structures](#8-control-structures)
9. [Arrays](#9-arrays)
10. [Functions](#10-functions)
11. [Input and Output](#11-input-and-output)
12. [Regular Expressions](#12-regular-expressions)
13. [Built-in Functions Reference](#13-built-in-functions-reference)
14. [Special Variables Reference](#14-special-variables-reference)
15. [gawk Extensions](#15-gawk-extensions)
16. [Examples](#16-examples)
17. [Troubleshooting](#17-troubleshooting)

---

## 1. Introduction

### What is AWK?

AWK is a powerful text-processing programming language designed for pattern scanning and processing. It excels at:

- Extracting fields from structured data
- Transforming text files
- Generating reports
- Processing log files
- Quick data analysis

### About This Interpreter

This AWK interpreter is a complete implementation written in C++17, featuring:

- Full POSIX AWK compliance
- Extensive gawk (GNU AWK) extensions
- Cross-platform support (Windows, Linux, macOS)
- Over 50 built-in functions

---

## 2. Getting Started

### Running AWK Programs

There are three ways to run AWK programs:

**1. Command-line program:**
```bash
awk 'program' input_file
```

**2. Program file:**
```bash
awk -f program.awk input_file
```

**3. Without input file (BEGIN only):**
```bash
awk 'BEGIN { print "Hello, World!" }'
```

### Command-Line Options

| Option | Description |
|--------|-------------|
| `-F fs` | Set field separator |
| `-v var=value` | Set variable before execution |
| `-f file` | Read program from file |
| `-h`, `--help` | Show help |
| `--version` | Show version |

### Your First AWK Program

Create a file `data.txt`:
```
Alice 25 Engineering
Bob 30 Marketing
Carol 28 Engineering
```

Run:
```bash
awk '{ print $1, $3 }' data.txt
```

Output:
```
Alice Engineering
Bob Marketing
Carol Engineering
```

---

## 3. AWK Program Structure

### Basic Structure

An AWK program consists of pattern-action pairs:

```awk
pattern { action }
pattern { action }
...
```

- If **pattern** is omitted, the action applies to all records
- If **action** is omitted, matching records are printed
- Multiple pattern-action pairs are processed in order

### Program Flow

```
1. Execute BEGIN rules (once, before input)
2. For each input file:
   a. Execute BEGINFILE rules
   b. For each record (line):
      - Split into fields
      - Test each pattern
      - Execute matching actions
   c. Execute ENDFILE rules
3. Execute END rules (once, after all input)
```

### Example Program Structure

```awk
BEGIN {
    # Initialization
    FS = ","
    print "Processing started"
}

/pattern/ {
    # Process matching lines
    count++
}

END {
    # Final processing
    print "Total matches:", count
}
```

---

## 4. Patterns

### Pattern Types

#### BEGIN and END

Execute once before/after processing:

```awk
BEGIN { print "Start" }
END { print "End" }
```

#### BEGINFILE and ENDFILE (gawk)

Execute before/after each file:

```awk
BEGINFILE { print "Processing:", FILENAME }
ENDFILE { print "Done with:", FILENAME }
```

#### Regular Expression

Match lines containing a pattern:

```awk
/error/ { print }        # Lines containing "error"
/^#/ { next }            # Skip comment lines
/[0-9]+/ { print }       # Lines with numbers
```

#### Expression

Match based on any expression:

```awk
$3 > 100 { print }           # Field 3 greater than 100
NR > 10 { print }            # After line 10
length($0) > 80 { print }    # Long lines
```

#### Negated Pattern

Match lines NOT matching a pattern:

```awk
!/debug/ { print }       # Lines without "debug"
```

#### Range Pattern

Match from start pattern to end pattern:

```awk
/START/,/END/ { print }  # Lines between START and END
NR==5,NR==10 { print }   # Lines 5 through 10
```

#### Compound Patterns

Combine patterns with logical operators:

```awk
/error/ && /critical/ { print }   # Both patterns
/warning/ || /error/ { print }    # Either pattern
```

---

## 5. Actions

### Statement Types

Actions are blocks of statements enclosed in braces:

```awk
{
    statement1
    statement2
    ...
}
```

### Multiple Statements

Statements can be separated by:
- Newlines
- Semicolons

```awk
{ x = 1; y = 2; print x + y }

# Or:
{
    x = 1
    y = 2
    print x + y
}
```

### Empty Action

An empty action prints the entire record:

```awk
/pattern/    # Same as: /pattern/ { print $0 }
```

---

## 6. Variables

### Field Variables

| Variable | Description |
|----------|-------------|
| `$0` | Entire current record |
| `$1, $2, ...` | Individual fields |
| `$NF` | Last field |
| `$(NF-1)` | Second-to-last field |

```awk
{ print $1, $NF }        # First and last fields
{ print $2 + $3 }        # Sum of fields 2 and 3
{ $2 = "NEW"; print }    # Modify field 2
```

### User Variables

Variables are created on first use:

```awk
{ count++ }                      # Numeric (starts at 0)
{ name = "John" }                # String
{ total += $2 }                  # Accumulator
BEGIN { threshold = 100 }        # Initialize in BEGIN
```

### Variable Types

AWK variables are dynamically typed:

```awk
x = 42           # Numeric
x = "hello"      # String
x = "42"         # String that looks numeric
```

Type conversion is automatic:
- String to number: Leading numeric portion, or 0
- Number to string: Formatted by CONVFMT

### Special Variables

See [Section 14](#14-special-variables-reference) for complete list.

Common special variables:

| Variable | Description | Default |
|----------|-------------|---------|
| `FS` | Field separator | whitespace |
| `RS` | Record separator | newline |
| `OFS` | Output field separator | space |
| `ORS` | Output record separator | newline |
| `NR` | Record number (total) | - |
| `NF` | Number of fields | - |
| `FNR` | Record number in file | - |
| `FILENAME` | Current filename | - |

---

## 7. Operators

### Arithmetic Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `x + y` |
| `-` | Subtraction | `x - y` |
| `*` | Multiplication | `x * y` |
| `/` | Division | `x / y` |
| `%` | Modulo | `x % y` |
| `^` | Exponentiation | `x ^ y` |

### Assignment Operators

| Operator | Description | Equivalent |
|----------|-------------|------------|
| `=` | Assignment | `x = y` |
| `+=` | Add and assign | `x = x + y` |
| `-=` | Subtract and assign | `x = x - y` |
| `*=` | Multiply and assign | `x = x * y` |
| `/=` | Divide and assign | `x = x / y` |
| `%=` | Modulo and assign | `x = x % y` |
| `^=` | Power and assign | `x = x ^ y` |

### Increment/Decrement

| Operator | Description |
|----------|-------------|
| `++x` | Pre-increment |
| `x++` | Post-increment |
| `--x` | Pre-decrement |
| `x--` | Post-decrement |

### Comparison Operators

| Operator | Description |
|----------|-------------|
| `<` | Less than |
| `<=` | Less than or equal |
| `>` | Greater than |
| `>=` | Greater than or equal |
| `==` | Equal |
| `!=` | Not equal |

### Logical Operators

| Operator | Description |
|----------|-------------|
| `&&` | Logical AND |
| `\|\|` | Logical OR |
| `!` | Logical NOT |

### String Operators

| Operator | Description | Example |
|----------|-------------|---------|
| (space) | Concatenation | `"Hello" " " "World"` |
| `~` | Regex match | `$1 ~ /pattern/` |
| `!~` | Regex not match | `$1 !~ /pattern/` |

### Other Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `? :` | Ternary | `x > 0 ? "pos" : "neg"` |
| `in` | Array membership | `key in array` |
| `$` | Field access | `$1`, `$(i+1)` |

### Operator Precedence

From highest to lowest:

1. `$` (field access)
2. `^` (exponentiation)
3. `++ --` (increment/decrement)
4. `! - +` (unary)
5. `* / %` (multiplication)
6. `+ -` (addition)
7. (concatenation)
8. `< <= > >= != ==` (comparison)
9. `~ !~` (regex match)
10. `in` (array membership)
11. `&&` (logical AND)
12. `||` (logical OR)
13. `?:` (ternary)
14. `= += -= *=` etc. (assignment)

---

## 8. Control Structures

### if-else

```awk
if (condition) {
    statements
}

if (condition) {
    statements
} else {
    statements
}

if (condition1) {
    statements
} else if (condition2) {
    statements
} else {
    statements
}
```

Example:
```awk
{
    if ($3 > 100) {
        print $1, "high"
    } else if ($3 > 50) {
        print $1, "medium"
    } else {
        print $1, "low"
    }
}
```

### while

```awk
while (condition) {
    statements
}
```

Example:
```awk
BEGIN {
    i = 1
    while (i <= 5) {
        print i
        i++
    }
}
```

### do-while

```awk
do {
    statements
} while (condition)
```

Example:
```awk
BEGIN {
    i = 1
    do {
        print i
        i++
    } while (i <= 5)
}
```

### for

```awk
for (init; condition; increment) {
    statements
}
```

Example:
```awk
BEGIN {
    for (i = 1; i <= 10; i++) {
        print i, i * i
    }
}
```

### for-in (Arrays)

```awk
for (key in array) {
    statements
}
```

Example:
```awk
END {
    for (word in count) {
        print word, count[word]
    }
}
```

### switch-case (gawk)

```awk
switch (expression) {
    case value1:
        statements
        break
    case value2:
        statements
        break
    default:
        statements
}
```

Example:
```awk
{
    switch ($1) {
        case "red":
        case "green":
        case "blue":
            print "Primary color"
            break
        default:
            print "Other color"
    }
}
```

### Control Statements

| Statement | Description |
|-----------|-------------|
| `break` | Exit innermost loop |
| `continue` | Next iteration of loop |
| `next` | Skip to next record |
| `nextfile` | Skip to next file |
| `exit [code]` | Exit program |

---

## 9. Arrays

### Associative Arrays

AWK arrays are associative (indexed by strings):

```awk
# Create/access elements
array["key"] = value
array[1] = "first"
array["name"] = "John"

# Access
print array["key"]
```

### Array Operations

**Check existence:**
```awk
if ("key" in array) {
    print "exists"
}
```

**Delete element:**
```awk
delete array["key"]
```

**Delete entire array:**
```awk
delete array
```

**Iterate:**
```awk
for (key in array) {
    print key, array[key]
}
```

### Multi-Dimensional Arrays

Simulated using SUBSEP:

```awk
# Store
matrix[1, 2] = 10
matrix[row, col] = value

# Access
print matrix[1, 2]

# Check existence
if ((i, j) in matrix) {
    print "exists"
}
```

### Array Examples

**Word frequency:**
```awk
{
    for (i = 1; i <= NF; i++)
        words[$i]++
}
END {
    for (w in words)
        print w, words[w]
}
```

**Group by field:**
```awk
{
    sum[$1] += $2
    count[$1]++
}
END {
    for (key in sum)
        print key, sum[key]/count[key]
}
```

---

## 10. Functions

### User-Defined Functions

```awk
function name(parameters) {
    statements
    return value
}
```

Example:
```awk
function max(a, b) {
    return a > b ? a : b
}

function factorial(n) {
    if (n <= 1) return 1
    return n * factorial(n - 1)
}

BEGIN {
    print max(5, 3)
    print factorial(6)
}
```

### Local Variables

Declare local variables as extra parameters:

```awk
function sum_array(arr,    i, total) {
    total = 0
    for (i in arr)
        total += arr[i]
    return total
}
```

### Array Parameters

Arrays are passed by reference:

```awk
function double_values(arr,    key) {
    for (key in arr)
        arr[key] *= 2
}
```

### Built-in Functions

See [Section 13](#13-built-in-functions-reference) for complete reference.

---

## 11. Input and Output

### print Statement

```awk
print                    # Print $0
print expr               # Print expression
print expr1, expr2       # Print with OFS separator
```

### printf Statement

```awk
printf format, expr1, expr2, ...
```

**Format specifiers:**

| Specifier | Description |
|-----------|-------------|
| `%d`, `%i` | Integer |
| `%f` | Floating-point |
| `%e`, `%E` | Scientific notation |
| `%g`, `%G` | Shorter of %f or %e |
| `%s` | String |
| `%c` | Character |
| `%x`, `%X` | Hexadecimal |
| `%o` | Octal |
| `%%` | Literal percent |

**Modifiers:**

| Modifier | Description |
|----------|-------------|
| `-` | Left justify |
| `+` | Show sign |
| ` ` | Space before positive |
| `0` | Zero padding |
| `width` | Minimum width |
| `.precision` | Decimal places / max string length |

Examples:
```awk
printf "%10s %5d %8.2f\n", name, count, average
printf "%-20s %08d\n", $1, $2
printf "%+.2f%%\n", percent
```

### Output Redirection

**Write to file:**
```awk
print "data" > "file.txt"
```

**Append to file:**
```awk
print "data" >> "file.txt"
```

**Pipe to command:**
```awk
print "data" | "sort"
```

**Coprocess (gawk):**
```awk
print "data" |& "command"
"command" |& getline result
```

### getline

Read input from various sources:

| Form | Description |
|------|-------------|
| `getline` | Read next record from input |
| `getline var` | Read into variable |
| `getline < "file"` | Read from file |
| `getline var < "file"` | Read from file into variable |
| `"cmd" \| getline` | Read from command |
| `"cmd" \| getline var` | Read from command into variable |

Return values:
- `1` = success
- `0` = end of file
- `-1` = error

Example:
```awk
BEGIN {
    while ((getline line < "data.txt") > 0) {
        print line
    }
    close("data.txt")
}
```

### Closing Files

```awk
close("filename")     # Close file
close("command")      # Close pipe
```

---

## 12. Regular Expressions

### Basic Syntax

```awk
/pattern/           # Pattern delimited by slashes
$1 ~ /pattern/      # Field matches pattern
$1 !~ /pattern/     # Field doesn't match pattern
```

### Metacharacters

| Character | Description |
|-----------|-------------|
| `.` | Any single character |
| `*` | Zero or more of previous |
| `+` | One or more of previous |
| `?` | Zero or one of previous |
| `^` | Start of string/line |
| `$` | End of string/line |
| `[...]` | Character class |
| `[^...]` | Negated character class |
| `\|` | Alternation (OR) |
| `(...)` | Grouping |
| `{n}` | Exactly n times |
| `{n,}` | n or more times |
| `{n,m}` | n to m times |

### Character Classes

| Class | Description |
|-------|-------------|
| `[abc]` | a, b, or c |
| `[a-z]` | Lowercase letter |
| `[A-Z]` | Uppercase letter |
| `[0-9]` | Digit |
| `[^0-9]` | Not a digit |
| `[a-zA-Z]` | Any letter |
| `[a-zA-Z0-9]` | Alphanumeric |

### Escape Sequences

| Sequence | Description |
|----------|-------------|
| `\\` | Literal backslash |
| `\.` | Literal dot |
| `\*` | Literal asterisk |
| `\n` | Newline |
| `\t` | Tab |

### Examples

```awk
/^$/                    # Empty line
/^#/                    # Comment line
/[0-9]+/                # Contains number
/^[A-Z][a-z]+$/         # Capitalized word
/error\|warning/i       # Error or warning (case-insensitive)
/^.{80,}$/              # Lines 80+ characters
```

---

## 13. Built-in Functions Reference

### String Functions

#### length(s)
Returns string length, or array length if s is an array.
```awk
print length("hello")     # 5
print length(array)       # Number of elements
```

#### substr(s, start [, len])
Returns substring starting at position start (1-indexed).
```awk
print substr("hello", 2, 3)    # "ell"
print substr("hello", 3)       # "llo"
```

#### index(s, t)
Returns position of t in s, or 0 if not found.
```awk
print index("hello", "ll")     # 3
```

#### split(s, a [, fs [, seps]])
Splits string s into array a using separator fs.
```awk
n = split("a,b,c", arr, ",")   # n=3, arr[1]="a", etc.
```

#### sub(regex, replacement [, target])
Replaces first match of regex in target (default $0).
```awk
sub(/old/, "new")              # In $0
sub(/old/, "new", str)         # In str
```

#### gsub(regex, replacement [, target])
Replaces all matches of regex in target.
```awk
gsub(/old/, "new")             # Replace all in $0
n = gsub(/a/, "A", str)        # Returns count
```

#### gensub(regex, replacement, how [, target]) (gawk)
Replace with backreference support.
```awk
# how: "g" for global, or number for nth occurrence
gensub(/(.)(.)/, "\\2\\1", "g", "abcd")   # "badc"
```

#### match(s, regex [, arr])
Returns position of match, sets RSTART and RLENGTH.
```awk
if (match(s, /[0-9]+/)) {
    print substr(s, RSTART, RLENGTH)
}

# With capture groups (gawk):
match(s, /([0-9]+)-([0-9]+)/, groups)
print groups[1], groups[2]
```

#### tolower(s)
Returns lowercase version of s.
```awk
print tolower("Hello")         # "hello"
```

#### toupper(s)
Returns uppercase version of s.
```awk
print toupper("Hello")         # "HELLO"
```

#### sprintf(format, expr, ...)
Returns formatted string.
```awk
s = sprintf("%05d", 42)        # "00042"
```

#### strtonum(s) (gawk)
Converts string to number, recognizing hex (0x) and octal (0).
```awk
print strtonum("0x1F")         # 31
```

### Math Functions

| Function | Description |
|----------|-------------|
| `sin(x)` | Sine (radians) |
| `cos(x)` | Cosine |
| `tan(x)` | Tangent |
| `atan2(y, x)` | Arc tangent of y/x |
| `asin(x)` | Arc sine |
| `acos(x)` | Arc cosine |
| `sinh(x)` | Hyperbolic sine |
| `cosh(x)` | Hyperbolic cosine |
| `tanh(x)` | Hyperbolic tangent |
| `exp(x)` | e^x |
| `log(x)` | Natural logarithm |
| `log10(x)` | Base-10 logarithm |
| `log2(x)` | Base-2 logarithm |
| `sqrt(x)` | Square root |
| `int(x)` | Truncate to integer |
| `ceil(x)` | Round up |
| `floor(x)` | Round down |
| `round(x)` | Round to nearest |
| `abs(x)` | Absolute value |
| `rand()` | Random number [0,1) |
| `srand([seed])` | Seed random generator |

### Array Functions (gawk)

#### asort(src [, dest [, how]])
Sorts array values.
```awk
n = asort(arr)                 # Sort in place
n = asort(arr, sorted)         # Sort into new array
```

#### asorti(src [, dest [, how]])
Sorts array indices.
```awk
n = asorti(arr, keys)          # Get sorted keys
```

#### isarray(x)
Returns 1 if x is an array.
```awk
if (isarray(arr)) print "is array"
```

### I/O Functions

#### close(file)
Closes file or pipe.
```awk
close("output.txt")
close("sort")
```

#### fflush([file])
Flushes output buffer.
```awk
fflush()                       # Flush all
fflush("output.txt")           # Flush specific file
```

#### system(command)
Executes shell command, returns exit status.
```awk
status = system("ls -l")
```

### Time Functions

#### systime()
Returns current Unix timestamp.
```awk
now = systime()
```

#### mktime(datespec)
Converts date specification to timestamp.
```awk
ts = mktime("2024 06 15 12 30 00")
```

#### strftime(format [, timestamp])
Formats timestamp as string.
```awk
print strftime("%Y-%m-%d %H:%M:%S", systime())
```

Common format codes:
| Code | Description |
|------|-------------|
| `%Y` | 4-digit year |
| `%m` | Month (01-12) |
| `%d` | Day (01-31) |
| `%H` | Hour (00-23) |
| `%M` | Minute (00-59) |
| `%S` | Second (00-59) |
| `%A` | Weekday name |
| `%B` | Month name |

### Bitwise Functions (gawk)

| Function | Description |
|----------|-------------|
| `and(x, y)` | Bitwise AND |
| `or(x, y)` | Bitwise OR |
| `xor(x, y)` | Bitwise XOR |
| `lshift(x, n)` | Left shift |
| `rshift(x, n)` | Right shift |
| `compl(x)` | Bitwise complement |

### Type Functions (gawk)

| Function | Description |
|----------|-------------|
| `typeof(x)` | Returns type name |
| `isarray(x)` | Test if array |
| `mkbool(x)` | Convert to boolean |

---

## 14. Special Variables Reference

### Input/Output Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `FS` | Input field separator | whitespace |
| `RS` | Input record separator | newline |
| `OFS` | Output field separator | space |
| `ORS` | Output record separator | newline |
| `CONVFMT` | Number-to-string format | `"%.6g"` |
| `OFMT` | Output number format | `"%.6g"` |
| `SUBSEP` | Array subscript separator | `"\034"` |

### Record Information

| Variable | Description |
|----------|-------------|
| `NR` | Total records read |
| `NF` | Fields in current record |
| `FNR` | Records in current file |
| `$0` | Entire current record |
| `$1..$NF` | Individual fields |
| `FILENAME` | Current input filename |

### Regex Match Results

| Variable | Description |
|----------|-------------|
| `RSTART` | Start position of match |
| `RLENGTH` | Length of match |

### Program Arguments

| Variable | Description |
|----------|-------------|
| `ARGC` | Number of arguments |
| `ARGV` | Array of arguments |
| `ENVIRON` | Environment variables |

### gawk Extensions

| Variable | Description |
|----------|-------------|
| `IGNORECASE` | Case-insensitive matching (0/1) |
| `RT` | Actual record terminator text |
| `FPAT` | Field pattern (alternative to FS) |
| `SYMTAB` | Symbol table access |
| `FUNCTAB` | Function table |
| `PROCINFO` | Process information |

---

## 15. gawk Extensions

This interpreter supports many GNU AWK extensions:

### BEGINFILE and ENDFILE

```awk
BEGINFILE {
    print "Starting:", FILENAME
}
ENDFILE {
    print "Finished:", FILENAME, "with", FNR, "records"
}
```

### switch Statement

```awk
switch (value) {
    case "a":
    case "b":
        print "Letter"
        break
    case 1:
        print "Number"
        break
    default:
        print "Other"
}
```

### Namespaces

```awk
@namespace "mylib"

function helper() {
    return 42
}

@namespace "awk"

BEGIN {
    print mylib::helper()
}
```

### Coprocesses

Bidirectional communication with external processes:

```awk
BEGIN {
    cmd = "sort"
    print "banana" |& cmd
    print "apple" |& cmd
    close(cmd, "to")

    while ((cmd |& getline line) > 0)
        print line
    close(cmd)
}
```

### Indirect Function Calls

```awk
function add(a, b) { return a + b }
function mul(a, b) { return a * b }

BEGIN {
    op = "add"
    print @op(3, 4)    # Calls add(3, 4)
}
```

### FPAT (Field Pattern)

Define fields by content rather than separator:

```awk
BEGIN {
    FPAT = "([^,]+)|(\"[^\"]+\")"
}
{
    # Correctly handles: field1,"field,with,commas",field3
    for (i = 1; i <= NF; i++)
        print i ": " $i
}
```

### IGNORECASE

```awk
BEGIN { IGNORECASE = 1 }
/error/ { print }    # Matches ERROR, Error, error, etc.
```

---

## 16. Examples

### Line Numbering

```awk
{ print NR ": " $0 }
```

### Remove Blank Lines

```awk
NF > 0 { print }
# Or: /./
# Or: !/^$/
```

### Sum a Column

```awk
{ sum += $2 }
END { print "Sum:", sum }
```

### Average

```awk
{ sum += $2; count++ }
END { print "Average:", sum/count }
```

### Find Maximum

```awk
NR == 1 || $2 > max { max = $2; maxline = $0 }
END { print "Max:", max, "Line:", maxline }
```

### Word Frequency

```awk
{
    for (i = 1; i <= NF; i++) {
        word = tolower($i)
        gsub(/[^a-z]/, "", word)
        if (word) words[word]++
    }
}
END {
    for (w in words)
        printf "%4d %s\n", words[w], w
}
```

### Remove Duplicates (Preserve Order)

```awk
!seen[$0]++ { print }
```

### CSV to Fixed Width

```awk
BEGIN { FS = ","; OFS = "" }
{
    printf "%-20s %-10s %8.2f\n", $1, $2, $3
}
```

### Log Analysis

```awk
/ERROR/ { errors++ }
/WARNING/ { warnings++ }
END {
    print "Errors:", errors
    print "Warnings:", warnings
}
```

### Group Statistics

```awk
{
    group = $1
    value = $2
    sum[group] += value
    count[group]++
    if (!(group in min) || value < min[group]) min[group] = value
    if (!(group in max) || value > max[group]) max[group] = value
}
END {
    printf "%-10s %8s %8s %8s %8s\n", "Group", "Count", "Sum", "Min", "Max"
    for (g in sum)
        printf "%-10s %8d %8.2f %8.2f %8.2f\n", g, count[g], sum[g], min[g], max[g]
}
```

---

## 17. Troubleshooting

### Common Errors

**"field separator is empty"**
```bash
# Wrong: Empty separator
awk -F'' '...'

# Right: Use single character
awk -F',' '...'
```

**Division by zero**
```awk
# Check before dividing
{ if (count > 0) print total/count }
```

**Uninitialized variable**
```awk
# Variables default to "" or 0
# Use BEGIN to initialize if needed
BEGIN { count = 0 }
```

**String vs. numeric comparison**
```awk
# "10" < "9" is TRUE (string comparison)
# Force numeric: add 0
{ if ($1 + 0 > 9) print }
```

### Debugging Tips

**Print variable values:**
```awk
{ print "DEBUG: x=" x " y=" y > "/dev/stderr" }
```

**Print field information:**
```awk
{
    print "NF=" NF
    for (i = 1; i <= NF; i++)
        print "  $" i "=[" $i "]"
}
```

**Check field separator:**
```awk
BEGIN { print "FS=[" FS "]" }
```

### Platform Differences

**Line endings:**
- Windows: CRLF (`\r\n`)
- Unix/Mac: LF (`\n`)

Handle with:
```awk
{ gsub(/\r$/, "") }  # Remove trailing CR
```

**Path separators:**
- Windows: `\`
- Unix: `/`

---

## Quick Reference Card

### One-Liners

```bash
# Print lines matching pattern
awk '/pattern/' file

# Print specific fields
awk '{ print $1, $3 }' file

# Print lines where field > value
awk '$2 > 100' file

# Sum column
awk '{ s += $1 } END { print s }' file

# Count lines
awk 'END { print NR }' file

# Print line numbers
awk '{ print NR, $0 }' file

# Remove duplicates
awk '!seen[$0]++' file

# Print last field
awk '{ print $NF }' file

# Print lines 10-20
awk 'NR >= 10 && NR <= 20' file

# Replace text
awk '{ gsub(/old/, "new"); print }' file
```

---

*End of User Manual*
