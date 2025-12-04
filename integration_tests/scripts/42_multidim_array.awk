# Test: Multi-dimensional arrays
BEGIN {
    # Create a 3x3 matrix
    for (i = 1; i <= 3; i++)
        for (j = 1; j <= 3; j++)
            matrix[i,j] = i * j

    print "Matrix:"
    for (i = 1; i <= 3; i++) {
        printf "  "
        for (j = 1; j <= 3; j++)
            printf "%3d", matrix[i,j]
        print ""
    }

    print "SUBSEP:", length(SUBSEP), "chars"
}
