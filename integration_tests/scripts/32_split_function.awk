# Test: split function
BEGIN {
    str = "apple:banana:cherry:date"
    n = split(str, arr, ":")
    print "Split into", n, "parts:"
    for (i = 1; i <= n; i++) {
        print "  arr["i"] =", arr[i]
    }
}
