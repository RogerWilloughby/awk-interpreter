BEGIN { for (i = 1; i <= 100000; i++) if (i ~ /^[0-9]+$/) c++; print c }
