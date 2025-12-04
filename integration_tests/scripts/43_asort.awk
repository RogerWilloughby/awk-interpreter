# Test: asort and asorti functions
BEGIN {
    a["z"] = "cherry"
    a["a"] = "apple"
    a["m"] = "banana"

    print "Original array:"
    for (k in a) print "  a[" k "] =", a[k]

    n = asorti(a, sorted_keys)
    print "Sorted keys (asorti):"
    for (i = 1; i <= n; i++) print "  " i ":", sorted_keys[i]

    # Recreate for asort test
    b[1] = "cherry"
    b[2] = "apple"
    b[3] = "banana"

    n = asort(b)
    print "Sorted values (asort):"
    for (i = 1; i <= n; i++) print "  " i ":", b[i]
}
