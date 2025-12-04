#ifndef AWK_VALUE_HPP
#define AWK_VALUE_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cmath>
#include <regex>

namespace awk {

// Forward declaration
class AWKValue;

// Type alias for arrays
using AWKArray = std::unordered_map<std::string, AWKValue>;

// AWK value types
enum class ValueType {
    UNINITIALIZED,  // Never assigned
    NUMBER,         // Numeric value
    STRING,         // String value
    STRNUM,         // String that looks like a number (e.g. from input)
    REGEX,          // Compiled regular expression
    ARRAY           // Associative array
};

// Main value class for all AWK values
class AWKValue {
public:
    // ========================================================================
    // Constructors
    // ========================================================================

    // Uninitialized value
    AWKValue();

    // Numeric value
    explicit AWKValue(double num);
    explicit AWKValue(int num);
    explicit AWKValue(long long num);

    // String value
    explicit AWKValue(const std::string& str);
    explicit AWKValue(std::string&& str);
    explicit AWKValue(const char* str);

    // STRNUM (string from input that might be numeric)
    static AWKValue strnum(const std::string& str);

    // Copy/Move
    AWKValue(const AWKValue& other);
    AWKValue(AWKValue&& other) noexcept;
    AWKValue& operator=(const AWKValue& other);
    AWKValue& operator=(AWKValue&& other) noexcept;

    ~AWKValue();

    // ========================================================================
    // Type queries
    // ========================================================================

    ValueType type() const { return type_; }

    bool is_uninitialized() const { return type_ == ValueType::UNINITIALIZED; }
    bool is_number() const { return type_ == ValueType::NUMBER; }
    bool is_string() const { return type_ == ValueType::STRING; }
    bool is_strnum() const { return type_ == ValueType::STRNUM; }
    bool is_regex() const { return type_ == ValueType::REGEX; }
    bool is_array() const { return type_ == ValueType::ARRAY; }

    // Is this value numeric or looks numeric?
    bool is_numeric() const {
        return type_ == ValueType::NUMBER ||
               type_ == ValueType::STRNUM ||
               type_ == ValueType::UNINITIALIZED;
    }

    // Type name for typeof()
    std::string type_name() const;

    // ========================================================================
    // Conversions (AWK semantics)
    // ========================================================================

    // Convert to number
    double to_number() const;

    // Convert to string
    std::string to_string() const;
    std::string to_string(const std::string& convfmt) const;

    // Convert to boolean (for conditions)
    bool to_bool() const;

    // ========================================================================
    // Arithmetic Operations
    // ========================================================================

    AWKValue operator+(const AWKValue& other) const;
    AWKValue operator-(const AWKValue& other) const;
    AWKValue operator*(const AWKValue& other) const;
    AWKValue operator/(const AWKValue& other) const;
    AWKValue operator%(const AWKValue& other) const;
    AWKValue power(const AWKValue& other) const;
    AWKValue operator-() const;  // Unary minus
    AWKValue operator+() const;  // Unary plus

    // Compound Assignment Helper
    AWKValue& operator+=(const AWKValue& other);
    AWKValue& operator-=(const AWKValue& other);
    AWKValue& operator*=(const AWKValue& other);
    AWKValue& operator/=(const AWKValue& other);
    AWKValue& operator%=(const AWKValue& other);

    // Increment/Decrement
    AWKValue& pre_increment();
    AWKValue& pre_decrement();
    AWKValue post_increment();
    AWKValue post_decrement();

    // ========================================================================
    // Comparisons (AWK semantics)
    // ========================================================================

    // Comparison with AWK semantics (-1, 0, 1)
    int compare(const AWKValue& other) const;

    bool operator==(const AWKValue& other) const;
    bool operator!=(const AWKValue& other) const;
    bool operator<(const AWKValue& other) const;
    bool operator>(const AWKValue& other) const;
    bool operator<=(const AWKValue& other) const;
    bool operator>=(const AWKValue& other) const;

    // ========================================================================
    // String Concatenation
    // ========================================================================

    AWKValue concatenate(const AWKValue& other) const;

    // ========================================================================
    // Array Operations
    // ========================================================================

    // Access array element (creates if needed)
    AWKValue& array_access(const std::string& key);

    // Access without creation
    const AWKValue* array_get(const std::string& key) const;

    // Check if key exists
    bool array_contains(const std::string& key) const;

    // Delete element
    void array_delete(const std::string& key);

    // Clear entire array
    void array_clear();

    // Array size
    size_t array_size() const;

    // All keys
    std::vector<std::string> array_keys() const;

    // Use as array (converts if needed)
    AWKArray& as_array();
    const AWKArray& as_array() const;

    // Create multi-dimensional array key
    static std::string make_array_key(const std::vector<AWKValue>& indices,
                                      const std::string& subsep);

    // ========================================================================
    // Regex Operations
    // ========================================================================

    // Set as regex
    void set_regex(const std::string& pattern);

    // Regex match
    bool regex_match(const std::string& text) const;

    // Get regex pattern (for IGNORECASE)
    const std::string& regex_pattern() const { return regex_pattern_; }

    // Regex for sub/gsub
    std::string regex_replace(const std::string& text,
                              const std::string& replacement,
                              bool global) const;

private:
    ValueType type_ = ValueType::UNINITIALIZED;
    double number_value_ = 0.0;
    std::string string_value_;
    std::unique_ptr<AWKArray> array_value_;
    std::shared_ptr<std::regex> regex_value_;
    std::string regex_pattern_;  // Original pattern for debugging

    // Helper functions
    void copy_from(const AWKValue& other);
    void move_from(AWKValue&& other) noexcept;

    // Convert string to number (AWK semantics)
    static double string_to_number(const std::string& str);

    // Check if string is numeric
    static bool looks_numeric(const std::string& str);

    // Convert number to string
    static std::string number_to_string(double num, const std::string& format = "%.6g");
};

// ============================================================================
// Inline implementations for performance
// ============================================================================

inline AWKValue::AWKValue() : type_(ValueType::UNINITIALIZED), number_value_(0.0) {}

inline AWKValue::AWKValue(double num) : type_(ValueType::NUMBER), number_value_(num) {}

inline AWKValue::AWKValue(int num) : type_(ValueType::NUMBER), number_value_(static_cast<double>(num)) {}

inline AWKValue::AWKValue(long long num) : type_(ValueType::NUMBER), number_value_(static_cast<double>(num)) {}

inline AWKValue::AWKValue(const char* str) : AWKValue(std::string(str)) {}

inline double AWKValue::to_number() const {
    switch (type_) {
        case ValueType::NUMBER:
            return number_value_;
        case ValueType::UNINITIALIZED:
            return 0.0;
        case ValueType::STRING:
        case ValueType::STRNUM:
            return string_to_number(string_value_);
        case ValueType::ARRAY:
        case ValueType::REGEX:
            return 0.0;
    }
    return 0.0;
}

inline bool AWKValue::to_bool() const {
    switch (type_) {
        case ValueType::NUMBER:
            return number_value_ != 0.0;
        case ValueType::UNINITIALIZED:
            return false;
        case ValueType::STRING:
        case ValueType::STRNUM:
            return !string_value_.empty();
        case ValueType::ARRAY:
            return array_value_ && !array_value_->empty();
        case ValueType::REGEX:
            return true;
    }
    return false;
}

} // namespace awk

#endif // AWK_VALUE_HPP
