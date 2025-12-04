# Benchmark: String concatenation
# Tests: ConcatExpr pre-allocation
BEGIN { FS = "," }
{
    result = $3 " " $4 " (ID: " $1 ") - " $9 ": " $10
    all = all result "\n"
}
END { print "Concatenated", NR, "lines, total length:", length(all) }
