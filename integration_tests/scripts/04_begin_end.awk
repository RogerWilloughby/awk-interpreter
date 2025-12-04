# Test: BEGIN and END blocks with NR
BEGIN {
    print "=== Processing Numbers ==="
    sum = 0
}
{
    for (i = 1; i <= NF; i++) sum += $i
}
END {
    print "=== Summary ==="
    print "Total lines:", NR
    print "Grand total:", sum
}
