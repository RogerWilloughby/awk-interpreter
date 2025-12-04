# AWK Integration Test Suite

This directory contains integration tests for the AWK interpreter.

## Directory Structure

```
integration_tests/
├── scripts/        # AWK test scripts (.awk files)
├── input/          # Input data files
├── expected/       # Expected output files
├── run_tests.sh    # Test runner for Linux/macOS
├── run_tests.bat   # Test runner for Windows
└── README.md       # This file
```

## Running Tests

### Windows
```batch
cd integration_tests
run_tests.bat
```

Or specify a custom AWK executable:
```batch
run_tests.bat C:\path\to\awk.exe
```

### Linux/macOS
```bash
cd integration_tests
chmod +x run_tests.sh
./run_tests.sh
```

Or specify a custom AWK executable:
```bash
./run_tests.sh /path/to/awk
```

## Test Categories

| Prefix | Category | Description |
|--------|----------|-------------|
| 01-05 | Basic I/O | Print, field access, printf, BEGIN/END |
| 10-14 | Patterns | Regex, negation, ranges, expressions |
| 20-24 | Control Flow | if/else, loops, next, switch |
| 30-34 | Functions | String, math, split, user-defined, sprintf |
| 40-43 | Arrays | Associative, delete, multidimensional, sort |
| 50-54 | Advanced | CSV processing, log analysis, namespaces |

## Test Files

### Input Files
- `employees.txt` - Employee data (pipe-delimited)
- `numbers.txt` - Numeric data for calculations
- `log.txt` - Server log entries
- `csv_data.txt` - Product inventory (CSV)
- `text.txt` - Sample text for word processing

### Adding New Tests

1. Create script: `scripts/XX_test_name.awk`
2. Create expected output: `expected/XX_test_name.txt`
3. If input needed, add to `input/` and update test runner

## Naming Convention

- Scripts: `XX_description.awk` where XX is category number
- Expected: `XX_description.txt` (same name, .txt extension)
- Test names should be descriptive and lowercase with underscores
