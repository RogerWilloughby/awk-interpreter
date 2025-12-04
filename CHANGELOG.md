# Changelog

All notable changes to the AWK Interpreter project are documented in this file.

## [1.0.0] - 2024

### Initial Release

A complete AWK interpreter with full POSIX AWK compliance and extensive gawk extensions.

#### Core AWK Features
- All POSIX AWK patterns: BEGIN, END, regex, expression, range
- Complete operator set: arithmetic, comparison, logical, regex match
- Control flow: if/else, while, do-while, for, for-in, switch/case
- Associative arrays with multi-dimensional support (SUBSEP)
- User-defined functions with recursion and local variables
- All 8 getline variants
- I/O redirection: files, pipes, append

#### Built-in Functions (50+)

**String Functions:**
- `length`, `substr`, `index`, `split`, `sub`, `gsub`, `gensub`
- `match` (with capture groups), `tolower`, `toupper`, `sprintf`
- `strtonum`, `patsplit`, `ord`, `chr`

**Math Functions:**
- Trigonometric: `sin`, `cos`, `tan`, `atan2`, `asin`, `acos`
- Hyperbolic: `sinh`, `cosh`, `tanh`
- Logarithmic: `log`, `log10`, `log2`, `exp`
- Arithmetic: `sqrt`, `int`, `ceil`, `floor`, `round`, `abs`, `fmod`, `pow`
- Random: `rand`, `srand`
- Comparison: `min`, `max`

**Array Functions:**
- `asort`, `asorti`, `isarray`

**I/O Functions:**
- `getline`, `close`, `fflush`, `system`

**Time Functions:**
- `systime`, `mktime`, `strftime`

**Bitwise Functions:**
- `and`, `or`, `xor`, `lshift`, `rshift`, `compl`

**Type Functions:**
- `typeof`, `isarray`, `mkbool`

#### gawk Extensions
- `BEGINFILE` / `ENDFILE` patterns
- `switch` / `case` / `default` statement
- Coprocesses (`|&` operator)
- `@namespace` directive for function namespaces
- `@include` directive for file inclusion
- Indirect function calls (`@varname(args)`)
- `IGNORECASE` variable for case-insensitive matching
- `RT` variable (record terminator)
- `FPAT` variable (field pattern)
- Multi-character `RS` (regex record separator)
- Internationalization functions: `dcgettext`, `dcngettext`, `bindtextdomain`

#### Special Variables
- Standard: `FS`, `RS`, `OFS`, `ORS`, `NR`, `NF`, `FNR`, `FILENAME`
- Standard: `SUBSEP`, `CONVFMT`, `OFMT`, `ARGC`, `ARGV`, `ENVIRON`
- Standard: `RSTART`, `RLENGTH`
- gawk: `IGNORECASE`, `RT`, `FPAT`, `TEXTDOMAIN`
- gawk: `SYMTAB`, `FUNCTAB`, `PROCINFO`

#### Printf Format Specifiers
- Integer: `%d`, `%i`, `%u`, `%o`, `%x`, `%X`
- Floating-point: `%e`, `%E`, `%f`, `%F`, `%g`, `%G`
- String/char: `%s`, `%c`
- Width, precision, and flags: `-`, `+`, ` `, `#`, `0`
- Dynamic width: `%*d`

#### Platform Support
- Windows (MSVC 2019+)
- Linux (GCC 8+, Clang 7+)
- macOS (Xcode 10+, Clang 7+)

#### Test Coverage
- 632 unit tests passing
- 32 integration tests passing
- gawk compatibility: 31/32 tests produce identical output

#### Performance Features
- Regex caching (LRU, 64 entries)
- Special variable caching (FS, RS, OFS, SUBSEP)
- Lazy field parsing
- Optimized string concatenation

### Known Limitations

The following advanced gawk features are not implemented:

- Arbitrary precision arithmetic (`-M` / MPFR)
- Dynamic extensions (`@load`)
- Interactive debugger (`-D`)
- Profiler (`--profile`)
- Pretty printer (`-o`)
- Network I/O (`/inet/`)
- Persistent memory (`@persist`)

---

## Future Plans

Potential improvements for future versions:

- Performance optimization for string concatenation
- Additional gawk compatibility improvements
- Extended error messages with source location
- Optional strict POSIX mode
