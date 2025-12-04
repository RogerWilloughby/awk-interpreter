# Test: Coprocess (two-way pipe) feature - reading from coprocess
# Note: Full bidirectional pipes (print |& cmd then cmd |& getline)
# may have platform-specific issues on Windows
BEGIN {
    # Test 1: Simple coprocess read
    print "Coprocess read test:"
    "echo hello" |& getline result1
    sub(/[ \r]+$/, "", result1)  # Trim trailing whitespace
    print "  echo result:", result1

    # Test 2: Multi-line coprocess read
    print "Multi-line coprocess:"
    cmd = "echo line1 && echo line2 && echo line3"
    while ((cmd |& getline line) > 0) {
        sub(/[ \r]+$/, "", line)  # Trim trailing whitespace
        print "  " line
    }
    close(cmd)

    # Test 3: Command with arguments
    print "Command with args:"
    "echo test output" |& getline result2
    sub(/[ \r]+$/, "", result2)  # Trim trailing whitespace
    print "  got:", result2
}
