# Test: Math functions
BEGIN {
    print "sin(0):", sin(0)
    print "cos(0):", cos(0)
    print "sqrt(16):", sqrt(16)
    print "int(3.7):", int(3.7)
    print "int(-3.7):", int(-3.7)
    print "abs(-5):", abs(-5)
    print "log(2.718281828):", int(log(2.718281828) * 1000) / 1000
    print "exp(1):", int(exp(1) * 1000) / 1000
    print "pow(2, 10):", pow(2, 10)
}
