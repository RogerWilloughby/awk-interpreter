# Test: delete from array
BEGIN {
    a["x"] = 1
    a["y"] = 2
    a["z"] = 3
    print "Before delete:"
    for (k in a) print "  a[" k "] =", a[k]

    delete a["y"]
    print "After delete a[y]:"
    for (k in a) print "  a[" k "] =", a[k]

    print "Check if y exists:", ("y" in a) ? "yes" : "no"
    print "Check if x exists:", ("x" in a) ? "yes" : "no"
}
