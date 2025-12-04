// ============================================================================
// interpreter_builtins_string.cpp - String Built-in Functions
// ============================================================================

#include "awk/interpreter.hpp"
#include "awk/i18n.hpp"
#include <sstream>
#include <algorithm>
#include <regex>

namespace awk {

// Helper: Prepare target string for sub/gsub operations
// Returns the target string and sets modify_record to true if $0 should be updated
static std::string prepare_sub_target(std::vector<AWKValue>& args, Interpreter& interp, bool& modify_record) {
    if (args.size() >= 3 && !args[2].is_uninitialized()) {
        modify_record = false;
        return args[2].to_string();
    } else {
        modify_record = true;
        return interp.current_record();
    }
}

// Helper: Perform sub/gsub operation
// Returns number of replacements made (0 or 1 for sub, 0-N for gsub)
static int do_substitution(const std::string& pattern, const std::string& replacement,
                           std::string& target, bool global, Interpreter& interp) {
    try {
        const std::regex& re = interp.get_cached_regex(pattern);
        std::string awk_replacement = convert_awk_replacement(replacement);

        if (global) {
            // Count matches first (gsub returns count)
            int count = 0;
            std::string::const_iterator searchStart(target.cbegin());
            std::smatch match;
            while (std::regex_search(searchStart, target.cend(), match, re)) {
                ++count;
                searchStart = match.suffix().first;
                if (match.length() == 0) {
                    if (searchStart != target.cend()) ++searchStart;
                    else break;
                }
            }
            target = std::regex_replace(target, re, awk_replacement);
            return count;
        } else {
            // sub: replace first only
            std::string result = std::regex_replace(target, re, awk_replacement,
                                                    std::regex_constants::format_first_only);
            if (result != target) {
                target = result;
                return 1;
            }
            return 0;
        }
    } catch (...) {
        return 0;
    }
}

// Helper: Split a string using a separator (FS-style splitting)
// Handles special case of " " (whitespace), single char, and regex separators
static std::vector<std::string> split_string(const std::string& str, const std::string& fs,
                                             Interpreter& interp) {
    std::vector<std::string> parts;

    if (fs == " ") {
        // Standard AWK: whitespace splitting, multiple spaces ignored
        std::istringstream iss(str);
        std::string part;
        while (iss >> part) {
            parts.push_back(part);
        }
    } else if (fs.length() == 1) {
        // Single character separator
        std::string::size_type start = 0;
        std::string::size_type pos;
        while ((pos = str.find(fs, start)) != std::string::npos) {
            parts.push_back(str.substr(start, pos - start));
            start = pos + 1;
        }
        parts.push_back(str.substr(start));
    } else {
        // Regex separator
        try {
            const std::regex& re = interp.get_cached_regex(fs);
            std::sregex_token_iterator it(str.begin(), str.end(), re, -1);
            std::sregex_token_iterator end;
            while (it != end) {
                parts.push_back(*it++);
            }
        } catch (...) {
            parts.push_back(str);
        }
    }

    return parts;
}

void Interpreter::register_string_builtins() {
    env_.register_builtin("length", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.empty()) {
            return AWKValue(static_cast<double>(interp.current_record().length()));
        }
        if (args[0].is_array()) {
            return AWKValue(static_cast<double>(args[0].array_size()));
        }
        return AWKValue(static_cast<double>(args[0].to_string().length()));
    });

    env_.register_builtin("substr", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue("");
        std::string str = args[0].to_string();
        int start = args.size() > 1 ? static_cast<int>(args[1].to_number()) : 1;
        size_t len = args.size() > 2
            ? static_cast<size_t>(args[2].to_number())
            : std::string::npos;

        if (start < 1) start = 1;
        size_t idx = static_cast<size_t>(start - 1);

        if (idx >= str.length()) return AWKValue("");
        return AWKValue(str.substr(idx, len));
    });

    env_.register_builtin("index", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.size() < 2) return AWKValue(0.0);
        std::string str = args[0].to_string();
        std::string needle = args[1].to_string();
        size_t pos = str.find(needle);
        return AWKValue(pos == std::string::npos ? 0.0 : static_cast<double>(pos + 1));
    });

    env_.register_builtin("tolower", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue("");
        std::string str = args[0].to_string();
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return AWKValue(str);
    });

    env_.register_builtin("toupper", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue("");
        std::string str = args[0].to_string();
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return AWKValue(str);
    });

    env_.register_builtin("sprintf", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.empty()) return AWKValue("");
        std::string format = args[0].to_string();
        std::vector<AWKValue> fmt_args(args.begin() + 1, args.end());
        return AWKValue(interp.do_sprintf(format, fmt_args));
    });

    env_.register_builtin("strtonum", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(0.0);

        std::string str = args[0].to_string();
        const char* s = str.c_str();

        while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;

        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            return AWKValue(static_cast<double>(std::strtoll(s, nullptr, 16)));
        }

        if (s[0] == '0' && std::isdigit(static_cast<unsigned char>(s[1]))) {
            return AWKValue(static_cast<double>(std::strtoll(s, nullptr, 8)));
        }

        return AWKValue(std::strtod(s, nullptr));
    });

    env_.register_builtin("ord", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(0.0);
        std::string str = args[0].to_string();
        if (str.empty()) return AWKValue(0.0);
        return AWKValue(static_cast<double>(static_cast<unsigned char>(str[0])));
    });

    env_.register_builtin("chr", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue("");
        int code = static_cast<int>(args[0].to_number());
        if (code < 0 || code > 255) return AWKValue("");
        return AWKValue(std::string(1, static_cast<char>(code)));
    });

    // sub(regexp, replacement [, target])
    env_.register_builtin("sub", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.size() < 2) return AWKValue(0.0);

        bool modify_record;
        std::string target = prepare_sub_target(args, interp, modify_record);
        int count = do_substitution(args[0].to_string(), args[1].to_string(),
                                    target, false, interp);

        if (count > 0 && modify_record) {
            interp.set_record(target);
        }
        return AWKValue(static_cast<double>(count));
    });

    // gsub(regexp, replacement [, target])
    env_.register_builtin("gsub", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.size() < 2) return AWKValue(0.0);

        bool modify_record;
        std::string target = prepare_sub_target(args, interp, modify_record);
        int count = do_substitution(args[0].to_string(), args[1].to_string(),
                                    target, true, interp);

        if (count > 0 && modify_record) {
            interp.set_record(target);
        }
        return AWKValue(static_cast<double>(count));
    });

    // gensub(regexp, replacement, how [, target])
    env_.register_builtin("gensub", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.size() < 3) return AWKValue("");

        std::string pattern = args[0].to_string();
        std::string replacement = args[1].to_string();
        std::string how = args[2].to_string();

        std::string target;
        if (args.size() >= 4) {
            target = args[3].to_string();
        } else {
            target = interp.current_record();
        }

        try {
            const std::regex& re = interp.get_cached_regex(pattern);
            std::string awk_replacement = convert_awk_replacement(replacement, true);

            if (how == "g" || how == "G") {
                return AWKValue(std::regex_replace(target, re, awk_replacement));
            } else {
                int which = static_cast<int>(std::strtod(how.c_str(), nullptr));
                if (which < 1) which = 1;

                std::string::const_iterator searchStart(target.cbegin());
                std::smatch match;
                int count = 0;

                while (std::regex_search(searchStart, target.cend(), match, re)) {
                    ++count;
                    size_t matchStart = (searchStart - target.cbegin()) + match.position();

                    if (count == which) {
                        std::string result = target.substr(0, matchStart);
                        result += match.format(awk_replacement);
                        result += target.substr(matchStart + match.length());
                        return AWKValue(result);
                    }

                    searchStart = match.suffix().first;
                    if (match.length() == 0 && searchStart != target.cend()) {
                        ++searchStart;
                    }
                }

                return AWKValue(target);
            }

        } catch (...) {
            return AWKValue(target);
        }
    });

    // split(string, array [, fieldsep]) - Note: Special handling in evaluate(CallExpr&)
    env_.register_builtin("split", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.size() < 2) return AWKValue(0.0);

        std::string str = args[0].to_string();
        std::string fs = args.size() >= 3 ? args[2].to_string()
                                          : interp.environment().FS().to_string();

        std::vector<std::string> parts = split_string(str, fs, interp);
        return AWKValue(static_cast<double>(parts.size()));
    });

    // patsplit - Note: Special handling in evaluate(CallExpr&)
    env_.register_builtin("patsplit", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.size() < 3) return AWKValue(0.0);

        std::string str = args[0].to_string();
        std::string pattern = args[2].to_string();
        bool has_seps = (args.size() >= 4);

        args[1].array_clear();
        if (has_seps) {
            args[3].array_clear();
        }

        try {
            const std::regex& re = interp.get_cached_regex(pattern);
            std::sregex_iterator it(str.begin(), str.end(), re);
            std::sregex_iterator end;

            int count = 0;
            size_t last_end = 0;
            while (it != end) {
                if (has_seps) {
                    std::string sep = str.substr(last_end, it->position() - last_end);
                    args[3].array_access(std::to_string(count)) = AWKValue(sep);
                }

                count++;
                args[1].array_access(std::to_string(count)) = AWKValue(it->str());

                last_end = it->position() + it->length();
                ++it;
            }

            if (has_seps && last_end < str.length()) {
                args[3].array_access(std::to_string(count)) = AWKValue(str.substr(last_end));
            }

            return AWKValue(static_cast<double>(count));

        } catch (...) {
            return AWKValue(0.0);
        }
    });

    // match - Note: Special handling in evaluate(CallExpr&) for array parameter
    env_.register_builtin("match", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.size() < 2) return AWKValue(0.0);

        std::string str = args[0].to_string();
        std::string pattern = args[1].to_string();

        try {
            const std::regex& re = interp.get_cached_regex(pattern);
            std::smatch match;

            if (std::regex_search(str, match, re)) {
                int start = static_cast<int>(match.position()) + 1;
                int length = static_cast<int>(match.length());

                interp.environment().RSTART() = AWKValue(static_cast<double>(start));
                interp.environment().RLENGTH() = AWKValue(static_cast<double>(length));

                if (args.size() >= 3) {
                    args[2].array_clear();
                    for (size_t i = 0; i < match.size(); ++i) {
                        args[2].array_access(std::to_string(i)) = AWKValue(match[i].str());
                    }
                }

                return AWKValue(static_cast<double>(start));
            } else {
                interp.environment().RSTART() = AWKValue(0.0);
                interp.environment().RLENGTH() = AWKValue(-1.0);

                if (args.size() >= 3) {
                    args[2].array_clear();
                }
                return AWKValue(0.0);
            }

        } catch (...) {
            interp.environment().RSTART() = AWKValue(0.0);
            interp.environment().RLENGTH() = AWKValue(-1.0);
            return AWKValue(0.0);
        }
    });

    // asort(source [, dest]) - Sort array values
    // Sorts the values and re-indexes with 1, 2, 3, ...
    // If dest is provided, source is unchanged and result goes to dest
    env_.register_builtin("asort", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty() || !args[0].is_array()) return AWKValue(0.0);

        auto keys = args[0].array_keys();
        std::vector<AWKValue> values;

        for (const auto& key : keys) {
            const AWKValue* val = args[0].array_get(key);
            if (val) {
                values.push_back(*val);
            }
        }

        // Sort by string value
        std::sort(values.begin(), values.end(),
                  [](const AWKValue& a, const AWKValue& b) {
                      return a.to_string() < b.to_string();
                  });

        // Determine destination array (source or dest if provided)
        AWKValue& dest = (args.size() >= 2) ? args[1] : args[0];

        // Clear the destination array
        dest.array_clear();

        // Write sorted values with numeric indices 1, 2, 3, ...
        for (size_t i = 0; i < values.size(); ++i) {
            dest.array_access(std::to_string(i + 1)) = values[i];
        }

        return AWKValue(static_cast<double>(values.size()));
    });

    // asorti(source [, dest]) - Sort array indices
    // Sorts the indices and stores them as values with indices 1, 2, 3, ...
    // If dest is provided, source is unchanged and result goes to dest
    env_.register_builtin("asorti", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty() || !args[0].is_array()) return AWKValue(0.0);

        auto keys = args[0].array_keys();
        std::sort(keys.begin(), keys.end());

        // Determine destination array (source or dest if provided)
        AWKValue& dest = (args.size() >= 2) ? args[1] : args[0];

        // Clear the destination array
        dest.array_clear();

        // Write sorted indices as values with numeric indices 1, 2, 3, ...
        for (size_t i = 0; i < keys.size(); ++i) {
            dest.array_access(std::to_string(i + 1)) = AWKValue(keys[i]);
        }

        return AWKValue(static_cast<double>(keys.size()));
    });

    // ========================================================================
    // Internationalization Functions (gawk extension)
    // ========================================================================
    // Full i18n support with .mo file loading

    // dcgettext(string [, domain [, category]]) - Translation from specific domain/category
    // Returns the translated string (or original string if no translation)
    env_.register_builtin("dcgettext", [](std::vector<AWKValue>& args, Interpreter& interp) {
        // dcgettext(string [, domain [, category]])
        // string: string to translate
        // domain: text domain (default: TEXTDOMAIN)
        // category: LC_* category (optional, default LC_MESSAGES)
        if (args.empty()) return AWKValue("");

        std::string msgid = args[0].to_string();
        std::string domain = (args.size() >= 2 && !args[1].to_string().empty())
            ? args[1].to_string()
            : interp.environment().TEXTDOMAIN().to_string();
        std::string category = (args.size() >= 3)
            ? args[2].to_string()
            : "LC_MESSAGES";

        return AWKValue(I18n::instance().dcgettext(msgid, domain, category));
    });

    // dcngettext(string1, string2, number [, domain [, category]]) - Plural translation
    // Chooses between singular and plural based on number
    env_.register_builtin("dcngettext", [](std::vector<AWKValue>& args, Interpreter& interp) {
        // dcngettext(singular, plural, n [, domain [, category]])
        // singular: singular form
        // plural: plural form
        // n: number to determine form
        // domain: text domain (default: TEXTDOMAIN)
        // category: LC_* category (optional, default LC_MESSAGES)
        if (args.size() < 3) return AWKValue("");

        std::string singular = args[0].to_string();
        std::string plural = args[1].to_string();
        unsigned long n = static_cast<unsigned long>(args[2].to_number());
        std::string domain = (args.size() >= 4 && !args[3].to_string().empty())
            ? args[3].to_string()
            : interp.environment().TEXTDOMAIN().to_string();
        std::string category = (args.size() >= 5)
            ? args[4].to_string()
            : "LC_MESSAGES";

        return AWKValue(I18n::instance().dcngettext(singular, plural, n, domain, category));
    });

    // bindtextdomain(directory [, domain]) - Sets directory for translation files
    // If directory is empty string, returns currently set directory for domain
    // If directory specified, stores the mapping and returns directory
    env_.register_builtin("bindtextdomain", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.empty()) return AWKValue("");
        std::string directory = args[0].to_string();
        std::string domain = (args.size() >= 2)
            ? args[1].to_string()
            : interp.environment().TEXTDOMAIN().to_string();
        if (directory.empty()) {
            // Empty directory: query current binding
            return AWKValue(interp.get_textdomain_directory(domain));
        }
        // Directory specified: store and return
        return AWKValue(interp.bind_textdomain(domain, directory));
    });
}

} // namespace awk
