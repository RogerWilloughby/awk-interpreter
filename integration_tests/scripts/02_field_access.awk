# Test: Field access and NF
BEGIN { FS="|" }
{ print $1, "works in", $2, "- Salary:", $3 }
