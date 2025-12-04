// ============================================================================
// interpreter_coprocess.cpp - Coprocess Handling (gawk |& Extension)
// ============================================================================

#include "awk/interpreter.hpp"
#include <cstdio>
#include <cerrno>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace awk {

// ============================================================================
// Coprocess Close
// ============================================================================

int Coprocess::close() {
    int result = 0;

#ifdef _WIN32
    if (to_child) {
        fclose(to_child);
        to_child = nullptr;
    }
    if (from_child) {
        fclose(from_child);
        from_child = nullptr;
    }
    if (process_handle) {
        // Wait for process end and close handle
        WaitForSingleObject(process_handle, INFINITE);
        DWORD exit_code = 0;
        GetExitCodeProcess(process_handle, &exit_code);
        CloseHandle(process_handle);
        result = static_cast<int>(exit_code);
        process_handle = nullptr;
    }
#else
    if (to_child) {
        fclose(to_child);
        to_child = nullptr;
    }
    if (from_child) {
        fclose(from_child);
        from_child = nullptr;
    }
    if (pid > 0) {
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            result = WEXITSTATUS(status);
        }
        pid = -1;
    }
#endif

    return result;
}

// ============================================================================
// Coprocess Management
// ============================================================================

Coprocess* Interpreter::get_or_create_coprocess(const std::string& command) {
    // Already open?
    auto it = coprocesses_.find(command);
    if (it != coprocesses_.end()) {
        return it->second.get();
    }

    auto coproc = std::make_unique<Coprocess>();

#ifdef _WIN32
    // Windows: CreateProcess with pipes
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE child_stdin_read = nullptr, child_stdin_write = nullptr;
    HANDLE child_stdout_read = nullptr, child_stdout_write = nullptr;

    // Pipes for child process stdin
    if (!CreatePipe(&child_stdin_read, &child_stdin_write, &sa, 0)) {
        *error_ << "awk: can't create stdin pipe for coprocess: " << command << "\n";
        return nullptr;
    }
    SetHandleInformation(child_stdin_write, HANDLE_FLAG_INHERIT, 0);

    // Pipes for child process stdout
    if (!CreatePipe(&child_stdout_read, &child_stdout_write, &sa, 0)) {
        CloseHandle(child_stdin_read);
        CloseHandle(child_stdin_write);
        *error_ << "awk: can't create stdout pipe for coprocess: " << command << "\n";
        return nullptr;
    }
    SetHandleInformation(child_stdout_read, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdInput = child_stdin_read;
    si.hStdOutput = child_stdout_write;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    // Execute command via cmd /c
    std::string cmd_line = "cmd /c " + command;
    std::vector<char> cmd_buf(cmd_line.begin(), cmd_line.end());
    cmd_buf.push_back('\0');

    if (!CreateProcessA(
            nullptr,
            cmd_buf.data(),
            nullptr, nullptr,
            TRUE,
            0,
            nullptr, nullptr,
            &si, &pi)) {
        CloseHandle(child_stdin_read);
        CloseHandle(child_stdin_write);
        CloseHandle(child_stdout_read);
        CloseHandle(child_stdout_write);
        *error_ << "awk: can't start coprocess: " << command << "\n";
        return nullptr;
    }

    // Close unneeded handles
    CloseHandle(child_stdin_read);
    CloseHandle(child_stdout_write);
    CloseHandle(pi.hThread);

    // Create FILE* from HANDLEs
    int fd_write = _open_osfhandle(reinterpret_cast<intptr_t>(child_stdin_write), 0);
    int fd_read = _open_osfhandle(reinterpret_cast<intptr_t>(child_stdout_read), _O_RDONLY);

    coproc->to_child = _fdopen(fd_write, "w");
    coproc->from_child = _fdopen(fd_read, "r");
    coproc->process_handle = pi.hProcess;

#else
    // Unix: fork/exec with pipes
    int stdin_pipe[2];
    int stdout_pipe[2];

    if (pipe(stdin_pipe) < 0) {
        *error_ << "awk: can't create stdin pipe for coprocess: " << command << ": " << std::strerror(errno) << "\n";
        return nullptr;
    }

    if (pipe(stdout_pipe) < 0) {
        ::close(stdin_pipe[0]);
        ::close(stdin_pipe[1]);
        *error_ << "awk: can't create stdout pipe for coprocess: " << command << ": " << std::strerror(errno) << "\n";
        return nullptr;
    }

    pid_t child_pid = fork();

    if (child_pid < 0) {
        ::close(stdin_pipe[0]);
        ::close(stdin_pipe[1]);
        ::close(stdout_pipe[0]);
        ::close(stdout_pipe[1]);
        *error_ << "awk: can't fork for coprocess: " << command << ": " << std::strerror(errno) << "\n";
        return nullptr;
    }

    if (child_pid == 0) {
        // Child process
        ::close(stdin_pipe[1]);  // Close write end of stdin pipe
        ::close(stdout_pipe[0]); // Close read end of stdout pipe

        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);

        ::close(stdin_pipe[0]);
        ::close(stdout_pipe[1]);

        execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
        _exit(127);  // exec failed
    }

    // Parent process
    ::close(stdin_pipe[0]);   // Close read end of stdin pipe
    ::close(stdout_pipe[1]);  // Close write end of stdout pipe

    coproc->to_child = fdopen(stdin_pipe[1], "w");
    coproc->from_child = fdopen(stdout_pipe[0], "r");
    coproc->pid = child_pid;
#endif

    if (!coproc->to_child || !coproc->from_child) {
        *error_ << "awk: can't open streams for coprocess: " << command << "\n";
        return nullptr;
    }

    Coprocess* ptr = coproc.get();
    coprocesses_[command] = std::move(coproc);
    return ptr;
}

// ============================================================================
// Getline from Coprocess
// ============================================================================

int Interpreter::getline_from_coprocess(const std::string& command, Expr* variable) {
    Coprocess* coproc = get_or_create_coprocess(command);
    if (!coproc || !coproc->from_child) {
        return -1;
    }

    // Flush output before reading (bidirectional!)
    if (coproc->to_child) {
        fflush(coproc->to_child);
    }

    return getline_from_pipe(coproc->from_child, variable, false);
}

// ============================================================================
// Write to Coprocess
// ============================================================================

bool Interpreter::write_to_coprocess(const std::string& command, const std::string& data) {
    Coprocess* coproc = get_or_create_coprocess(command);
    if (!coproc || !coproc->to_child) {
        return false;
    }

    size_t written = fwrite(data.c_str(), 1, data.size(), coproc->to_child);
    return written == data.size();
}

// ============================================================================
// Close Coprocess
// ============================================================================

bool Interpreter::close_coprocess(const std::string& command) {
    auto it = coprocesses_.find(command);
    if (it != coprocesses_.end()) {
        // Also remove the associated output stream
        output_pipes_.erase("__coproc__" + command);
        coprocesses_.erase(it);
        return true;
    }
    return false;
}

} // namespace awk
