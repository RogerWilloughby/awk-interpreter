#include "awk/i18n.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <clocale>
#endif

namespace awk {

// ============================================================================
// MoCatalog Implementation
// ============================================================================

// .mo file magic numbers
static constexpr uint32_t MO_MAGIC_LE = 0x950412de;  // Little-endian
static constexpr uint32_t MO_MAGIC_BE = 0xde120495;  // Big-endian

// .mo file header offsets (all uint32_t values)
static constexpr size_t MO_HEADER_SIZE = 28;         // Minimum valid header size
static constexpr size_t MO_OFFSET_MAGIC = 0;         // Magic number
static constexpr size_t MO_OFFSET_VERSION = 4;       // File format revision
static constexpr size_t MO_OFFSET_NUM_STRINGS = 8;   // Number of strings
static constexpr size_t MO_OFFSET_ORIGINALS = 12;    // Offset of original strings table
static constexpr size_t MO_OFFSET_TRANSLATIONS = 16; // Offset of translation strings table
static constexpr size_t MO_OFFSET_HASH_SIZE = 20;    // Size of hashing table
static constexpr size_t MO_OFFSET_HASH = 24;         // Offset of hashing table

// String descriptor entry size (length + offset, each uint32_t)
static constexpr size_t MO_STRING_DESC_SIZE = 8;
static constexpr size_t MO_STRING_DESC_LEN_OFFSET = 0;
static constexpr size_t MO_STRING_DESC_PTR_OFFSET = 4;

// Swap bytes for endianness conversion
static uint32_t swap32(uint32_t value) {
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

bool MoCatalog::load(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }

    // Read entire file into memory
    file.seekg(0, std::ios::end);
    size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    if (file_size < MO_HEADER_SIZE) {
        return false;
    }

    std::vector<char> data(file_size);
    file.read(data.data(), file_size);
    if (!file) {
        return false;
    }

    // Check magic number and determine endianness
    uint32_t magic;
    std::memcpy(&magic, data.data() + MO_OFFSET_MAGIC, sizeof(uint32_t));

    bool swap_bytes = false;
    if (magic == MO_MAGIC_LE) {
        swap_bytes = false;
    } else if (magic == MO_MAGIC_BE) {
        swap_bytes = true;
    } else {
        return false;  // Invalid magic number
    }

    // Helper to read uint32 with correct endianness
    auto read_u32 = [&data, swap_bytes](size_t offset) -> uint32_t {
        uint32_t value;
        std::memcpy(&value, data.data() + offset, sizeof(uint32_t));
        return swap_bytes ? swap32(value) : value;
    };

    // Read header
    // uint32_t version = read_u32(MO_OFFSET_VERSION);  // Not needed for basic parsing
    uint32_t num_strings = read_u32(MO_OFFSET_NUM_STRINGS);
    uint32_t offset_originals = read_u32(MO_OFFSET_ORIGINALS);
    uint32_t offset_translations = read_u32(MO_OFFSET_TRANSLATIONS);
    // uint32_t hash_size = read_u32(MO_OFFSET_HASH_SIZE);  // Not using hash table
    // uint32_t hash_offset = read_u32(MO_OFFSET_HASH);

    // Validate offsets
    if (offset_originals + num_strings * MO_STRING_DESC_SIZE > file_size ||
        offset_translations + num_strings * MO_STRING_DESC_SIZE > file_size) {
        return false;
    }

    // Helper to read a string from the file
    auto read_string = [&data, file_size, &read_u32](size_t table_offset, size_t index) -> std::string {
        size_t desc_offset = table_offset + index * MO_STRING_DESC_SIZE;
        uint32_t str_len = read_u32(desc_offset + MO_STRING_DESC_LEN_OFFSET);
        uint32_t str_offset = read_u32(desc_offset + MO_STRING_DESC_PTR_OFFSET);

        if (str_offset + str_len > file_size) {
            return "";
        }

        return std::string(data.data() + str_offset, str_len);
    };

    // Read all translations
    for (uint32_t i = 0; i < num_strings; ++i) {
        std::string original = read_string(offset_originals, i);
        std::string translation = read_string(offset_translations, i);

        if (original.empty() && i == 0) {
            // First entry with empty msgid is the header
            parse_header(translation);
            continue;
        }

        // Check for plural forms (contains NUL byte)
        size_t nul_pos = original.find('\0');
        if (nul_pos != std::string::npos) {
            // Plural form: msgid\0msgid_plural
            std::string msgid = original.substr(0, nul_pos);

            // Split translations by NUL
            std::vector<std::string> plurals;
            size_t start = 0;
            size_t pos;
            while ((pos = translation.find('\0', start)) != std::string::npos) {
                plurals.push_back(translation.substr(start, pos - start));
                start = pos + 1;
            }
            if (start < translation.size()) {
                plurals.push_back(translation.substr(start));
            }

            plural_translations_[msgid] = std::move(plurals);
        } else {
            // Simple translation
            translations_[original] = translation;
        }
    }

    loaded_ = true;
    return true;
}

void MoCatalog::parse_header(const std::string& header) {
    // Parse header lines for metadata
    std::istringstream iss(header);
    std::string line;

    while (std::getline(iss, line)) {
        // Remove trailing \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Look for Content-Type to get charset
        if (line.find("Content-Type:") == 0) {
            size_t charset_pos = line.find("charset=");
            if (charset_pos != std::string::npos) {
                charset_ = line.substr(charset_pos + 8);
                // Remove trailing whitespace
                while (!charset_.empty() && (charset_.back() == ' ' || charset_.back() == '\n')) {
                    charset_.pop_back();
                }
            }
        }

        // Look for Plural-Forms
        if (line.find("Plural-Forms:") == 0) {
            std::string plural_info = line.substr(13);
            // Trim leading whitespace
            size_t start = plural_info.find_first_not_of(" \t");
            if (start != std::string::npos) {
                plural_info = plural_info.substr(start);
            }

            plural_func_ = parse_plural_forms(plural_info, nplurals_);
        }
    }

    if (!plural_func_) {
        plural_func_ = default_plural;
        nplurals_ = 2;
    }
}

// Simple plural form expression evaluator
// Supports: nplurals=N; plural=EXPR;
// EXPR can use: n, ?, :, ==, !=, <, >, <=, >=, %, &&, ||, (, )
PluralFunc MoCatalog::parse_plural_forms(const std::string& expr, int& nplurals) {
    // Extract nplurals
    size_t np_pos = expr.find("nplurals=");
    if (np_pos != std::string::npos) {
        nplurals = std::atoi(expr.c_str() + np_pos + 9);
    } else {
        nplurals = 2;
    }

    // Extract plural expression
    size_t pl_pos = expr.find("plural=");
    if (pl_pos == std::string::npos) {
        return default_plural;
    }

    std::string plural_expr = expr.substr(pl_pos + 7);
    // Remove trailing semicolon and whitespace
    size_t end = plural_expr.find(';');
    if (end != std::string::npos) {
        plural_expr = plural_expr.substr(0, end);
    }

    // Common plural forms - we'll handle these with pre-defined functions
    // This avoids the complexity of a full expression parser

    // English: n != 1  (or n!=1)
    if (plural_expr == "n != 1" || plural_expr == "n!=1" ||
        plural_expr == "(n != 1)" || plural_expr == "(n!=1)") {
        return [](unsigned long n) -> int { return n != 1 ? 1 : 0; };
    }

    // French/Brazilian Portuguese: n > 1
    if (plural_expr == "n > 1" || plural_expr == "n>1" ||
        plural_expr == "(n > 1)" || plural_expr == "(n>1)") {
        return [](unsigned long n) -> int { return n > 1 ? 1 : 0; };
    }

    // Russian/Ukrainian/Serbian/Croatian: 3 forms
    // (n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)
    if (plural_expr.find("n%10==1") != std::string::npos &&
        plural_expr.find("n%100!=11") != std::string::npos) {
        return [](unsigned long n) -> int {
            if (n % 10 == 1 && n % 100 != 11) return 0;
            if (n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 10 || n % 100 >= 20)) return 1;
            return 2;
        };
    }

    // Polish: 3 forms
    // n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2
    if (plural_expr.find("n==1") != std::string::npos &&
        plural_expr.find("n%10>=2") != std::string::npos) {
        return [](unsigned long n) -> int {
            if (n == 1) return 0;
            if (n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 10 || n % 100 >= 20)) return 1;
            return 2;
        };
    }

    // Czech/Slovak: 3 forms
    // (n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2
    if (plural_expr.find("n==1") != std::string::npos &&
        plural_expr.find("n>=2") != std::string::npos &&
        plural_expr.find("n<=4") != std::string::npos) {
        return [](unsigned long n) -> int {
            if (n == 1) return 0;
            if (n >= 2 && n <= 4) return 1;
            return 2;
        };
    }

    // Arabic: 6 forms (simplified)
    if (nplurals == 6) {
        return [](unsigned long n) -> int {
            if (n == 0) return 0;
            if (n == 1) return 1;
            if (n == 2) return 2;
            if (n % 100 >= 3 && n % 100 <= 10) return 3;
            if (n % 100 >= 11) return 4;
            return 5;
        };
    }

    // Japanese/Chinese/Korean: 1 form (no plural)
    if (plural_expr == "0") {
        return [](unsigned long) -> int { return 0; };
    }

    // Default: English rule
    return default_plural;
}

const std::string* MoCatalog::gettext(const std::string& msgid) const {
    auto it = translations_.find(msgid);
    if (it != translations_.end()) {
        return &it->second;
    }
    return nullptr;
}

const std::string* MoCatalog::ngettext(const std::string& msgid, const std::string& msgid_plural,
                                        unsigned long n) const {
    // First check plural translations
    auto it = plural_translations_.find(msgid);
    if (it != plural_translations_.end()) {
        const auto& forms = it->second;
        int index = plural_func_ ? plural_func_(n) : (n != 1 ? 1 : 0);
        if (index < 0) index = 0;
        if (static_cast<size_t>(index) >= forms.size()) {
            index = static_cast<int>(forms.size()) - 1;
        }
        if (index >= 0 && static_cast<size_t>(index) < forms.size()) {
            return &forms[index];
        }
    }

    // Fall back to simple translation lookup
    auto simple_it = translations_.find(msgid);
    if (simple_it != translations_.end()) {
        return &simple_it->second;
    }

    // No translation found - caller will use original
    (void)msgid_plural;  // Unused in lookup, only for fallback
    return nullptr;
}

// ============================================================================
// I18n Implementation
// ============================================================================

I18n& I18n::instance() {
    static I18n instance;
    return instance;
}

I18n::I18n() : locale_(detect_locale()) {
}

std::string I18n::detect_locale() {
#ifdef _WIN32
    // Get Windows locale using wide character API
    wchar_t locale_name_w[LOCALE_NAME_MAX_LENGTH];
    if (GetUserDefaultLocaleName(locale_name_w, LOCALE_NAME_MAX_LENGTH) > 0) {
        // Convert wide string to narrow string
        char locale_name[LOCALE_NAME_MAX_LENGTH];
        size_t converted = 0;
        wcstombs_s(&converted, locale_name, sizeof(locale_name), locale_name_w, _TRUNCATE);
        // Convert from Windows format (e.g., "en-US") to POSIX (e.g., "en_US")
        std::string result(locale_name);
        std::replace(result.begin(), result.end(), '-', '_');
        return result;
    }
    return "C";
#else
    // Get POSIX locale from environment
    const char* lang = std::getenv("LANGUAGE");
    if (!lang || !*lang) {
        lang = std::getenv("LC_ALL");
    }
    if (!lang || !*lang) {
        lang = std::getenv("LC_MESSAGES");
    }
    if (!lang || !*lang) {
        lang = std::getenv("LANG");
    }
    if (lang && *lang) {
        return lang;
    }
    return "C";
#endif
}

std::string I18n::get_locale() const {
    return locale_;
}

void I18n::set_locale(const std::string& locale) {
    if (locale != locale_) {
        locale_ = locale;
        // Clear cache when locale changes
        catalogs_.clear();
    }
}

std::string I18n::bindtextdomain(const std::string& domain, const std::string& directory) {
    if (directory.empty()) {
        // Query current binding
        auto it = domain_directories_.find(domain);
        if (it != domain_directories_.end()) {
            return it->second;
        }
        return "";
    }

    domain_directories_[domain] = directory;

    // Invalidate cached catalogs for this domain
    for (auto it = catalogs_.begin(); it != catalogs_.end();) {
        if (it->first.find(domain + ":") == 0) {
            it = catalogs_.erase(it);
        } else {
            ++it;
        }
    }

    return directory;
}

std::string I18n::get_textdomain_directory(const std::string& domain) const {
    auto it = domain_directories_.find(domain);
    if (it != domain_directories_.end()) {
        return it->second;
    }
    return "";
}

std::string I18n::build_mo_path(const std::string& domain, const std::string& locale,
                                 const std::string& category) const {
    auto it = domain_directories_.find(domain);
    if (it == domain_directories_.end()) {
        return "";
    }

    const std::string& base_dir = it->second;

    // Standard gettext path: <basedir>/<locale>/<category>/<domain>.mo
    // e.g., /usr/share/locale/de/LC_MESSAGES/myapp.mo

#ifdef _WIN32
    const char sep = '\\';
#else
    const char sep = '/';
#endif

    return base_dir + sep + locale + sep + category + sep + domain + ".mo";
}

std::shared_ptr<MoCatalog> I18n::get_catalog(const std::string& domain,
                                              const std::string& category) {
    std::string key = domain + ":" + locale_ + ":" + category;

    auto it = catalogs_.find(key);
    if (it != catalogs_.end()) {
        return it->second;
    }

    // Try to load catalog
    auto catalog = std::make_shared<MoCatalog>();

    // Try full locale first (e.g., de_DE.UTF-8)
    std::string path = build_mo_path(domain, locale_, category);
    if (!path.empty() && catalog->load(path)) {
        catalogs_[key] = catalog;
        return catalog;
    }

    // Try without encoding (e.g., de_DE)
    size_t dot_pos = locale_.find('.');
    if (dot_pos != std::string::npos) {
        std::string short_locale = locale_.substr(0, dot_pos);
        path = build_mo_path(domain, short_locale, category);
        if (!path.empty() && catalog->load(path)) {
            catalogs_[key] = catalog;
            return catalog;
        }
    }

    // Try language only (e.g., de)
    size_t underscore_pos = locale_.find('_');
    if (underscore_pos != std::string::npos) {
        std::string lang = locale_.substr(0, underscore_pos);
        path = build_mo_path(domain, lang, category);
        if (!path.empty() && catalog->load(path)) {
            catalogs_[key] = catalog;
            return catalog;
        }
    }

    // Cache the failure too (nullptr)
    catalogs_[key] = nullptr;
    return nullptr;
}

std::string I18n::dcgettext(const std::string& msgid, const std::string& domain,
                             const std::string& category) {
    auto catalog = get_catalog(domain, category);
    if (catalog) {
        const std::string* translation = catalog->gettext(msgid);
        if (translation) {
            return *translation;
        }
    }
    return msgid;  // Return original if no translation
}

std::string I18n::dcngettext(const std::string& msgid, const std::string& msgid_plural,
                              unsigned long n, const std::string& domain,
                              const std::string& category) {
    auto catalog = get_catalog(domain, category);
    if (catalog) {
        const std::string* translation = catalog->ngettext(msgid, msgid_plural, n);
        if (translation) {
            return *translation;
        }
    }

    // Return original based on English plural rule
    return (n == 1) ? msgid : msgid_plural;
}

void I18n::clear_cache() {
    catalogs_.clear();
}

} // namespace awk
