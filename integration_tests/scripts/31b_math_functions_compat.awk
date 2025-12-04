# Test: Math functions (gawk-compatible version)
# Note: abs() and pow() are extensions - using standard alternatives here
BEGIN {
    print "sin(0):", sin(0)
    print "cos(0):", cos(0)
    print "sqrt(16):", sqrt(16)
    print "int(3.7):", int(3.7)
    print "int(-3.7):", int(-3.7)
    # abs(-5) using standard AWK: x < 0 ? -x : x
    x = -5
    print "abs(-5):", (x < 0 ? -x : x)
    print "log(2.718281828):", int(log(2.718281828) * 1000) / 1000
    print "exp(1):", int(exp(1) * 1000) / 1000
    # pow(2, 10) using standard AWK: 2^10
    print "pow(2, 10):", 2^10
}
