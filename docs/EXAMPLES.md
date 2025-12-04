# AWK Examples

This document provides practical AWK script examples demonstrating various features.

## Basic Examples

### Hello World

```awk
BEGIN { print "Hello, World!" }
```

### Print All Lines

```awk
{ print }
```

Or equivalently:
```awk
{ print $0 }
```

### Print Line Numbers

```awk
{ print NR, $0 }
```

### Print Specific Fields

```awk
# Print first and third fields
{ print $1, $3 }

# Print last field
{ print $NF }

# Print second-to-last field
{ print $(NF-1) }
```

---

## Pattern Matching

### Regex Matching

```awk
# Print lines containing "error"
/error/ { print }

# Case-insensitive (gawk)
BEGIN { IGNORECASE = 1 }
/error/ { print }
```

### Negated Pattern

```awk
# Print lines NOT containing "debug"
!/debug/ { print }
```

### Field Matching

```awk
# Print lines where field 3 matches a pattern
$3 ~ /pattern/ { print }

# Print lines where field 2 does NOT match
$2 !~ /^[0-9]+$/ { print "Non-numeric:", $0 }
```

### Range Pattern

```awk
# Print lines from START to END markers
/START/,/END/ { print }

# Print lines 10 through 20
NR == 10, NR == 20 { print }
```

### Expression Pattern

```awk
# Print lines where field 2 > 100
$2 > 100 { print }

# Compound conditions
$2 > 50 && $3 == "active" { print }
```

---

## Field Processing

### CSV Processing

```awk
BEGIN { FS = "," }
{
    name = $1
    age = $2
    city = $3
    print name, "is", age, "years old, lives in", city
}
```

### Reformat Output

```awk
BEGIN {
    FS = ","
    OFS = "\t"
}
{ print $3, $1, $2 }  # Reorder columns
```

### Calculate New Fields

```awk
BEGIN { FS = ","; OFS = "," }
{
    total = $2 * $3    # qty * price
    print $0, total    # Append calculated field
}
```

### Sum a Column

```awk
{ sum += $2 }
END { print "Total:", sum }
```

---

## Control Flow

### If-Else

```awk
{
    if ($2 > 100)
        print $1, "high"
    else if ($2 > 50)
        print $1, "medium"
    else
        print $1, "low"
}
```

### While Loop

```awk
BEGIN {
    i = 1
    while (i <= 5) {
        print "Number:", i
        i++
    }
}
```

### For Loop

```awk
BEGIN {
    for (i = 1; i <= 10; i++)
        print i, i*i
}
```

### For-In Loop (Arrays)

```awk
END {
    for (key in count)
        print key, count[key]
}
```

### Switch Statement (gawk)

```awk
{
    switch ($1) {
        case "red":
            print "Primary color"
            break
        case "green":
            print "Primary color"
            break
        case "blue":
            print "Primary color"
            break
        default:
            print "Other color"
    }
}
```

### Next Statement

```awk
# Skip blank lines
/^$/ { next }
{ print }
```

---

## User-Defined Functions

### Basic Functions

```awk
function max(a, b) {
    return a > b ? a : b
}

function min(a, b) {
    return a < b ? a : b
}

BEGIN {
    print "max(5, 3):", max(5, 3)
    print "min(5, 3):", min(5, 3)
}
```

### Recursive Functions

```awk
function factorial(n) {
    if (n <= 1) return 1
    return n * factorial(n - 1)
}

function fibonacci(n) {
    if (n <= 1) return n
    return fibonacci(n - 1) + fibonacci(n - 2)
}

BEGIN {
    print "factorial(6):", factorial(6)
    print "fibonacci(10):", fibonacci(10)
}
```

### Functions with Local Variables

```awk
# Local variables declared as extra parameters
function sum_array(arr,    i, total) {
    total = 0
    for (i in arr)
        total += arr[i]
    return total
}
```

---

## Arrays

### Associative Arrays

```awk
{
    count[$1]++
}
END {
    for (item in count)
        print item, count[item]
}
```

### Multi-Dimensional Arrays

```awk
{
    # Store data by row and column
    data[NR, 1] = $1
    data[NR, 2] = $2
    data[NR, 3] = $3
}
END {
    # Access stored data
    for (r = 1; r <= NR; r++)
        print data[r, 1], data[r, 2], data[r, 3]
}
```

### Array Membership Test

```awk
BEGIN {
    fruits["apple"] = 1
    fruits["banana"] = 1

    if ("apple" in fruits)
        print "Apple is a fruit"

    if (!("carrot" in fruits))
        print "Carrot is not in the list"
}
```

### Delete Array Elements

```awk
BEGIN {
    a[1] = "one"
    a[2] = "two"
    a[3] = "three"

    delete a[2]  # Delete single element

    for (i in a)
        print i, a[i]

    delete a     # Delete entire array
}
```

### Sorting Arrays (gawk)

```awk
BEGIN {
    a[1] = "cherry"
    a[2] = "apple"
    a[3] = "banana"

    # Sort values
    n = asort(a)
    for (i = 1; i <= n; i++)
        print a[i]
}
```

---

## String Functions

### String Manipulation

```awk
BEGIN {
    s = "Hello, World!"

    print "Length:", length(s)
    print "Upper:", toupper(s)
    print "Lower:", tolower(s)
    print "Substr:", substr(s, 1, 5)
    print "Index:", index(s, "World")
}
```

### Search and Replace

```awk
{
    # Replace first match
    sub(/old/, "new")

    # Replace all matches
    gsub(/pattern/, "replacement")

    print
}
```

### gensub (gawk - with backreferences)

```awk
BEGIN {
    s = "foo123bar456baz"

    # Replace with captured groups
    result = gensub(/([a-z]+)([0-9]+)/, "[\\1:\\2]", "g", s)
    print result  # [foo:123][bar:456]baz
}
```

### Split String

```awk
BEGIN {
    s = "apple,banana,cherry"
    n = split(s, arr, ",")

    for (i = 1; i <= n; i++)
        print i, arr[i]
}
```

### Match with Capture Groups

```awk
BEGIN {
    s = "user@example.com"
    if (match(s, /([^@]+)@(.+)/, groups)) {
        print "User:", groups[1]
        print "Domain:", groups[2]
    }
}
```

---

## Math Functions

```awk
BEGIN {
    print "sin(0.5):", sin(0.5)
    print "cos(0.5):", cos(0.5)
    print "sqrt(2):", sqrt(2)
    print "exp(1):", exp(1)
    print "log(10):", log(10)
    print "int(3.7):", int(3.7)
    print "rand():", rand()

    # Power
    print "2^10:", 2^10

    # Random integer 1-100
    srand()
    print "Random 1-100:", int(rand() * 100) + 1
}
```

---

## Time Functions

```awk
BEGIN {
    # Current timestamp
    now = systime()
    print "Timestamp:", now

    # Format timestamp
    print "Formatted:", strftime("%Y-%m-%d %H:%M:%S", now)

    # Parse date string
    ts = mktime("2024 06 15 12 30 00")
    print "Parsed:", ts
}
```

---

## I/O Operations

### Read from File

```awk
BEGIN {
    while ((getline line < "data.txt") > 0)
        print line
    close("data.txt")
}
```

### Write to File

```awk
{
    print $0 > "output.txt"
}
END {
    close("output.txt")
}
```

### Pipe to Command

```awk
{
    print | "sort"
}
END {
    close("sort")
}
```

### Read from Command

```awk
BEGIN {
    while (("ls -l" | getline line) > 0)
        print line
    close("ls -l")
}
```

---

## Real-World Examples

### CSV Report Generator

```awk
# Usage: awk -f report.awk data.csv
BEGIN {
    FS = ","
    print "Product Report"
    print "=============="
}
NR == 1 { next }  # Skip header
{
    product = $1
    qty = $2
    price = $3
    category = $4
    total = qty * price
    grand_total += total
    cat_total[category] += total
    printf "%-12s: %3d x $%7.2f = $%10.2f\n", product, qty, price, total
}
END {
    print "=============="
    print "By Category:"
    for (cat in cat_total)
        printf "  %-12s: $%10.2f\n", cat, cat_total[cat]
    print "=============="
    printf "Grand Total: $%10.2f\n", grand_total
}
```

### Log File Analyzer

```awk
# Usage: awk -f log_analysis.awk server.log
BEGIN {
    print "Log Analysis Report"
    print "==================="
}
{
    level = $3
    count[level]++
    if (level == "ERROR") {
        errors[NR] = $0
    }
}
END {
    print "Message counts:"
    for (lvl in count)
        printf "  %-8s: %d\n", lvl, count[lvl]
    print ""
    print "Error details:"
    for (line in errors)
        print "  Line " line ": " errors[line]
}
```

### Word Frequency Counter

```awk
# Usage: awk -f wordfreq.awk document.txt
{
    for (i = 1; i <= NF; i++) {
        word = tolower($i)
        gsub(/[^a-z]/, "", word)
        if (length(word) > 0)
            words[word]++
    }
}
END {
    print "Word Frequency (words appearing more than once):"
    for (w in words)
        if (words[w] > 1)
            printf "  %-15s: %d\n", w, words[w]
}
```

### Duplicate Line Remover

```awk
# Removes duplicate lines while preserving order
!seen[$0]++ { print }
```

### Column Statistics

```awk
# Calculate min, max, sum, avg for column 2
BEGIN { min = ""; max = "" }
{
    sum += $2
    count++
    if (min == "" || $2 < min) min = $2
    if (max == "" || $2 > max) max = $2
}
END {
    print "Count:", count
    print "Sum:", sum
    print "Min:", min
    print "Max:", max
    print "Avg:", sum/count
}
```

### JSON-like Output

```awk
BEGIN { FS = ","; print "[" }
NR > 1 { if (NR > 2) print "," }
NR > 1 {
    printf "  {\"name\": \"%s\", \"age\": %s, \"city\": \"%s\"}", $1, $2, $3
}
END { print "\n]" }
```

---

## gawk Extensions

### Namespaces

```awk
@namespace "math"
function square(x) { return x * x }
function cube(x) { return x * x * x }

@namespace "string"
function repeat(s, n,    result, i) {
    result = ""
    for (i = 1; i <= n; i++) result = result s
    return result
}

@namespace "awk"
BEGIN {
    print "math::square(5) =", math::square(5)
    print "math::cube(3) =", math::cube(3)
    print "string::repeat(ab, 4) =", string::repeat("ab", 4)
}
```

### Bitwise Operations

```awk
BEGIN {
    a = 12    # 1100 in binary
    b = 10    # 1010 in binary

    print "and(12, 10):", and(a, b)    # 8 (1000)
    print "or(12, 10):", or(a, b)      # 14 (1110)
    print "xor(12, 10):", xor(a, b)    # 6 (0110)
    print "lshift(1, 4):", lshift(1, 4) # 16
    print "rshift(16, 2):", rshift(16, 2) # 4
}
```

### FPAT (Field Pattern)

```awk
# Handle CSV with quoted fields containing commas
BEGIN {
    FPAT = "([^,]+)|(\"[^\"]+\")"
}
{
    for (i = 1; i <= NF; i++)
        print i": "$i
}
```

### Coprocesses

```awk
BEGIN {
    cmd = "sort"
    print "banana" |& cmd
    print "apple" |& cmd
    print "cherry" |& cmd
    close(cmd, "to")

    while ((cmd |& getline line) > 0)
        print line
    close(cmd)
}
```

---

## One-Liners

```bash
# Print lines longer than 80 characters
awk 'length > 80' file.txt

# Print lines 5-10
awk 'NR >= 5 && NR <= 10' file.txt

# Print every other line
awk 'NR % 2 == 1' file.txt

# Reverse fields
awk '{ for (i=NF; i>0; i--) printf "%s ", $i; print "" }' file.txt

# Add line numbers
awk '{ print NR ": " $0 }' file.txt

# Sum all numbers in file
awk '{ for (i=1; i<=NF; i++) sum+=$i } END { print sum }' file.txt

# Print unique lines (in order)
awk '!seen[$0]++' file.txt

# Count lines in each file
awk 'FNR==1 { if (NR>1) print file, count; file=FILENAME; count=0 } { count++ } END { print file, count }' *.txt

# Extract email addresses
awk -F'[<>]' '/@/ { print $2 }' file.txt

# Print between two patterns
awk '/START/,/END/' file.txt
```
