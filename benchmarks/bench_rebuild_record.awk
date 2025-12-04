# Benchmark: Record rebuilding with custom OFS
# Tests: OFS caching, rebuild_record() pre-allocation
BEGIN { FS = ","; OFS = "|" }
{ $1 = $1 }  # Force record rebuild
END { print "Processed", NR, "records" }
