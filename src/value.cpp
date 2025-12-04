#include "awk/value.hpp"
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

namespace awk {

// ============================================================================
// Constructors
// ============================================================================

AWKValue::AWKValue(const std::string& str)
    : type_(ValueType::STRING)
    , number_value_(0.0)
    , string_value_(str) {}

AWKValue::AWKValue(std::string&& str)
    : type_(ValueType::STRING)
    , number_value_(0.0)
    , string_value_(std::move(str)) {}

AWKValue AWKValue::strnum(const std::string& str) {
    AWKValue v;
    v.type_ = ValueType::STRNUM;
    v.string_value_ = str;
    v.number_value_ = string_to_number(str);
    return v;
}

AWKValue::AWKValue(const AWKValue& other) {
    copy_from(other);
}

AWKValue::AWKValue(AWKValue&& other) noexcept {
    move_from(std::move(other));
}

AWKValue& AWKValue::operator=(const AWKValue& other) {
    if (this != &other) {
        copy_from(other);
    }
    return *this;
}

AWKValue& AWKValue::operator=(AWKValue&& other) noexcept {
    if (this != &other) {
        move_from(std::move(other));
    }
    return *this;
}

AWKValue::~AWKValue() = default;

void AWKValue::copy_from(const AWKValue& other) {
    type_ = other.type_;
    number_value_ = other.number_value_;
    string_value_ = other.string_value_;

    if (other.array_value_) {
        array_value_ = std::make_unique<AWKArray>(*other.array_value_);
    } else {
        array_value_.reset();
    }

    regex_value_ = other.regex_value_;
    regex_pattern_ = other.regex_pattern_;
}

void AWKValue::move_from(AWKValue&& other) noexcept {
    type_ = other.type_;
    number_value_ = other.number_value_;
    string_value_ = std::move(other.string_value_);
    array_value_ = std::move(other.array_value_);
    regex_value_ = std::move(other.regex_value_);
    regex_pattern_ = std::move(other.regex_pattern_);

    other.type_ = ValueType::UNINITIALIZED;
    other.number_value_ = 0.0;
}

// ============================================================================
// Type Information
// ============================================================================

std::string AWKValue::type_name() const {
    switch (type_) {
        case ValueType::UNINITIALIZED: return "unassigned";
        case ValueType::NUMBER: return "number";
        case ValueType::STRING: return "string";
        case ValueType::STRNUM: return "strnum";
        case ValueType::REGEX: return "regexp";
        case ValueType::ARRAY: return "array";
    }
    return "unknown";
}

// ============================================================================
// Conversions
// ============================================================================

std::string AWKValue::to_string() const {
    return to_string("%.6g");
}

std::string AWKValue::to_string(const std::string& convfmt) const {
    switch (type_) {
        case ValueType::STRING:
        case ValueType::STRNUM:
            return string_value_;

        case ValueType::NUMBER:
            return number_to_string(number_value_, convfmt);

        case ValueType::UNINITIALIZED:
            return "";

        case ValueType::ARRAY:
            return "";  // Arrays cannot be converted to strings

        case ValueType::REGEX:
            return regex_pattern_;
    }
    return "";
}

double AWKValue::string_to_number(const std::string& str) {
    if (str.empty()) return 0.0;

    const char* s = str.c_str();
    char* end;

    // Skip whitespace
    while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;

    if (*s == '\0') return 0.0;

    // Hexadecimal?
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        long long val = std::strtoll(s, &end, 16);
        return static_cast<double>(val);
    }

    // Parse normally
    double val = std::strtod(s, &end);

    // If nothing was parsed, return 0
    if (end == s) return 0.0;

    return val;
}

bool AWKValue::looks_numeric(const std::string& str) {
    if (str.empty()) return false;

    const char* s = str.c_str();

    // Skip whitespace
    while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;

    if (*s == '\0') return false;

    // Optional sign
    if (*s == '+' || *s == '-') ++s;

    if (*s == '\0') return false;

    bool has_digit = false;
    bool has_dot = false;
    bool has_exp = false;

    // Digits before the decimal point
    while (std::isdigit(static_cast<unsigned char>(*s))) {
        has_digit = true;
        ++s;
    }

    // Dezimalpunkt
    if (*s == '.') {
        has_dot = true;
        ++s;

        // Digits after the decimal point
        while (std::isdigit(static_cast<unsigned char>(*s))) {
            has_digit = true;
            ++s;
        }
    }

    if (!has_digit) return false;

    // Exponent
    if (*s == 'e' || *s == 'E') {
        has_exp = true;
        ++s;

        // Optional sign in exponent
        if (*s == '+' || *s == '-') ++s;

        // At least one digit in exponent
        if (!std::isdigit(static_cast<unsigned char>(*s))) return false;

        while (std::isdigit(static_cast<unsigned char>(*s))) ++s;
    }

    // Skip trailing whitespace
    while (*s && std::isspace(static_cast<unsigned char>(*s))) ++s;

    // Must be at end
    return *s == '\0';
}

std::string AWKValue::number_to_string(double num, const std::string& format) {
    // Ganzzahl-Optimierung
    if (std::floor(num) == num &&
        num >= std::numeric_limits<long long>::min() &&
        num <= std::numeric_limits<long long>::max()) {
        return std::to_string(static_cast<long long>(num));
    }

    // sprintf-artige Formatierung
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), format.c_str(), num);
    return buffer;
}

// ============================================================================
// Arithmetische Operationen
// ============================================================================

AWKValue AWKValue::operator+(const AWKValue& other) const {
    return AWKValue(to_number() + other.to_number());
}

AWKValue AWKValue::operator-(const AWKValue& other) const {
    return AWKValue(to_number() - other.to_number());
}

AWKValue AWKValue::operator*(const AWKValue& other) const {
    return AWKValue(to_number() * other.to_number());
}

AWKValue AWKValue::operator/(const AWKValue& other) const {
    double divisor = other.to_number();
    if (divisor == 0.0) {
        // AWK returns +inf or -inf, not an error
        double dividend = to_number();
        if (dividend > 0) return AWKValue(std::numeric_limits<double>::infinity());
        if (dividend < 0) return AWKValue(-std::numeric_limits<double>::infinity());
        return AWKValue(std::nan(""));
    }
    return AWKValue(to_number() / divisor);
}

AWKValue AWKValue::operator%(const AWKValue& other) const {
    double divisor = other.to_number();
    if (divisor == 0.0) {
        return AWKValue(std::nan(""));
    }
    return AWKValue(std::fmod(to_number(), divisor));
}

AWKValue AWKValue::power(const AWKValue& other) const {
    return AWKValue(std::pow(to_number(), other.to_number()));
}

AWKValue AWKValue::operator-() const {
    return AWKValue(-to_number());
}

AWKValue AWKValue::operator+() const {
    return AWKValue(to_number());
}

AWKValue& AWKValue::operator+=(const AWKValue& other) {
    *this = *this + other;
    return *this;
}

AWKValue& AWKValue::operator-=(const AWKValue& other) {
    *this = *this - other;
    return *this;
}

AWKValue& AWKValue::operator*=(const AWKValue& other) {
    *this = *this * other;
    return *this;
}

AWKValue& AWKValue::operator/=(const AWKValue& other) {
    *this = *this / other;
    return *this;
}

AWKValue& AWKValue::operator%=(const AWKValue& other) {
    *this = *this % other;
    return *this;
}

AWKValue& AWKValue::pre_increment() {
    double val = to_number() + 1.0;
    type_ = ValueType::NUMBER;
    number_value_ = val;
    string_value_.clear();
    return *this;
}

AWKValue& AWKValue::pre_decrement() {
    double val = to_number() - 1.0;
    type_ = ValueType::NUMBER;
    number_value_ = val;
    string_value_.clear();
    return *this;
}

AWKValue AWKValue::post_increment() {
    AWKValue old = *this;
    pre_increment();
    return old;
}

AWKValue AWKValue::post_decrement() {
    AWKValue old = *this;
    pre_decrement();
    return old;
}

// ============================================================================
// Comparisons
// ============================================================================

int AWKValue::compare(const AWKValue& other) const {
    // AWK comparison rules:
    // 1. If both are numeric (or strnum), numeric comparison
    // 2. Otherwise string comparison

    bool left_numeric = (type_ == ValueType::NUMBER ||
                         type_ == ValueType::STRNUM ||
                         type_ == ValueType::UNINITIALIZED);

    bool right_numeric = (other.type_ == ValueType::NUMBER ||
                          other.type_ == ValueType::STRNUM ||
                          other.type_ == ValueType::UNINITIALIZED);

    if (left_numeric && right_numeric) {
        double l = to_number();
        double r = other.to_number();
        if (l < r) return -1;
        if (l > r) return 1;
        return 0;
    }

    // String-Vergleich
    std::string l = to_string();
    std::string r = other.to_string();
    int cmp = l.compare(r);
    if (cmp < 0) return -1;
    if (cmp > 0) return 1;
    return 0;
}

bool AWKValue::operator==(const AWKValue& other) const {
    return compare(other) == 0;
}

bool AWKValue::operator!=(const AWKValue& other) const {
    return compare(other) != 0;
}

bool AWKValue::operator<(const AWKValue& other) const {
    return compare(other) < 0;
}

bool AWKValue::operator>(const AWKValue& other) const {
    return compare(other) > 0;
}

bool AWKValue::operator<=(const AWKValue& other) const {
    return compare(other) <= 0;
}

bool AWKValue::operator>=(const AWKValue& other) const {
    return compare(other) >= 0;
}

// ============================================================================
// String Concatenation
// ============================================================================

AWKValue AWKValue::concatenate(const AWKValue& other) const {
    return AWKValue(to_string() + other.to_string());
}

// ============================================================================
// Array-Operationen
// ============================================================================

AWKValue& AWKValue::array_access(const std::string& key) {
    if (type_ != ValueType::ARRAY) {
        type_ = ValueType::ARRAY;
        array_value_ = std::make_unique<AWKArray>();
    }
    return (*array_value_)[key];
}

const AWKValue* AWKValue::array_get(const std::string& key) const {
    if (type_ != ValueType::ARRAY || !array_value_) {
        return nullptr;
    }
    auto it = array_value_->find(key);
    if (it == array_value_->end()) {
        return nullptr;
    }
    return &it->second;
}

bool AWKValue::array_contains(const std::string& key) const {
    if (type_ != ValueType::ARRAY || !array_value_) {
        return false;
    }
    return array_value_->find(key) != array_value_->end();
}

void AWKValue::array_delete(const std::string& key) {
    if (type_ == ValueType::ARRAY && array_value_) {
        array_value_->erase(key);
    }
}

void AWKValue::array_clear() {
    if (type_ == ValueType::ARRAY && array_value_) {
        array_value_->clear();
    }
}

size_t AWKValue::array_size() const {
    if (type_ != ValueType::ARRAY || !array_value_) {
        return 0;
    }
    return array_value_->size();
}

std::vector<std::string> AWKValue::array_keys() const {
    std::vector<std::string> keys;
    if (type_ == ValueType::ARRAY && array_value_) {
        keys.reserve(array_value_->size());
        for (const auto& [key, _] : *array_value_) {
            keys.push_back(key);
        }
    }
    return keys;
}

AWKArray& AWKValue::as_array() {
    if (type_ != ValueType::ARRAY) {
        type_ = ValueType::ARRAY;
        array_value_ = std::make_unique<AWKArray>();
    }
    return *array_value_;
}

const AWKArray& AWKValue::as_array() const {
    static AWKArray empty;
    if (type_ != ValueType::ARRAY || !array_value_) {
        return empty;
    }
    return *array_value_;
}

std::string AWKValue::make_array_key(const std::vector<AWKValue>& indices,
                                     const std::string& subsep) {
    if (indices.empty()) return "";
    if (indices.size() == 1) return indices[0].to_string();

    // Pre-calculate total size to avoid reallocations
    std::vector<std::string> parts;
    parts.reserve(indices.size());
    size_t total_size = 0;
    for (const auto& idx : indices) {
        parts.push_back(idx.to_string());
        total_size += parts.back().length();
    }
    total_size += subsep.length() * (indices.size() - 1);

    // Build key with pre-allocated buffer
    std::string key;
    key.reserve(total_size);
    key = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        key += subsep;
        key += parts[i];
    }
    return key;
}

// ============================================================================
// Regex-Operationen
// ============================================================================

void AWKValue::set_regex(const std::string& pattern) {
    type_ = ValueType::REGEX;
    regex_pattern_ = pattern;
    try {
        regex_value_ = std::make_shared<std::regex>(
            pattern,
            std::regex_constants::extended
        );
    } catch (const std::regex_error&) {
        // For invalid pattern: create empty regex
        regex_value_ = std::make_shared<std::regex>();
    }
}

bool AWKValue::regex_match(const std::string& text) const {
    if (type_ == ValueType::REGEX && regex_value_) {
        return std::regex_search(text, *regex_value_);
    }
    // Als String-Pattern interpretieren
    try {
        std::regex re(to_string(), std::regex_constants::extended);
        return std::regex_search(text, re);
    } catch (...) {
        return false;
    }
}

std::string AWKValue::regex_replace(const std::string& text,
                                    const std::string& replacement,
                                    bool global) const {
    std::regex re;

    if (type_ == ValueType::REGEX && regex_value_) {
        re = *regex_value_;
    } else {
        try {
            re = std::regex(to_string(), std::regex_constants::extended);
        } catch (...) {
            return text;
        }
    }

    if (global) {
        return std::regex_replace(text, re, replacement);
    } else {
        return std::regex_replace(text, re, replacement,
                                  std::regex_constants::format_first_only);
    }
}

} // namespace awk
