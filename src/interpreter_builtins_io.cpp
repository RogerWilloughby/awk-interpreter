// ============================================================================
// interpreter_builtins_io.cpp - I/O Built-in Functions
// ============================================================================

#include "awk/interpreter.hpp"
#include <cstdlib>

namespace awk {

void Interpreter::register_io_builtins() {
    env_.register_builtin("system", [](std::vector<AWKValue>& args, Interpreter&) {
        if (args.empty()) return AWKValue(0.0);
        int result = std::system(args[0].to_string().c_str());
        return AWKValue(static_cast<double>(result));
    });

    env_.register_builtin("close", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.empty()) return AWKValue(-1.0);
        std::string filename = args[0].to_string();
        return AWKValue(interp.close_file(filename) ? 0.0 : -1.0);
    });

    env_.register_builtin("fflush", [](std::vector<AWKValue>& args, Interpreter& interp) {
        if (args.empty()) {
            interp.flush_all_files();
            return AWKValue(0.0);
        }
        std::string filename = args[0].to_string();
        return AWKValue(interp.flush_file(filename) ? 0.0 : -1.0);
    });
}

} // namespace awk
