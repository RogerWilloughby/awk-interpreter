// ============================================================================
// platform.hpp - Cross-platform utilities
// ============================================================================

#pragma once

#include <string>
#include <cstring>
#include <cerrno>

namespace awk {

// Thread-safe, warning-free strerror wrapper
inline std::string safe_strerror(int errnum) {
#ifdef _WIN32
    char buf[256];
    if (strerror_s(buf, sizeof(buf), errnum) == 0) {
        return std::string(buf);
    }
    return "Unknown error";
#else
    return std::strerror(errnum);
#endif
}

} // namespace awk
