#ifndef AWK_I18N_HPP
#define AWK_I18N_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cstdint>

namespace awk {

// Plural form evaluation function type
// Takes 'n' and returns the plural form index
using PluralFunc = int(*)(unsigned long n);

// A loaded translation catalog (.mo file)
class MoCatalog {
public:
    MoCatalog() = default;

    // Load from a .mo file, returns true on success
    bool load(const std::string& filename);

    // Get translation for a string (returns original if not found)
    const std::string* gettext(const std::string& msgid) const;

    // Get plural form translation
    // Returns nullptr if not found, otherwise the appropriate plural form
    const std::string* ngettext(const std::string& msgid, const std::string& msgid_plural,
                                unsigned long n) const;

    // Check if catalog is loaded
    bool is_loaded() const { return loaded_; }

    // Get charset from metadata
    const std::string& charset() const { return charset_; }

private:
    bool loaded_ = false;
    std::string charset_ = "UTF-8";

    // Simple translation map: msgid -> msgstr
    std::unordered_map<std::string, std::string> translations_;

    // Plural translations: msgid -> vector of plural forms
    std::unordered_map<std::string, std::vector<std::string>> plural_translations_;

    // Number of plural forms
    int nplurals_ = 2;

    // Plural formula function
    PluralFunc plural_func_ = nullptr;

    // Parse the header entry to extract charset and plural info
    void parse_header(const std::string& header);

    // Default English plural rule: n != 1
    static int default_plural(unsigned long n) { return n != 1 ? 1 : 0; }

    // Parse plural forms expression and return a function
    static PluralFunc parse_plural_forms(const std::string& expr, int& nplurals);
};

// The i18n translation manager
class I18n {
public:
    static I18n& instance();

    // Bind a text domain to a directory
    std::string bindtextdomain(const std::string& domain, const std::string& directory);

    // Get the directory for a domain
    std::string get_textdomain_directory(const std::string& domain) const;

    // Get translation for a string
    std::string dcgettext(const std::string& msgid, const std::string& domain,
                          const std::string& category = "LC_MESSAGES");

    // Get plural translation
    std::string dcngettext(const std::string& msgid, const std::string& msgid_plural,
                           unsigned long n, const std::string& domain,
                           const std::string& category = "LC_MESSAGES");

    // Get/set current locale
    std::string get_locale() const;
    void set_locale(const std::string& locale);

    // Clear all cached catalogs (for testing)
    void clear_cache();

private:
    I18n();

    // Domain -> directory mapping
    std::unordered_map<std::string, std::string> domain_directories_;

    // Cache of loaded catalogs: "domain:locale:category" -> catalog
    std::unordered_map<std::string, std::shared_ptr<MoCatalog>> catalogs_;

    // Current locale
    std::string locale_;

    // Get or load a catalog for domain/locale/category
    std::shared_ptr<MoCatalog> get_catalog(const std::string& domain,
                                            const std::string& category);

    // Build path to .mo file
    std::string build_mo_path(const std::string& domain, const std::string& locale,
                              const std::string& category) const;

    // Detect system locale
    static std::string detect_locale();
};

} // namespace awk

#endif // AWK_I18N_HPP
