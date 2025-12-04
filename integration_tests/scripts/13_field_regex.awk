# Test: Field-specific regex matching
BEGIN { FS="|" }
$2 ~ /Engineering/ { print $1, "- Engineer, salary:", $3 }
