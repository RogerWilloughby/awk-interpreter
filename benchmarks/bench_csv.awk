BEGIN { FS = "," } { c += NF } END { print c }
