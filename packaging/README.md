# AWK Interpreter Packaging

This directory contains packaging configurations for various platforms and package managers.

## Quick Reference

| Platform | Package Manager | Directory |
|----------|-----------------|-----------|
| Windows | NSIS/WiX installer | `windows/` |
| macOS | Homebrew | `homebrew/` |
| Arch Linux | AUR/PKGBUILD | `arch/` |
| Debian/Ubuntu | APT | `linux/debian/` |
| Fedora/RHEL | RPM | `linux/awk.spec` |
| Cross-platform | vcpkg | `vcpkg/` |
| Cross-platform | Conan | `conan/` |

---

## Building Packages

### Using CPack (Recommended)

CPack is integrated into CMake and can generate multiple package formats:

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Create packages
cd build

# Windows
cpack -G NSIS    # Creates .exe installer
cpack -G WIX     # Creates .msi installer
cpack -G ZIP     # Creates .zip archive

# Linux
cpack -G DEB     # Creates .deb package
cpack -G RPM     # Creates .rpm package
cpack -G TGZ     # Creates .tar.gz archive

# macOS
cpack -G DragNDrop  # Creates .dmg
cpack -G TGZ        # Creates .tar.gz

# Source package
cpack --config CPackSourceConfig.cmake
```

---

## Platform-Specific Instructions

### Windows

#### Option 1: CPack with NSIS
Requires [NSIS](https://nsis.sourceforge.io/) to be installed.

```batch
cd build
cpack -G NSIS -C Release
```

#### Option 2: Custom NSIS Script
```batch
cd packaging\windows
build_installer.bat
```

#### Option 3: WiX Toolset
Requires [WiX Toolset](https://wixtoolset.org/).

```batch
cd build
cpack -G WIX -C Release
```

### macOS (Homebrew)

#### Local Installation
```bash
brew install --build-from-source packaging/homebrew/awk.rb
```

#### Creating a Tap
1. Create a GitHub repo named `homebrew-awk`
2. Copy `awk.rb` to the repo
3. Update the URL and SHA256
4. Users install with:
```bash
brew tap yourusername/awk
brew install awk
```

### Arch Linux (AUR)

#### Local Build
```bash
cd packaging/arch
makepkg -si
```

#### Publishing to AUR
1. Create AUR account
2. Clone AUR repo: `git clone ssh://aur@aur.archlinux.org/awk-interpreter.git`
3. Copy PKGBUILD and .SRCINFO
4. Update checksums: `updpkgsums`
5. Generate .SRCINFO: `makepkg --printsrcinfo > .SRCINFO`
6. Commit and push

### Debian/Ubuntu

#### Build .deb Package
```bash
# Using CPack
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build && cpack -G DEB

# Using debuild (requires devscripts)
cp -r packaging/linux/debian .
debuild -us -uc -b
```

#### Adding to PPA
1. Create Launchpad account
2. Set up PPA
3. Upload source package with dput

### Fedora/RHEL (RPM)

#### Build .rpm Package
```bash
# Using CPack
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build && cpack -G RPM

# Using rpmbuild
rpmbuild -ba packaging/linux/awk.spec
```

### vcpkg

#### Local Installation
```bash
# Add as overlay port
vcpkg install awk-interpreter --overlay-ports=packaging/vcpkg

# Or copy to vcpkg ports directory
cp -r packaging/vcpkg $VCPKG_ROOT/ports/awk-interpreter
vcpkg install awk-interpreter
```

#### Submitting to vcpkg Registry
1. Fork microsoft/vcpkg
2. Add port to `ports/awk-interpreter/`
3. Add version to `versions/a-/awk-interpreter.json`
4. Create pull request

### Conan

#### Local Usage
```bash
cd packaging/conan
conan create . --build=missing
```

#### Publishing to Conan Center
1. Fork conan-io/conan-center-index
2. Add recipe to `recipes/awk-interpreter/`
3. Create pull request

---

## Package Contents

All packages include:

| Component | Description | Install Location |
|-----------|-------------|------------------|
| `awk` executable | Main interpreter | `/usr/bin/` or `bin/` |
| `libawk` library | Static library | `/usr/lib/` or `lib/` |
| Headers | C++ API headers | `/usr/include/awk/` |
| Documentation | README, manual | `/usr/share/doc/awk/` |
| CMake config | Find module | `/usr/lib/cmake/awk/` |
| pkg-config | .pc file | `/usr/lib/pkgconfig/` |

---

## Version Checklist

When releasing a new version:

1. Update version in `CMakeLists.txt`
2. Update `CHANGELOG.md`
3. Update package files:
   - `packaging/homebrew/awk.rb` (version, sha256)
   - `packaging/arch/PKGBUILD` (pkgver)
   - `packaging/arch/.SRCINFO` (pkgver)
   - `packaging/linux/debian/changelog` (version, date)
   - `packaging/linux/awk.spec` (Version)
   - `packaging/vcpkg/vcpkg.json` (version)
   - `packaging/conan/conanfile.py` (version)
4. Create git tag: `git tag -a v1.x.x -m "Release 1.x.x"`
5. Push tag: `git push origin v1.x.x`
6. GitHub Actions will create release packages

---

## Testing Packages

Before releasing, test packages on clean systems:

### Windows
```powershell
# Test installer
.\awk-1.0.0-setup.exe /S
awk --version
awk 'BEGIN { print "test" }'
```

### Linux
```bash
# Test deb
sudo dpkg -i awk-1.0.0-amd64.deb
awk --version

# Test rpm
sudo rpm -i awk-1.0.0-x86_64.rpm
awk --version
```

### macOS
```bash
brew install --build-from-source awk.rb
awk --version
```
