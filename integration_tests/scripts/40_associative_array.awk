# Test: Associative arrays - count by department
BEGIN { FS="|" }
{ dept_count[$2]++; dept_salary[$2] += $3 }
END {
    print "Department Summary:"
    for (dept in dept_count) {
        avg = dept_salary[dept] / dept_count[dept]
        printf "  %-12s: %d employees, avg salary: %.0f\n", dept, dept_count[dept], avg
    }
}
