# Test: sprintf function
BEGIN {
    print sprintf("Integer: %d", 42)
    print sprintf("Float: %.2f", 3.14159)
    print sprintf("String: %s", "hello")
    print sprintf("Padded: |%10s|", "test")
    print sprintf("Left: |%-10s|", "test")
    print sprintf("Hex: %x", 255)
    print sprintf("Octal: %o", 64)
    print sprintf("Multiple: %s is %d years old", "Alice", 30)
}
