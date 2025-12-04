BEGIN { for (i = 1; i <= 50000; i++) a[i] = i; for (i in a) sum += a[i]; print sum }
