# Test: gensub function (gawk extension)
BEGIN {
    s = "hello world hello"

    # Replace first occurrence
    print "Original:", s
    print "gensub first:", gensub(/hello/, "HELLO", 1, s)
    print "gensub all:", gensub(/hello/, "HELLO", "g", s)

    # With backreferences
    t = "foo123bar456baz"
    print "Numbers:", gensub(/([a-z]+)([0-9]+)/, "[\\1:\\2]", "g", t)
}
