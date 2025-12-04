// ============================================================================
// interpreter_builtins_math.cpp - Mathematical Built-in Functions
// ============================================================================

#include "awk/interpreter.hpp"
#include <cmath>
#include <random>
#include <ctime>

namespace awk {

void Interpreter::register_math_builtins() {
    env_.register_builtin("sin", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::sin(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("cos", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::cos(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("atan2", [](std::vector<AWKValue>& args, Interpreter&) {
        double y = args.size() > 0 ? args[0].to_number() : 0.0;
        double x = args.size() > 1 ? args[1].to_number() : 0.0;
        return AWKValue(std::atan2(y, x));
    });

    env_.register_builtin("exp", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::exp(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("log", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::log(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("sqrt", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::sqrt(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("int", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::trunc(args.empty() ? 0.0 : args[0].to_number()));
    });

    static std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    static std::uniform_real_distribution<> dist(0.0, 1.0);

    env_.register_builtin("rand", [](std::vector<AWKValue>&, Interpreter&) {
        return AWKValue(dist(rng));
    });

    env_.register_builtin("srand", [](std::vector<AWKValue>& args, Interpreter&) {
        unsigned seed = args.empty()
            ? static_cast<unsigned>(std::time(nullptr))
            : static_cast<unsigned>(args[0].to_number());
        rng.seed(seed);
        return AWKValue(static_cast<double>(seed));
    });

    // Extended mathematical functions
    env_.register_builtin("atan", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::atan(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("tan", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::tan(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("asin", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::asin(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("acos", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::acos(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("sinh", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::sinh(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("cosh", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::cosh(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("tanh", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::tanh(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("log10", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::log10(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("log2", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::log2(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("ceil", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::ceil(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("floor", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::floor(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("round", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::round(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("abs", [](std::vector<AWKValue>& args, Interpreter&) {
        return AWKValue(std::abs(args.empty() ? 0.0 : args[0].to_number()));
    });

    env_.register_builtin("fmod", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.size() < 2) return AWKValue(0.0);
        return AWKValue(std::fmod(args[0].to_number(), args[1].to_number()));
    });

    env_.register_builtin("pow", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.size() < 2) return AWKValue(0.0);
        return AWKValue(std::pow(args[0].to_number(), args[1].to_number()));
    });

    env_.register_builtin("min", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(0.0);
        double result = args[0].to_number();
        for (size_t i = 1; i < args.size(); ++i) {
            double val = args[i].to_number();
            if (val < result) result = val;
        }
        return AWKValue(result);
    });

    env_.register_builtin("max", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(0.0);
        double result = args[0].to_number();
        for (size_t i = 1; i < args.size(); ++i) {
            double val = args[i].to_number();
            if (val > result) result = val;
        }
        return AWKValue(result);
    });
}

} // namespace awk
