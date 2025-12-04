# Test: Range pattern (from first ERROR to next INFO)
/ERROR/,/INFO/ { print NR": "$0 }
