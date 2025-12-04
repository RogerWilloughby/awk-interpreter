# AWK Interpreter Usage Guide

This guide covers command-line options, usage patterns, and practical tips.

## Command-Line Syntax

```
awk [options] 'program' [file ...]
awk [options] -f progfile [file ...]
```

## Options

| Option | Description |
|--------|-------------|
| `-F fs` | Set field separator to `fs` |
| `-v var=value` | Assign value to variable before execution |
| `-f progfile` | Read AWK program from file |
| `-h`, `--help` | Show help message |
| `--version` | Show version information |

### Field Separator (-F)

Set the input field separator (FS). Can be a single character or a regular expression.

```bash
# Single character
awk -F, '{ print $1, $2 }' data.csv

# Colon separator
awk -F: '{ print $1 }' /etc/passwd

# Multiple characters (regex)
awk -F'[,;]' '{ print $1 }' data.txt

# Tab separator
awk -F'\t' '{ print $1 }' data.tsv

# Any whitespace (default)
awk '{ print $1 }' data.txt
```

The `-F` option can be written with or without a space:
```bash
awk -F, '...'    # Both forms
awk -F , '...'   # are equivalent
```

### Variable Assignment (-v)

Pre-assign values to variables before the program runs (before BEGIN).

```bash
# Set a string variable
awk -v name="John" 'BEGIN { print "Hello", name }'

# Set a numeric variable
awk -v threshold=100 '$1 > threshold { print }' data.txt

# Multiple variables
awk -v min=10 -v max=100 '$1 >= min && $1 <= max' data.txt

# Pass shell variables
awk -v user="$USER" 'BEGIN { print "User:", user }'
```

### Program File (-f)

Read the AWK program from a file instead of the command line.

```bash
# Single program file
awk -f script.awk data.txt

# Program file with field separator
awk -F, -f process.awk data.csv

# Program file with variables
awk -v debug=1 -f script.awk data.txt
```

---

## Input Sources

### Files

```bash
# Single file
awk '{ print }' file.txt

# Multiple files
awk '{ print FILENAME, $0 }' file1.txt file2.txt file3.txt

# Glob patterns (shell expansion)
awk '{ print }' *.txt
```

### Standard Input

```bash
# Pipe data to awk
cat data.txt | awk '{ print $1 }'

# Heredoc
awk '{ print NR, $0 }' <<EOF
line one
line two
EOF

# Interactive (Ctrl+D to end)
awk '{ print "You entered:", $0 }'
```

### No Input (BEGIN only)

```bash
# Programs that don't need input
awk 'BEGIN { print "Hello, World!" }'

# Calculations
awk 'BEGIN { print 2^10 }'

# Generate sequences
awk 'BEGIN { for (i=1; i<=10; i++) print i }'
```

---

## Output Control

### Print Statement

```bash
# Default output (full record)
awk '/pattern/ { print }' file.txt

# Specific fields
awk '{ print $1, $3 }' file.txt

# Custom separator (OFS)
awk 'BEGIN { OFS="," } { print $1, $2, $3 }' file.txt

# No separator (concatenation)
awk '{ print $1 $2 $3 }' file.txt
```

### Printf Statement

```bash
# Formatted output
awk '{ printf "%10s %5d\n", $1, $2 }' file.txt

# Aligned columns
awk '{ printf "%-20s %8.2f\n", $1, $2 }' file.txt

# No automatic newline
awk '{ printf "%s", $1 }' file.txt
```

### Output Redirection

```bash
# Write to file
awk '{ print > "output.txt" }' input.txt

# Append to file
awk '{ print >> "log.txt" }' input.txt

# Pipe to command
awk '{ print | "sort" }' input.txt

# Different outputs for different records
awk '{ print > ($1 ".txt") }' input.txt
```

---

## Common Usage Patterns

### Field Processing

```bash
# Print specific columns
awk '{ print $1, $3, $5 }' file.txt

# Reorder columns
awk '{ print $3, $1, $2 }' file.txt

# Add columns
awk '{ print $0, $2+$3 }' file.txt

# Number of fields
awk '{ print NF, $0 }' file.txt
```

### Filtering

```bash
# Pattern match
awk '/error/' logfile.txt

# Field comparison
awk '$3 > 100' data.txt

# Multiple conditions
awk '$2 == "active" && $3 > 50' data.txt

# Negation
awk '!/debug/' logfile.txt
```

### Transformations

```bash
# Change field values
awk '{ $2 = $2 * 1.1; print }' prices.txt

# Add new fields
awk '{ $(NF+1) = $2 + $3; print }' data.txt

# Case conversion
awk '{ print toupper($1) }' file.txt
```

### Aggregation

```bash
# Sum a column
awk '{ sum += $2 } END { print sum }' data.txt

# Count records
awk 'END { print NR }' file.txt

# Average
awk '{ sum += $2; count++ } END { print sum/count }' data.txt

# Min/Max
awk 'NR==1 || $2>max { max=$2 } END { print max }' data.txt
```

### Grouping

```bash
# Group by field
awk '{ count[$1]++ } END { for (k in count) print k, count[k] }' file.txt

# Sum by group
awk '{ sum[$1] += $2 } END { for (k in sum) print k, sum[k] }' file.txt
```

---

## Special Variables

### Input Control

| Variable | Description | Default |
|----------|-------------|---------|
| `FS` | Input field separator | whitespace |
| `RS` | Input record separator | newline |
| `FPAT` | Field pattern (gawk) | - |
| `IGNORECASE` | Case-insensitive matching (gawk) | 0 |

### Record Information

| Variable | Description |
|----------|-------------|
| `NR` | Total records read |
| `FNR` | Records in current file |
| `NF` | Fields in current record |
| `$0` | Entire current record |
| `$1..$NF` | Individual fields |
| `FILENAME` | Current input filename |

### Output Control

| Variable | Description | Default |
|----------|-------------|---------|
| `OFS` | Output field separator | space |
| `ORS` | Output record separator | newline |
| `OFMT` | Output number format | `%.6g` |
| `CONVFMT` | Number-to-string format | `%.6g` |

### Match Results

| Variable | Description |
|----------|-------------|
| `RSTART` | Start of regex match |
| `RLENGTH` | Length of regex match |
| `RT` | Matched record terminator (gawk) |

### Program Arguments

| Variable | Description |
|----------|-------------|
| `ARGC` | Number of arguments |
| `ARGV` | Array of arguments |
| `ENVIRON` | Environment variables |

---

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Error (syntax error, file not found, runtime error) |
| N | Exit code from `exit N` statement |

```bash
# Check exit status
awk '/pattern/ { found=1 } END { exit !found }' file.txt
if [ $? -eq 0 ]; then
    echo "Pattern found"
fi
```

---

## Tips and Best Practices

### Quoting on Different Platforms

**Linux/macOS (bash):**
```bash
awk '{ print "Hello" }' file.txt
awk -F',' '{ print $1 }' file.csv
```

**Windows (cmd.exe):**
```batch
awk "{ print \"Hello\" }" file.txt
awk -F"," "{ print $1 }" file.csv
```

**Windows (PowerShell):**
```powershell
awk '{ print \"Hello\" }' file.txt
awk -F',' '{ print $1 }' file.csv
```

### Performance Tips

1. **Use field separator in -F option** rather than setting FS in BEGIN
2. **Avoid unnecessary field access** - accessing fields costs time
3. **Use `next` to skip records early** rather than nested conditions
4. **Pre-compile regex** by using constant patterns like `/pattern/`

### Debugging Tips

```bash
# Print all variables
awk '{ for (v in SYMTAB) print v, SYMTAB[v] }' file.txt

# Trace execution
awk '{ print "NR=" NR " NF=" NF " $0=" $0 }' file.txt

# Check field splitting
awk '{ print "Fields:"; for (i=1; i<=NF; i++) print i": ["$i"]" }' file.txt
```

### Common Pitfalls

1. **String vs Number comparison**
   ```bash
   # "10" < "9" is true (string comparison)
   # 10 < 9 is false (numeric comparison)
   awk '$1 + 0 > 9 { print }' file.txt  # Force numeric
   ```

2. **Uninitialized variables**
   ```bash
   # Variables default to "" (string) or 0 (numeric)
   awk '{ count[$1]++ } END { for (k in count) print k, count[k] }' file.txt
   ```

3. **Field separator regex**
   ```bash
   # Dot needs escaping in regex
   awk -F'\\.' '{ print $1 }' file.txt
   ```

---

## Examples

### Quick Reference

```bash
# Print lines matching pattern
awk '/pattern/' file.txt

# Print field from CSV
awk -F, '{ print $2 }' data.csv

# Sum column 3
awk '{ sum += $3 } END { print sum }' data.txt

# Count lines
awk 'END { print NR }' file.txt

# Print lines 10-20
awk 'NR >= 10 && NR <= 20' file.txt

# Remove duplicate lines
awk '!seen[$0]++' file.txt

# Print last field
awk '{ print $NF }' file.txt

# Replace text
awk '{ gsub(/old/, "new"); print }' file.txt

# Join lines
awk '{ printf "%s ", $0 } END { print "" }' file.txt
```

For more examples, see [EXAMPLES.md](EXAMPLES.md).
