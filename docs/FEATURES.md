# AWK Interpreter Features

This document provides a complete list of implemented features and gawk compatibility.

## Core AWK Language

### Patterns

| Pattern Type | Syntax | Status |
|--------------|--------|--------|
| BEGIN | `BEGIN { ... }` | Supported |
| END | `END { ... }` | Supported |
| BEGINFILE | `BEGINFILE { ... }` | Supported (gawk) |
| ENDFILE | `ENDFILE { ... }` | Supported (gawk) |
| Regex | `/pattern/ { ... }` | Supported |
| Expression | `expr { ... }` | Supported |
| Range | `/start/,/end/ { ... }` | Supported |
| Negated | `!/pattern/ { ... }` | Supported |

### Operators

| Category | Operators | Status |
|----------|-----------|--------|
| Arithmetic | `+ - * / % ^` | Supported |
| Assignment | `= += -= *= /= %= ^=` | Supported |
| Comparison | `< <= > >= == !=` | Supported |
| Logical | `&& \|\| !` | Supported |
| Regex Match | `~ !~` | Supported |
| Increment | `++ --` (pre and post) | Supported |
| Ternary | `? :` | Supported |
| String Concat | (space) | Supported |
| Field Access | `$` | Supported |
| Array Membership | `in` | Supported |

### Control Flow

| Statement | Syntax | Status |
|-----------|--------|--------|
| if/else | `if (cond) stmt [else stmt]` | Supported |
| while | `while (cond) stmt` | Supported |
| do-while | `do stmt while (cond)` | Supported |
| for | `for (init; cond; incr) stmt` | Supported |
| for-in | `for (key in array) stmt` | Supported |
| switch/case | `switch (expr) { case val: ... }` | Supported (gawk) |
| break | `break` | Supported |
| continue | `continue` | Supported |
| next | `next` | Supported |
| nextfile | `nextfile` | Supported |
| exit | `exit [code]` | Supported |
| return | `return [expr]` | Supported |

### Arrays

| Feature | Status |
|---------|--------|
| Associative arrays | Supported |
| Multi-dimensional arrays (SUBSEP) | Supported |
| Array membership test (`in`) | Supported |
| Delete element | Supported |
| Delete entire array | Supported |
| for-in iteration | Supported |

### Functions

| Feature | Status |
|---------|--------|
| User-defined functions | Supported |
| Local variables | Supported |
| Recursion | Supported |
| Array parameters (by reference) | Supported |
| Default parameter values | Supported |
| Indirect function calls (@var) | Supported (gawk) |

### I/O

| Feature | Syntax | Status |
|---------|--------|--------|
| Print | `print [args]` | Supported |
| Printf | `printf format, args` | Supported |
| Output to file | `print > "file"` | Supported |
| Append to file | `print >> "file"` | Supported |
| Output to pipe | `print \| "cmd"` | Supported |
| Getline (8 variants) | `getline [var] [< file]` | Supported |
| Coprocess | `print \|& "cmd"` | Supported (gawk) |
| Close | `close(file)` | Supported |
| Flush | `fflush([file])` | Supported |

---

## Built-in Functions

### String Functions

| Function | Description | Status |
|----------|-------------|--------|
| `length(s)` | String length | Supported |
| `length(a)` | Array length | Supported |
| `substr(s, start [, len])` | Substring | Supported |
| `index(s, t)` | Find substring | Supported |
| `split(s, a [, fs [, seps]])` | Split string | Supported |
| `sub(re, repl [, target])` | Replace first match | Supported |
| `gsub(re, repl [, target])` | Replace all matches | Supported |
| `gensub(re, repl, how [, target])` | Replace with backrefs | Supported (gawk) |
| `match(s, re [, arr])` | Regex match with groups | Supported |
| `tolower(s)` | Convert to lowercase | Supported |
| `toupper(s)` | Convert to uppercase | Supported |
| `sprintf(fmt, args...)` | Format string | Supported |
| `strtonum(s)` | String to number | Supported (gawk) |
| `patsplit(s, a, re [, seps])` | Split by pattern | Supported (gawk) |
| `ord(s)` | Character to ASCII | Supported |
| `chr(n)` | ASCII to character | Supported |

### Math Functions

| Function | Description | Status |
|----------|-------------|--------|
| `sin(x)` | Sine | Supported |
| `cos(x)` | Cosine | Supported |
| `tan(x)` | Tangent | Supported |
| `atan2(y, x)` | Arc tangent | Supported |
| `asin(x)` | Arc sine | Supported |
| `acos(x)` | Arc cosine | Supported |
| `sinh(x)` | Hyperbolic sine | Supported |
| `cosh(x)` | Hyperbolic cosine | Supported |
| `tanh(x)` | Hyperbolic tangent | Supported |
| `exp(x)` | Exponential | Supported |
| `log(x)` | Natural logarithm | Supported |
| `log10(x)` | Base-10 logarithm | Supported |
| `log2(x)` | Base-2 logarithm | Supported |
| `sqrt(x)` | Square root | Supported |
| `int(x)` | Truncate to integer | Supported |
| `ceil(x)` | Ceiling | Supported |
| `floor(x)` | Floor | Supported |
| `round(x)` | Round to nearest | Supported |
| `abs(x)` | Absolute value | Supported |
| `fmod(x, y)` | Floating-point modulo | Supported |
| `pow(x, y)` | Power | Supported |
| `rand()` | Random number [0,1) | Supported |
| `srand([seed])` | Seed random generator | Supported |
| `min(x, y, ...)` | Minimum | Supported |
| `max(x, y, ...)` | Maximum | Supported |

### Array Functions

| Function | Description | Status |
|----------|-------------|--------|
| `asort(src [, dest [, how]])` | Sort array values | Supported (gawk) |
| `asorti(src [, dest [, how]])` | Sort array indices | Supported (gawk) |
| `isarray(x)` | Test if array | Supported (gawk) |

### I/O Functions

| Function | Description | Status |
|----------|-------------|--------|
| `getline` | Read next record | Supported |
| `close(file)` | Close file/pipe | Supported |
| `fflush([file])` | Flush output | Supported |
| `system(cmd)` | Execute command | Supported |

### Time Functions

| Function | Description | Status |
|----------|-------------|--------|
| `systime()` | Current Unix timestamp | Supported |
| `mktime(datespec)` | Parse date to timestamp | Supported |
| `strftime(fmt [, timestamp])` | Format timestamp | Supported |

### Bitwise Functions (gawk)

| Function | Description | Status |
|----------|-------------|--------|
| `and(x, y)` | Bitwise AND | Supported |
| `or(x, y)` | Bitwise OR | Supported |
| `xor(x, y)` | Bitwise XOR | Supported |
| `lshift(x, n)` | Left shift | Supported |
| `rshift(x, n)` | Right shift | Supported |
| `compl(x)` | Bitwise complement | Supported |

### Type Functions (gawk)

| Function | Description | Status |
|----------|-------------|--------|
| `typeof(x)` | Get type name | Supported |
| `isarray(x)` | Test if array | Supported |
| `mkbool(x)` | Convert to boolean | Supported |

### Internationalization (gawk)

| Function | Description | Status |
|----------|-------------|--------|
| `dcgettext(domain, string, category)` | Translate string | Supported |
| `dcngettext(domain, s1, s2, n, cat)` | Translate plural | Supported |
| `bindtextdomain(domain, directory)` | Set locale directory | Supported |

---

## Special Variables

### Standard Variables

| Variable | Description | Status |
|----------|-------------|--------|
| `FS` | Input field separator | Supported |
| `RS` | Input record separator | Supported |
| `OFS` | Output field separator | Supported |
| `ORS` | Output record separator | Supported |
| `NR` | Total record number | Supported |
| `NF` | Number of fields | Supported |
| `FNR` | Record number in current file | Supported |
| `FILENAME` | Current filename | Supported |
| `SUBSEP` | Array subscript separator | Supported |
| `CONVFMT` | Number-to-string format | Supported |
| `OFMT` | Output number format | Supported |
| `ARGC` | Argument count | Supported |
| `ARGV` | Argument array | Supported |
| `ENVIRON` | Environment variables | Supported |
| `RSTART` | Match start position | Supported |
| `RLENGTH` | Match length | Supported |

### gawk Extension Variables

| Variable | Description | Status |
|----------|-------------|--------|
| `IGNORECASE` | Case-insensitive matching | Supported |
| `RT` | Record terminator (actual text) | Supported |
| `FPAT` | Field pattern (instead of FS) | Supported |
| `TEXTDOMAIN` | i18n text domain | Supported |
| `SYMTAB` | Symbol table access | Supported |
| `FUNCTAB` | Function table | Supported |
| `PROCINFO` | Process information | Supported |

---

## gawk Extensions

| Extension | Status |
|-----------|--------|
| BEGINFILE/ENDFILE patterns | Supported |
| switch/case/default statement | Supported |
| Coprocesses (\|& operator) | Supported |
| @namespace directive | Supported |
| Indirect function calls (@var) | Supported |
| Internationalization (i18n with .mo files) | Supported |
| Bitwise operations | Supported |
| Multi-char RS (regex) | Supported |
| FPAT field parsing | Supported |
| Capture groups in match() | Supported |

---

## Not Implemented

These advanced gawk features are not implemented:

| Feature | Description |
|---------|-------------|
| Arbitrary Precision (-M) | MPFR/GMP big numbers |
| Dynamic Extensions (@load) | Loading .so/.dll plugins |
| Debugger (-D) | Interactive debugging |
| Profiler (--profile) | Performance profiling |
| Pretty Printer (-o) | Source reformatting |
| Network I/O (/inet/) | TCP/UDP sockets |
| Persistent Memory (@persist) | gawk 5.2+ feature |

---

## Printf Format Specifiers

| Specifier | Description | Status |
|-----------|-------------|--------|
| `%d`, `%i` | Signed integer | Supported |
| `%u` | Unsigned integer | Supported |
| `%o` | Octal | Supported |
| `%x`, `%X` | Hexadecimal | Supported |
| `%e`, `%E` | Scientific notation | Supported |
| `%f`, `%F` | Floating-point | Supported |
| `%g`, `%G` | General format | Supported |
| `%c` | Character | Supported |
| `%s` | String | Supported |
| `%%` | Literal percent | Supported |
| Width/precision | `%10.2f` | Supported |
| Dynamic width | `%*d` | Supported |
| Flags | `-`, `+`, ` `, `#`, `0` | Supported |

---

## Escape Sequences

| Sequence | Description | Status |
|----------|-------------|--------|
| `\\` | Backslash | Supported |
| `\n` | Newline | Supported |
| `\t` | Tab | Supported |
| `\r` | Carriage return | Supported |
| `\b` | Backspace | Supported |
| `\f` | Form feed | Supported |
| `\"` | Double quote | Supported |
| `\/` | Forward slash (in regex) | Supported |
| `\xHH` | Hex character | Supported |
| `\OOO` | Octal character | Supported |
