#!/bin/bash
# AWK Integration Test Runner

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
AWK_EXE="${1:-../build/Release/awk.exe}"

SCRIPTS_DIR="$SCRIPT_DIR/scripts"
INPUT_DIR="$SCRIPT_DIR/input"
EXPECTED_DIR="$SCRIPT_DIR/expected"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASS=0
FAIL=0
SKIP=0

echo "========================================"
echo "AWK Integration Test Suite"
echo "========================================"
echo "AWK executable: $AWK_EXE"
echo ""

# Test configuration: script -> input file mapping
# Format: script_name:input_file (use "none" for no input)
declare -A TEST_INPUT=(
    ["01_print_all.awk"]="numbers.txt"
    ["02_field_access.awk"]="employees.txt"
    ["03_printf_formats.awk"]="employees.txt"
    ["04_begin_end.awk"]="numbers.txt"
    ["05_row_sum.awk"]="numbers.txt"
    ["10_regex_match.awk"]="log.txt"
    ["11_negated_regex.awk"]="log.txt"
    ["12_range_pattern.awk"]="log.txt"
    ["13_field_regex.awk"]="employees.txt"
    ["14_expression_pattern.awk"]="employees.txt"
    ["20_if_else.awk"]="employees.txt"
    ["21_while_loop.awk"]="none"
    ["22_for_loop.awk"]="none"
    ["23_next_statement.awk"]="log.txt"
    ["24_switch.awk"]="log.txt"
    ["30_string_functions.awk"]="none"
    ["31_math_functions.awk"]="none"
    ["31b_math_functions_compat.awk"]="none"
    ["32_split_function.awk"]="none"
    ["33_user_functions.awk"]="none"
    ["34_sprintf.awk"]="none"
    ["40_associative_array.awk"]="employees.txt"
    ["41_delete_array.awk"]="none"
    ["42_multidim_array.awk"]="none"
    ["43_asort.awk"]="none"
    ["50_csv_processing.awk"]="csv_data.txt"
    ["51_log_analysis.awk"]="log.txt"
    ["52_word_count.awk"]="text.txt"
    ["53_namespace.awk"]="none"
    ["54_gensub.awk"]="none"
    ["55_coprocess.awk"]="none"
    ["56_i18n.awk"]="none"
)

run_test() {
    local script="$1"
    local input="$2"
    local expected="$3"
    local test_name="${script%.awk}"

    printf "Testing %-30s ... " "$test_name"

    if [[ ! -f "$SCRIPTS_DIR/$script" ]]; then
        echo -e "${YELLOW}SKIP${NC} (script not found)"
        ((SKIP++))
        return
    fi

    if [[ ! -f "$expected" ]]; then
        echo -e "${YELLOW}SKIP${NC} (expected output not found)"
        ((SKIP++))
        return
    fi

    # Run the test
    local actual
    if [[ "$input" == "none" ]]; then
        # Pipe empty input to avoid waiting on stdin
        actual=$(echo "" | "$AWK_EXE" -f "$SCRIPTS_DIR/$script" 2>&1)
    else
        if [[ ! -f "$INPUT_DIR/$input" ]]; then
            echo -e "${YELLOW}SKIP${NC} (input file not found)"
            ((SKIP++))
            return
        fi
        actual=$("$AWK_EXE" -f "$SCRIPTS_DIR/$script" "$INPUT_DIR/$input" 2>&1)
    fi

    local expected_content
    expected_content=$(cat "$expected")

    if [[ "$actual" == "$expected_content" ]]; then
        echo -e "${GREEN}PASS${NC}"
        ((PASS++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAIL++))
        echo "  Expected:"
        echo "$expected_content" | head -5 | sed 's/^/    /'
        if [[ $(echo "$expected_content" | wc -l) -gt 5 ]]; then
            echo "    ..."
        fi
        echo "  Actual:"
        echo "$actual" | head -5 | sed 's/^/    /'
        if [[ $(echo "$actual" | wc -l) -gt 5 ]]; then
            echo "    ..."
        fi
    fi
}

# Run all tests
for script in "$SCRIPTS_DIR"/*.awk; do
    script_name=$(basename "$script")
    input="${TEST_INPUT[$script_name]:-none}"
    expected="$EXPECTED_DIR/${script_name%.awk}.txt"
    run_test "$script_name" "$input" "$expected"
done

echo ""
echo "========================================"
echo "Results: ${GREEN}$PASS passed${NC}, ${RED}$FAIL failed${NC}, ${YELLOW}$SKIP skipped${NC}"
echo "========================================"

if [[ $FAIL -gt 0 ]]; then
    exit 1
fi
exit 0
