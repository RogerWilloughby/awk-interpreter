# Test: String functions
BEGIN {
    s = "Hello, World!"
    print "Original:", s
    print "Length:", length(s)
    print "Upper:", toupper(s)
    print "Lower:", tolower(s)
    print "Substr(1,5):", substr(s, 1, 5)
    print "Index of 'World':", index(s, "World")

    t = s
    sub(/World/, "AWK", t)
    print "After sub:", t

    u = "foo bar foo baz foo"
    gsub(/foo/, "XXX", u)
    print "After gsub:", u
}
