# Test: Calculate sum of each row
{
    sum = 0
    for (i = 1; i <= NF; i++) sum += $i
    print "Row", NR, "sum:", sum
}
