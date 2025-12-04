# Test: Log file analysis
BEGIN {
    print "Log Analysis Report"
    print "==================="
}
{
    level = $3
    count[level]++
    if (level == "ERROR") {
        errors[NR] = $0
    }
}
END {
    print "Message counts:"
    for (lvl in count)
        printf "  %-8s: %d\n", lvl, count[lvl]
    print ""
    print "Error details:"
    for (line in errors)
        print "  Line " line ": " errors[line]
}
