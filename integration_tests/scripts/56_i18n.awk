# Test: i18n (internationalization) functions
# These are gawk extensions for gettext-based localization
BEGIN {
    print "i18n Functions Test:"
    print "===================="

    # Test 1: dcgettext(string [, domain [, category]]) - translate a string
    # Returns original string when no translation catalog is loaded
    msg1 = dcgettext("Hello")
    print "dcgettext test:"
    print "  dcgettext(Hello) =", msg1

    # With explicit domain
    msg2 = dcgettext("Goodbye", "myapp")
    print "  dcgettext(Goodbye, myapp) =", msg2

    # Test 2: dcngettext(singular, plural, n [, domain [, category]])
    # Returns singular/plural based on n
    msg3 = dcngettext("file", "files", 1)
    msg4 = dcngettext("file", "files", 5)
    print "dcngettext test:"
    print "  dcngettext(file, files, 1) =", msg3
    print "  dcngettext(file, files, 5) =", msg4

    # Test 3: bindtextdomain(domain, directory) - set domain directory
    path = bindtextdomain("myapp", "/usr/share/locale")
    print "bindtextdomain test:"
    print "  bindtextdomain(myapp, /usr/share/locale) =", path

    # Test 4: bindtextdomain query (no path argument)
    path2 = bindtextdomain("myapp")
    print "  bindtextdomain(myapp) =", path2
}
