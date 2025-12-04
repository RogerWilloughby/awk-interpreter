// Minimal test framework for AWK unit tests
#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace test {

struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

class TestRunner {
public:
    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }

    void add_test(const std::string& name, std::function<void()> func) {
        tests_.push_back({name, func});
    }

    int run_all() {
        int passed = 0;
        int failed = 0;

        std::cout << "\n========================================\n";
        std::cout << "Running " << tests_.size() << " tests...\n";
        std::cout << "========================================\n\n";

        for (auto& [name, func] : tests_) {
            current_test_ = name;
            test_failed_ = false;
            try {
                func();
                if (!test_failed_) {
                    std::cout << "[PASS] " << name << "\n";
                    passed++;
                } else {
                    failed++;
                }
            } catch (const std::exception& e) {
                std::cout << "[FAIL] " << name << " - Exception: " << e.what() << "\n";
                failed++;
            } catch (...) {
                std::cout << "[FAIL] " << name << " - Unknown exception\n";
                failed++;
            }
        }

        std::cout << "\n========================================\n";
        std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
        std::cout << "========================================\n";

        return failed > 0 ? 1 : 0;
    }

    void fail(const std::string& msg) {
        std::cout << "[FAIL] " << current_test_ << " - " << msg << "\n";
        test_failed_ = true;
    }

private:
    std::vector<std::pair<std::string, std::function<void()>>> tests_;
    std::string current_test_;
    bool test_failed_ = false;
};

// Macros for tests
#define TEST(name) \
    void test_##name(); \
    struct TestRegister_##name { \
        TestRegister_##name() { \
            test::TestRunner::instance().add_test(#name, test_##name); \
        } \
    } test_register_##name; \
    void test_##name()

#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::ostringstream ss; \
            ss << "ASSERT_TRUE failed: " << #expr << " at line " << __LINE__; \
            test::TestRunner::instance().fail(ss.str()); \
            return; \
        } \
    } while(0)

#define ASSERT_FALSE(expr) \
    do { \
        if (expr) { \
            std::ostringstream ss; \
            ss << "ASSERT_FALSE failed: " << #expr << " at line " << __LINE__; \
            test::TestRunner::instance().fail(ss.str()); \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (_a != _b) { \
            std::ostringstream ss; \
            ss << "ASSERT_EQ failed: " << #a << " != " << #b << " at line " << __LINE__; \
            test::TestRunner::instance().fail(ss.str()); \
            return; \
        } \
    } while(0)

#define ASSERT_NE(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (_a == _b) { \
            std::ostringstream ss; \
            ss << "ASSERT_NE failed: " << #a << " == " << #b << " at line " << __LINE__; \
            test::TestRunner::instance().fail(ss.str()); \
            return; \
        } \
    } while(0)

#define ASSERT_DOUBLE_EQ(a, b) \
    do { \
        double _a = (a); \
        double _b = (b); \
        double diff = (_a > _b) ? (_a - _b) : (_b - _a); \
        if (diff > 1e-9) { \
            std::ostringstream ss; \
            ss << "ASSERT_DOUBLE_EQ failed: " << #a << " != " << #b \
               << " (" << _a << " != " << _b << ") at line " << __LINE__; \
            test::TestRunner::instance().fail(ss.str()); \
            return; \
        } \
    } while(0)

#define ASSERT_THROWS(expr) \
    do { \
        bool threw = false; \
        try { expr; } catch (...) { threw = true; } \
        if (!threw) { \
            std::ostringstream ss; \
            ss << "ASSERT_THROWS failed: " << #expr << " did not throw at line " << __LINE__; \
            test::TestRunner::instance().fail(ss.str()); \
            return; \
        } \
    } while(0)

#define ASSERT_NO_THROW(expr) \
    do { \
        try { expr; } catch (const std::exception& e) { \
            std::ostringstream ss; \
            ss << "ASSERT_NO_THROW failed: " << #expr << " threw: " << e.what() << " at line " << __LINE__; \
            test::TestRunner::instance().fail(ss.str()); \
            return; \
        } catch (...) { \
            std::ostringstream ss; \
            ss << "ASSERT_NO_THROW failed: " << #expr << " threw unknown exception at line " << __LINE__; \
            test::TestRunner::instance().fail(ss.str()); \
            return; \
        } \
    } while(0)

#define RUN_ALL_TESTS() test::TestRunner::instance().run_all()

} // namespace test

#endif // TEST_FRAMEWORK_HPP
