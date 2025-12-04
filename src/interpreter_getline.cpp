// ============================================================================
// interpreter_getline.cpp - Getline Functions
// ============================================================================

#include "awk/interpreter.hpp"
#include <cstdio>
#include <cerrno>
#include <cstring>

namespace awk {

// ============================================================================
// Getline Expression Evaluation
// ============================================================================

AWKValue Interpreter::evaluate(GetlineExpr& expr) {
    // Complete getline implementation for all variants:
    // 1. getline              - from stdin into $0, NR++
    // 2. getline var          - from stdin into var, NR++
    // 3. getline < file       - from file into $0
    // 4. getline var < file   - from file into var
    // 5. command | getline    - from pipe into $0
    // 6. command | getline var - from pipe into var
    // 7. command |& getline   - from coprocess into $0 (gawk extension)
    // 8. command |& getline var - from coprocess into var (gawk extension)

    int result;

    if (expr.command) {
        std::string cmd = evaluate(*expr.command).to_string();

        if (expr.coprocess) {
            // Variant 7/8: command |& getline [var] (coprocess)
            result = getline_from_coprocess(cmd, expr.variable.get());
        } else {
            // Variant 5/6: command | getline [var]
            FILE* pipe = get_input_pipe(cmd);
            if (!pipe) {
                return AWKValue(-1.0);
            }
            result = getline_from_pipe(pipe, expr.variable.get(), false);
        }
    }
    else if (expr.file) {
        // Variant 3/4: getline [var] < file
        std::string filename = evaluate(*expr.file).to_string();
        std::istream* stream = get_input_file(filename);
        if (!stream) {
            return AWKValue(-1.0);
        }
        result = getline_from_stream(*stream, expr.variable.get(), false);
    }
    else {
        // Variant 1/2: getline [var] (from stdin)
        result = getline_from_stream(std::cin, expr.variable.get(), true);
    }

    return AWKValue(static_cast<double>(result));
}

// ============================================================================
// Input File Management
// ============================================================================

std::istream* Interpreter::get_input_file(const std::string& filename) {
    // Special files (gawk compatibility)
    if (filename == "/dev/stdin" || filename == "-") {
        return &std::cin;
    }

    // Already open?
    auto it = input_files_.find(filename);
    if (it != input_files_.end()) {
        return it->second.get();
    }

    // Open new file
    auto file = std::make_unique<std::ifstream>(filename);
    if (!file->is_open()) {
        *error_ << "awk: can't open file " << filename << " for reading: " << std::strerror(errno) << "\n";
        return nullptr;
    }

    std::istream* ptr = file.get();
    input_files_[filename] = std::move(file);
    return ptr;
}

// ============================================================================
// Input Pipe Management
// ============================================================================

FILE* Interpreter::get_input_pipe(const std::string& command) {
    // Already open?
    auto it = input_pipes_.find(command);
    if (it != input_pipes_.end()) {
        return it->second;
    }

    // Open new pipe
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif

    if (!pipe) {
        *error_ << "awk: can't open pipe from command: " << command << ": " << std::strerror(errno) << "\n";
        return nullptr;
    }

    input_pipes_[command] = pipe;
    return pipe;
}

// ============================================================================
// Getline from Stream
// ============================================================================

int Interpreter::getline_from_stream(std::istream& stream, Expr* variable, bool update_nr) {
    std::string rs = env_.RS().to_string();
    std::string line;

    if (rs.empty() || rs == "\n") {
        // Standard: read line by line
        if (!std::getline(stream, line)) {
            return stream.eof() ? -1 : 0;  // -1 = EOF, 0 = error
        }
    } else if (rs.length() == 1) {
        // Single character as RS
        char delimiter = rs[0];
        line.clear();
        char c;
        bool read_any = false;
        while (stream.get(c)) {
            read_any = true;
            if (c == delimiter) break;
            line += c;
        }
        if (!read_any && stream.eof()) {
            return -1;  // EOF
        }
    } else {
        // Multi-character RS (simplified: line by line)
        if (!std::getline(stream, line)) {
            return stream.eof() ? -1 : 0;
        }
    }

    // Store value
    if (variable) {
        AWKValue& var = get_lvalue(*variable);
        var = AWKValue::strnum(line);
    } else {
        // Store in $0 and update fields
        set_record(line);
    }

    // Update NR if desired (only for stdin)
    if (update_nr) {
        double nr = env_.NR().to_number() + 1;
        env_.NR() = AWKValue(nr);
    }

    return 1;  // Success
}

// ============================================================================
// Getline from Pipe
// ============================================================================

int Interpreter::getline_from_pipe(FILE* pipe, Expr* variable, bool update_nr) {
    if (!pipe) return -1;

    std::string rs = env_.RS().to_string();
    std::string line;

    if (rs.empty() || rs == "\n") {
        // Read line by line
        char buffer[4096];
        if (!fgets(buffer, sizeof(buffer), pipe)) {
            return feof(pipe) ? -1 : 0;
        }
        line = buffer;
        // Remove trailing newline
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }
        // Windows: also remove \r
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
    } else if (rs.length() == 1) {
        // Single character as RS
        char delimiter = rs[0];
        line.clear();
        int c;
        bool read_any = false;
        while ((c = fgetc(pipe)) != EOF) {
            read_any = true;
            if (c == delimiter) break;
            line += static_cast<char>(c);
        }
        if (!read_any && feof(pipe)) {
            return -1;
        }
    } else {
        // Multi-character RS (simplified)
        char buffer[4096];
        if (!fgets(buffer, sizeof(buffer), pipe)) {
            return feof(pipe) ? -1 : 0;
        }
        line = buffer;
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
    }

    // Store value
    if (variable) {
        AWKValue& var = get_lvalue(*variable);
        var = AWKValue::strnum(line);
    } else {
        set_record(line);
    }

    // Update NR if desired
    if (update_nr) {
        double nr = env_.NR().to_number() + 1;
        env_.NR() = AWKValue(nr);
    }

    return 1;  // Success
}

} // namespace awk
