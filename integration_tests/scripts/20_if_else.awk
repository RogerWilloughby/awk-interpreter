# Test: if-else statements
BEGIN { FS="|" }
{
    if ($3 >= 80000)
        level = "Senior"
    else if ($3 >= 65000)
        level = "Mid"
    else
        level = "Junior"
    print $1, "-", level
}
