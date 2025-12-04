# Test: Word frequency count
{
    for (i = 1; i <= NF; i++) {
        word = tolower($i)
        gsub(/[^a-z]/, "", word)
        if (length(word) > 0)
            words[word]++
    }
}
END {
    print "Word Frequency (words appearing more than once):"
    for (w in words)
        if (words[w] > 1)
            printf "  %-10s: %d\n", w, words[w]
}
