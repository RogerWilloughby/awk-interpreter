# Test: Expression pattern (salary > 70000)
BEGIN { FS="|" }
$3 > 70000 { print $1, "earns", $3 }
