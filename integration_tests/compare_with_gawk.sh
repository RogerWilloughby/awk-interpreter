#!/bin/bash
# AWK vs GAWK Comparison Test Runner
# Runs the same scripts against both implementations and compares output

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
AWK_EXE="${1:-../build/Release/awk.exe}"
GAWK_EXE="${2:-gawk}"

SCRIPTS_DIR="$SCRIPT_DIR/scripts"
INPUT_DIR="$SCRIPT_DIR/input"
RESULTS_DIR="$SCRIPT_DIR/comparison_results"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

MATCH=0
DIFFER=0
SKIP=0

# Create results directory
mkdir -p "$RESULTS_DIR"

echo "========================================"
echo "AWK vs GAWK Comparison Test"
echo "========================================"
echo "Custom AWK: $AWK_EXE"
echo "GNU AWK:    $GAWK_EXE"
echo ""

# Check if both executables exist
if [[ ! -f "$AWK_EXE" ]] && ! command -v "$AWK_EXE" &> /dev/null; then
    echo -e "${RED}Error: Custom AWK not found: $AWK_EXE${NC}"
    exit 1
fi

if ! command -v "$GAWK_EXE" &> /dev/null; then
    echo -e "${RED}Error: GAWK not found: $GAWK_EXE${NC}"
    exit 1
fi

echo "GAWK version: $($GAWK_EXE --version | head -1)"
echo ""

# Test configuration: script -> input file mapping
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

# Scripts that use gawk-specific features or have known differences
declare -A KNOWN_DIFFERENCES=(
    ["31_math_functions.awk"]="uses abs()/pow() which are extensions not in gawk"
    ["40_associative_array.awk"]="hash iteration order varies (sorted comparison)"
    ["43_asort.awk"]="hash iteration order varies (sorted comparison)"
    ["50_csv_processing.awk"]="hash iteration order varies (sorted comparison)"
    ["51_log_analysis.awk"]="hash iteration order varies (sorted comparison)"
    ["52_word_count.awk"]="hash iteration order varies (sorted comparison)"
    ["53_namespace.awk"]="@namespace is gawk 5.0+ feature"
)

# Scripts where we should compare sorted output (due to hash iteration order)
declare -A COMPARE_SORTED=(
    ["40_associative_array.awk"]=1
    ["43_asort.awk"]=1
    ["50_csv_processing.awk"]=1
    ["51_log_analysis.awk"]=1
    ["52_word_count.awk"]=1
)

normalize_output() {
    # Normalize output for comparison:
    # - Remove trailing whitespace
    # - Normalize line endings
    # - Sort lines for tests with hash iteration (optional, controlled by $2)
    local output="$1"
    local sort_lines="${2:-false}"

    if [[ "$sort_lines" == "true" ]]; then
        echo "$output" | sed 's/[[:space:]]*$//' | tr -d '\r' | sort
    else
        echo "$output" | sed 's/[[:space:]]*$//' | tr -d '\r'
    fi
}

run_comparison() {
    local script="$1"
    local input="$2"
    local test_name="${script%.awk}"
    local script_path="$SCRIPTS_DIR/$script"

    printf "Comparing %-30s ... " "$test_name"

    if [[ ! -f "$script_path" ]]; then
        echo -e "${YELLOW}SKIP${NC} (script not found)"
        ((SKIP++))
        return
    fi

    # Check for known differences
    if [[ -n "${KNOWN_DIFFERENCES[$script]}" ]]; then
        echo -e "${CYAN}INFO${NC} (${KNOWN_DIFFERENCES[$script]})"
    fi

    # Run custom AWK
    local awk_output
    local awk_exit
    if [[ "$input" == "none" ]]; then
        awk_output=$(echo "" | "$AWK_EXE" -f "$script_path" 2>&1)
        awk_exit=$?
    else
        if [[ ! -f "$INPUT_DIR/$input" ]]; then
            echo -e "${YELLOW}SKIP${NC} (input file not found)"
            ((SKIP++))
            return
        fi
        awk_output=$("$AWK_EXE" -f "$script_path" "$INPUT_DIR/$input" 2>&1)
        awk_exit=$?
    fi

    # Run GAWK
    local gawk_output
    local gawk_exit
    if [[ "$input" == "none" ]]; then
        gawk_output=$(echo "" | "$GAWK_EXE" -f "$script_path" 2>&1)
        gawk_exit=$?
    else
        gawk_output=$("$GAWK_EXE" -f "$script_path" "$INPUT_DIR/$input" 2>&1)
        gawk_exit=$?
    fi

    # Normalize outputs
    local awk_normalized=$(normalize_output "$awk_output")
    local gawk_normalized=$(normalize_output "$gawk_output")

    # Save outputs to files for detailed comparison
    echo "$awk_output" > "$RESULTS_DIR/${test_name}_awk.txt"
    echo "$gawk_output" > "$RESULTS_DIR/${test_name}_gawk.txt"

    # For tests with hash iteration order differences, compare sorted output
    local use_sorted="${COMPARE_SORTED[$script]:-0}"
    if [[ "$use_sorted" == "1" ]]; then
        awk_normalized=$(echo "$awk_normalized" | sort)
        gawk_normalized=$(echo "$gawk_normalized" | sort)
    fi

    # Compare
    if [[ "$awk_normalized" == "$gawk_normalized" ]]; then
        echo -e "${GREEN}MATCH${NC}"
        ((MATCH++))
    else
        echo -e "${RED}DIFFER${NC}"
        ((DIFFER++))

        # Show differences
        echo "  Exit codes: AWK=$awk_exit, GAWK=$gawk_exit"
        echo "  AWK output (first 5 lines):"
        echo "$awk_output" | head -5 | sed 's/^/    /'
        if [[ $(echo "$awk_output" | wc -l) -gt 5 ]]; then
            echo "    ..."
        fi
        echo "  GAWK output (first 5 lines):"
        echo "$gawk_output" | head -5 | sed 's/^/    /'
        if [[ $(echo "$gawk_output" | wc -l) -gt 5 ]]; then
            echo "    ..."
        fi
        echo "  (Full output saved to $RESULTS_DIR/${test_name}_*.txt)"
    fi
}

# Run all comparisons
for script in "$SCRIPTS_DIR"/*.awk; do
    script_name=$(basename "$script")
    input="${TEST_INPUT[$script_name]:-none}"
    run_comparison "$script_name" "$input"
done

echo ""
echo "========================================"
echo -e "Results: ${GREEN}$MATCH match${NC}, ${RED}$DIFFER differ${NC}, ${YELLOW}$SKIP skipped${NC}"
echo "========================================"
echo ""
echo "Detailed results saved to: $RESULTS_DIR/"

# Summary
echo ""
echo "Comparison Analysis:"
echo "===================="
echo "$MATCH tests produce identical output to gawk"
if [[ $DIFFER -gt 0 ]]; then
    echo "$DIFFER tests have differences (see notes below)"
fi
echo ""
echo "Notes:"
echo "  - Tests 40, 43, 50, 51, 52 use sorted comparison (hash iteration order)"
echo "  - Test 31 uses abs()/pow() which are custom extensions not in gawk"
echo "  - Test 53 uses @namespace which requires gawk 5.0+"
echo ""

if [[ $DIFFER -gt 0 ]]; then
    exit 1
fi
exit 0
