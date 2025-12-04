# Building the AWK Interpreter

This guide covers building the AWK interpreter on Windows, Linux, and macOS.

## Prerequisites

- **C++17 compatible compiler**
  - Windows: Visual Studio 2019+ or MSVC Build Tools
  - Linux: GCC 8+ or Clang 7+
  - macOS: Xcode 10+ or Clang 7+
- **CMake 3.14 or later**

## Quick Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run tests (optional)
./build/Release/awk_tests
```

## Windows (Visual Studio)

### Using Visual Studio IDE

1. Open Visual Studio
2. Select "Open a local folder" and navigate to the project directory
3. Visual Studio will automatically detect CMakeLists.txt
4. Select the Release configuration
5. Build → Build All (Ctrl+Shift+B)

### Using Command Line (Developer Command Prompt)

```batch
:: Configure with MSVC
cmake -B build -G "Visual Studio 17 2022" -A x64

:: Build Release
cmake --build build --config Release

:: The executable is at build\Release\awk.exe
```

### Using MSVC Build Tools

```batch
:: Configure
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -B build

:: Build
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release
```

## Linux

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install (optional)
sudo cmake --install build
```

### Fedora/RHEL

```bash
# Install dependencies
sudo dnf install gcc-c++ cmake

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## macOS

```bash
# Install Xcode Command Line Tools (if not already installed)
xcode-select --install

# Install CMake via Homebrew (if not already installed)
brew install cmake

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Debug | Build type: Debug, Release, RelWithDebInfo |
| `BUILD_TESTING` | ON | Build unit tests |

### Example with options

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON
cmake --build build --config Release
```

## Running Tests

### Unit Tests

```bash
# Run all unit tests
./build/Release/awk_tests

# Expected output:
# ========================================
# Running 632 tests...
# ========================================
# [PASS] Lexer_Empty_Source
# ...
# Results: 632 passed, 0 failed
# ========================================
```

### Integration Tests

```bash
cd integration_tests

# Linux/macOS
./run_tests.sh

# Windows
run_tests.bat
```

## Build Artifacts

After a successful build:

```
build/
├── Release/
│   ├── awk.exe          # Main executable (Windows)
│   ├── awk              # Main executable (Linux/macOS)
│   ├── awk_tests.exe    # Test executable (Windows)
│   ├── awk_tests        # Test executable (Linux/macOS)
│   └── awk_lib.lib      # Static library (Windows)
│   └── libawk_lib.a     # Static library (Linux/macOS)
```

## Troubleshooting

### CMake not found

Ensure CMake is in your PATH:
```bash
cmake --version
```

If not found, install CMake or add it to your PATH.

### C++17 not supported

Update your compiler:
- Windows: Install Visual Studio 2019 or later
- Linux: `sudo apt install g++-8` or later
- macOS: Update Xcode

### Build fails with regex errors

This interpreter uses `<regex>` from the C++ standard library. Some older compilers have incomplete regex support. Upgrade to:
- GCC 5+ (GCC 4.9 has broken regex)
- Clang 3.9+
- MSVC 2017+

### Permission denied on Linux/macOS

```bash
chmod +x build/Release/awk
chmod +x build/Release/awk_tests
```

## Clean Build

```bash
# Remove build directory and rebuild
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## IDE Support

### Visual Studio Code

1. Install the CMake Tools extension
2. Open the project folder
3. Press Ctrl+Shift+P → "CMake: Configure"
4. Press Ctrl+Shift+P → "CMake: Build"

### CLion

1. Open the project folder
2. CLion will automatically detect CMakeLists.txt
3. Select Release configuration
4. Build → Build Project
