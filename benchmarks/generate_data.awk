# Generate benchmark test data
# Usage: awk -f generate_data.awk -v lines=100000

BEGIN {
    if (lines == 0) lines = 100000

    # Generate CSV-like data with multiple fields
    for (i = 1; i <= lines; i++) {
        printf "%d,user%d,John,Doe,%d,%0.2f,active,2024-01-%02d,item%d,description for item %d\n", \
            i, i % 1000, 20 + (i % 50), (i * 1.5) + 0.99, (i % 28) + 1, i % 100, i
    }
}
