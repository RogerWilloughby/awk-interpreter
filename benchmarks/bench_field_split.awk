# Benchmark: Field splitting with custom FS
# Tests: FS caching, field parsing
BEGIN { FS = "," }
{ sum += $5 }
END { print "Sum of column 5:", sum }
