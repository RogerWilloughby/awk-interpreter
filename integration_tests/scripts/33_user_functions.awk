# Test: User-defined functions
function max(a, b) {
    return a > b ? a : b
}

function min(a, b) {
    return a < b ? a : b
}

function factorial(n) {
    if (n <= 1) return 1
    return n * factorial(n - 1)
}

function fibonacci(n) {
    if (n <= 1) return n
    return fibonacci(n - 1) + fibonacci(n - 2)
}

BEGIN {
    print "max(5, 3):", max(5, 3)
    print "min(5, 3):", min(5, 3)
    print "factorial(6):", factorial(6)
    print "fibonacci(10):", fibonacci(10)
}
