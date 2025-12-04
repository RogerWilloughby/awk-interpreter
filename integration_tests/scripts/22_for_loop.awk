# Test: for loop - multiplication table
BEGIN {
    for (i = 1; i <= 5; i++) {
        line = ""
        for (j = 1; j <= 5; j++) {
            line = line sprintf("%4d", i * j)
        }
        print line
    }
}
