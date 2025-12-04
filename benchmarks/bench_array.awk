# Benchmark: Multi-dimensional array operations
# Tests: SUBSEP caching, make_array_key() pre-allocation
BEGIN { FS = "," }
{
    # Multi-dimensional array access (uses SUBSEP)
    data[$2, $3, $4]++
    byUser[$2]++
    byAge[$5]++
}
END {
    print "Unique user+name combinations:", length(data)
    print "Unique users:", length(byUser)
    print "Unique ages:", length(byAge)
}
