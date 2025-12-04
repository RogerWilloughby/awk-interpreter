# Test: next statement - skip DEBUG lines
{
    if ($3 == "DEBUG") next
    print $3, "-", $0
}
