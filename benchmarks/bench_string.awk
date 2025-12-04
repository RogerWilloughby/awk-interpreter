BEGIN { s = ""; for (i = 1; i <= 50000; i++) s = s "x"; print length(s) }
