// i18n Unit Tests
#include "test_framework.hpp"
#include "awk/i18n.hpp"
#include <fstream>
#include <cstring>

using namespace awk;
using namespace test;

// Helper to create a minimal .mo file for testing
// This creates a valid .mo file with simple translations
static bool create_test_mo_file(const std::string& path,
                                 const std::vector<std::pair<std::string, std::string>>& translations) {
    // Create directories
#ifdef _WIN32
    std::string dir = path.substr(0, path.find_last_of("\\/"));
    std::string cmd = "mkdir \"" + dir + "\" 2>nul";
    system(cmd.c_str());
#else
    std::string dir = path.substr(0, path.find_last_of('/'));
    std::string cmd = "mkdir -p \"" + dir + "\"";
    system(cmd.c_str());
#endif

    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    // Calculate sizes
    size_t N = translations.size() + 1;  // +1 for header

    // Build string data
    std::vector<std::string> originals;
    std::vector<std::string> trans;

    // Header entry (empty msgid)
    originals.push_back("");
    trans.push_back("Content-Type: text/plain; charset=UTF-8\n");

    for (const auto& pair : translations) {
        originals.push_back(pair.first);
        trans.push_back(pair.second);
    }

    // Calculate offsets
    uint32_t header_size = 28;
    uint32_t orig_table_offset = header_size;
    uint32_t trans_table_offset = orig_table_offset + static_cast<uint32_t>(N * 8);
    uint32_t strings_offset = trans_table_offset + static_cast<uint32_t>(N * 8);

    // Calculate string positions
    std::vector<std::pair<uint32_t, uint32_t>> orig_desc;  // (length, offset)
    std::vector<std::pair<uint32_t, uint32_t>> trans_desc;

    uint32_t current_offset = strings_offset;
    for (const auto& s : originals) {
        orig_desc.push_back({static_cast<uint32_t>(s.size()), current_offset});
        current_offset += static_cast<uint32_t>(s.size()) + 1;
    }
    for (const auto& s : trans) {
        trans_desc.push_back({static_cast<uint32_t>(s.size()), current_offset});
        current_offset += static_cast<uint32_t>(s.size()) + 1;
    }

    // Write header
    auto write_u32 = [&file](uint32_t val) {
        file.write(reinterpret_cast<const char*>(&val), 4);
    };

    write_u32(0x950412de);  // Magic (little endian)
    write_u32(0);           // Revision
    write_u32(static_cast<uint32_t>(N));  // Number of strings
    write_u32(orig_table_offset);
    write_u32(trans_table_offset);
    write_u32(0);           // Hash table size
    write_u32(0);           // Hash table offset

    // Write original strings table
    for (const auto& desc : orig_desc) {
        write_u32(desc.first);   // Length
        write_u32(desc.second);  // Offset
    }

    // Write translation strings table
    for (const auto& desc : trans_desc) {
        write_u32(desc.first);
        write_u32(desc.second);
    }

    // Write strings
    for (const auto& s : originals) {
        file.write(s.c_str(), s.size() + 1);  // Include NUL
    }
    for (const auto& s : trans) {
        file.write(s.c_str(), s.size() + 1);
    }

    return file.good();
}

// Helper to clean up test files
static void cleanup_test_files() {
#ifdef _WIN32
    system("rmdir /s /q test_locale 2>nul");
#else
    system("rm -rf test_locale 2>/dev/null");
#endif
}

// ============================================================================
// MoCatalog Tests
// ============================================================================

TEST(I18n_MoCatalog_Load_Valid) {
    // Create a test .mo file
    std::vector<std::pair<std::string, std::string>> translations = {
        {"Hello", "Hallo"},
        {"World", "Welt"}
    };

    std::string mo_path = "test_locale/de/LC_MESSAGES/test.mo";
    ASSERT_TRUE(create_test_mo_file(mo_path, translations));

    MoCatalog catalog;
    ASSERT_TRUE(catalog.load(mo_path));
    ASSERT_TRUE(catalog.is_loaded());

    cleanup_test_files();
}

TEST(I18n_MoCatalog_Load_Invalid) {
    MoCatalog catalog;
    ASSERT_FALSE(catalog.load("nonexistent.mo"));
    ASSERT_FALSE(catalog.is_loaded());
}

TEST(I18n_MoCatalog_Gettext) {
    std::vector<std::pair<std::string, std::string>> translations = {
        {"Hello", "Hallo"},
        {"World", "Welt"},
        {"Hello World", "Hallo Welt"}
    };

    std::string mo_path = "test_locale/de/LC_MESSAGES/test.mo";
    ASSERT_TRUE(create_test_mo_file(mo_path, translations));

    MoCatalog catalog;
    ASSERT_TRUE(catalog.load(mo_path));

    const std::string* result = catalog.gettext("Hello");
    ASSERT_TRUE(result != nullptr);
    ASSERT_EQ(*result, "Hallo");

    result = catalog.gettext("World");
    ASSERT_TRUE(result != nullptr);
    ASSERT_EQ(*result, "Welt");

    result = catalog.gettext("Hello World");
    ASSERT_TRUE(result != nullptr);
    ASSERT_EQ(*result, "Hallo Welt");

    // Non-existent string
    result = catalog.gettext("Goodbye");
    ASSERT_TRUE(result == nullptr);

    cleanup_test_files();
}

// ============================================================================
// I18n Singleton Tests
// ============================================================================

TEST(I18n_Bindtextdomain) {
    I18n& i18n = I18n::instance();
    i18n.clear_cache();

    std::string result = i18n.bindtextdomain("myapp", "/usr/share/locale");
    ASSERT_EQ(result, "/usr/share/locale");

    result = i18n.get_textdomain_directory("myapp");
    ASSERT_EQ(result, "/usr/share/locale");

    result = i18n.get_textdomain_directory("unknown");
    ASSERT_EQ(result, "");
}

TEST(I18n_Dcgettext_No_Translation) {
    I18n& i18n = I18n::instance();
    i18n.clear_cache();

    // Without any binding, should return original
    std::string result = i18n.dcgettext("Hello", "nonexistent", "LC_MESSAGES");
    ASSERT_EQ(result, "Hello");
}

TEST(I18n_Dcgettext_With_Translation) {
    // Create test .mo file
    std::vector<std::pair<std::string, std::string>> translations = {
        {"Hello", "Hallo"},
        {"Goodbye", "Auf Wiedersehen"}
    };

    std::string mo_path = "test_locale/de/LC_MESSAGES/testapp.mo";
    ASSERT_TRUE(create_test_mo_file(mo_path, translations));

    I18n& i18n = I18n::instance();
    i18n.clear_cache();
    i18n.set_locale("de");
    i18n.bindtextdomain("testapp", "test_locale");

    std::string result = i18n.dcgettext("Hello", "testapp", "LC_MESSAGES");
    ASSERT_EQ(result, "Hallo");

    result = i18n.dcgettext("Goodbye", "testapp", "LC_MESSAGES");
    ASSERT_EQ(result, "Auf Wiedersehen");

    // Non-existent string returns original
    result = i18n.dcgettext("Unknown", "testapp", "LC_MESSAGES");
    ASSERT_EQ(result, "Unknown");

    cleanup_test_files();
}

TEST(I18n_Dcngettext_Singular) {
    I18n& i18n = I18n::instance();
    i18n.clear_cache();

    // Without translation, use English plural rule
    std::string result = i18n.dcngettext("1 file", "%d files", 1, "test", "LC_MESSAGES");
    ASSERT_EQ(result, "1 file");
}

TEST(I18n_Dcngettext_Plural) {
    I18n& i18n = I18n::instance();
    i18n.clear_cache();

    std::string result = i18n.dcngettext("1 file", "%d files", 5, "test", "LC_MESSAGES");
    ASSERT_EQ(result, "%d files");
}

TEST(I18n_Dcngettext_Zero) {
    I18n& i18n = I18n::instance();
    i18n.clear_cache();

    std::string result = i18n.dcngettext("1 file", "%d files", 0, "test", "LC_MESSAGES");
    ASSERT_EQ(result, "%d files");
}

TEST(I18n_Locale_Detection) {
    I18n& i18n = I18n::instance();

    // Just verify that locale detection doesn't crash
    std::string locale = i18n.get_locale();
    ASSERT_TRUE(!locale.empty());
}

TEST(I18n_Set_Locale) {
    I18n& i18n = I18n::instance();
    i18n.clear_cache();

    i18n.set_locale("fr_FR.UTF-8");
    ASSERT_EQ(i18n.get_locale(), "fr_FR.UTF-8");

    i18n.set_locale("de_DE");
    ASSERT_EQ(i18n.get_locale(), "de_DE");
}

TEST(I18n_Clear_Cache) {
    I18n& i18n = I18n::instance();

    // This should not crash
    i18n.clear_cache();

    // After clearing, bindings should still work
    i18n.bindtextdomain("test", "/tmp/locale");
    ASSERT_EQ(i18n.get_textdomain_directory("test"), "/tmp/locale");
}
