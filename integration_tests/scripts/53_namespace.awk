# Test: Namespace feature (gawk extension)
@namespace "math"
function square(x) { return x * x }
function cube(x) { return x * x * x }

@namespace "string"
function repeat(s, n,    result, i) {
    result = ""
    for (i = 1; i <= n; i++) result = result s
    return result
}

@namespace "awk"
BEGIN {
    print "Namespace test:"
    print "  math::square(5) =", math::square(5)
    print "  math::cube(3) =", math::cube(3)
    print "  string::repeat(ab, 4) =", string::repeat("ab", 4)
}
