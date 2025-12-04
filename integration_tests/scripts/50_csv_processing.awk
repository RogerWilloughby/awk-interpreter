# Test: CSV processing with calculations
BEGIN {
    FS = ","
    print "Product Report"
    print "=============="
}
NR == 1 { next }  # Skip header
{
    product = $1
    qty = $2
    price = $3
    category = $4
    total = qty * price
    grand_total += total
    cat_total[category] += total
    printf "%-12s: %3d x $%7.2f = $%10.2f\n", product, qty, price, total
}
END {
    print "=============="
    print "By Category:"
    for (cat in cat_total)
        printf "  %-12s: $%10.2f\n", cat, cat_total[cat]
    print "=============="
    printf "Grand Total: $%10.2f\n", grand_total
}
