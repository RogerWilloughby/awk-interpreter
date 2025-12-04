# Test: while loop - factorial
BEGIN {
    for (n = 1; n <= 7; n++) {
        fact = 1
        i = n
        while (i > 1) {
            fact *= i
            i--
        }
        print n"! =", fact
    }
}
