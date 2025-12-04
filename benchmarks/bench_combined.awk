# Benchmark: Combined operations (realistic workload)
# Tests all optimizations together
BEGIN { FS = ","; OFS = "\t" }
{
    # Field access (FS caching)
    user = $2
    name = $3 " " $4
    age = $5
    amount = $6

    # Multi-dimensional array (SUBSEP caching, array key optimization)
    userStats[user, "count"]++
    userStats[user, "total"] += amount

    # String concatenation (ConcatExpr optimization)
    summary = user ": " name " (age " age ") - $" amount

    # Record rebuild (OFS caching, rebuild_record optimization)
    if (NR % 100 == 0) {
        $1 = $1  # Force rebuild
    }
}
END {
    count = 0
    for (key in userStats) count++
    print "Processed", NR, "records"
    print "User stat entries:", count
}
