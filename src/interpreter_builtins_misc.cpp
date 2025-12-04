// ============================================================================
// interpreter_builtins_misc.cpp - Time, Bit, and Type Functions
// ============================================================================

#include "awk/interpreter.hpp"
#include <ctime>
#include <cstdio>

#ifdef _MSC_VER
#pragma warning(disable: 4996)  // Disable MSVC warnings for sscanf/gmtime
#endif

namespace awk {

// ============================================================================
// Time Functions
// ============================================================================

void Interpreter::register_time_builtins() {
    env_.register_builtin("systime", [](std::vector<AWKValue>&, Interpreter&) {
        return AWKValue(static_cast<double>(std::time(nullptr)));
    });

    env_.register_builtin("mktime", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(-1.0);

        std::string datespec = args[0].to_string();

        int year, month, day, hour, min, sec;
        int dst = -1;

        int n = std::sscanf(datespec.c_str(), "%d %d %d %d %d %d %d",
                           &year, &month, &day, &hour, &min, &sec, &dst);

        if (n < 6) return AWKValue(-1.0);

        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = min;
        tm.tm_sec = sec;
        tm.tm_isdst = dst;

        std::time_t t = std::mktime(&tm);
        return AWKValue(static_cast<double>(t));
    });

    env_.register_builtin("strftime", [](std::vector<AWKValue>& args, Interpreter&) {
        std::string format = args.empty() ? "%a %b %e %H:%M:%S %Z %Y" : args[0].to_string();
        std::time_t timestamp = args.size() > 1
            ? static_cast<std::time_t>(args[1].to_number())
            : std::time(nullptr);
        bool utc = args.size() > 2 && args[2].to_bool();

        std::tm* tm = utc ? std::gmtime(&timestamp) : std::localtime(&timestamp);
        if (!tm) return AWKValue("");

        char buffer[256];
        if (std::strftime(buffer, sizeof(buffer), format.c_str(), tm) == 0) {
            return AWKValue("");
        }

        return AWKValue(std::string(buffer));
    });
}

// ============================================================================
// Bit Functions
// ============================================================================

void Interpreter::register_bit_builtins() {
    env_.register_builtin("and", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.size() < 2) return AWKValue(0.0);
        auto a = static_cast<unsigned long long>(args[0].to_number());
        auto b = static_cast<unsigned long long>(args[1].to_number());
        return AWKValue(static_cast<double>(a & b));
    });

    env_.register_builtin("or", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.size() < 2) return AWKValue(0.0);
        auto a = static_cast<unsigned long long>(args[0].to_number());
        auto b = static_cast<unsigned long long>(args[1].to_number());
        return AWKValue(static_cast<double>(a | b));
    });

    env_.register_builtin("xor", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.size() < 2) return AWKValue(0.0);
        auto a = static_cast<unsigned long long>(args[0].to_number());
        auto b = static_cast<unsigned long long>(args[1].to_number());
        return AWKValue(static_cast<double>(a ^ b));
    });

    env_.register_builtin("lshift", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.size() < 2) return AWKValue(0.0);
        auto val = static_cast<unsigned long long>(args[0].to_number());
        auto shift = static_cast<int>(args[1].to_number());
        return AWKValue(static_cast<double>(val << shift));
    });

    env_.register_builtin("rshift", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.size() < 2) return AWKValue(0.0);
        auto val = static_cast<unsigned long long>(args[0].to_number());
        auto shift = static_cast<int>(args[1].to_number());
        return AWKValue(static_cast<double>(val >> shift));
    });

    env_.register_builtin("compl", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(0.0);
        auto val = static_cast<unsigned long long>(args[0].to_number());
        return AWKValue(static_cast<double>(~val));
    });
}

// ============================================================================
// Type Functions
// ============================================================================

void Interpreter::register_type_builtins() {
    env_.register_builtin("typeof", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue("unassigned");
        return AWKValue(args[0].type_name());
    });

    env_.register_builtin("isarray", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(0.0);
        return AWKValue(args[0].is_array() ? 1.0 : 0.0);
    });

    // mkbool(expr) - Converts expr to boolean (0 or 1)
    // Returns 1 if expr is "truthy", otherwise 0
    env_.register_builtin("mkbool", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(0.0);
        return AWKValue(args[0].to_bool() ? 1.0 : 0.0);
    });
}

} // namespace awk
