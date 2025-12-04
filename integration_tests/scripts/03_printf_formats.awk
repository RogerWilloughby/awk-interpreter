# Test: printf with various formats
BEGIN { FS="|" }
{ printf "%-20s | %-12s | $%10.2f\n", $1, $2, $3 }
