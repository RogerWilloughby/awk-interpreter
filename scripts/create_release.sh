#!/bin/bash
# Release creation script for AWK Interpreter
# Usage: ./scripts/create_release.sh <version>
# Example: ./scripts/create_release.sh 1.0.0

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 1.0.0"
    exit 1
fi

VERSION="$1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
RELEASE_DIR="$PROJECT_DIR/release"

echo "========================================"
echo "AWK Interpreter Release Builder"
echo "Version: $VERSION"
echo "========================================"

cd "$PROJECT_DIR"

# Validate version format
if ! [[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Error: Version must be in format X.Y.Z"
    exit 1
fi

# Check for uncommitted changes
if ! git diff --quiet HEAD 2>/dev/null; then
    echo "Warning: You have uncommitted changes!"
    read -p "Continue anyway? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Create release directory
echo ""
echo "Creating release directory..."
rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

# Build release
echo ""
echo "Building release..."
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -DAWK_ENABLE_LTO=ON

cmake --build build --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Create packages
echo ""
echo "Creating packages..."
cd build

# Create tar.gz
cpack -G TGZ
mv awk-*.tar.gz "$RELEASE_DIR/"

# Create source archive
cd "$PROJECT_DIR"
git archive --format=tar.gz --prefix="awk-$VERSION/" HEAD > "$RELEASE_DIR/awk-$VERSION-source.tar.gz"

# Generate checksums
echo ""
echo "Generating checksums..."
cd "$RELEASE_DIR"
sha256sum *.tar.gz > checksums.sha256

# Show results
echo ""
echo "========================================"
echo "Release packages created in: $RELEASE_DIR"
echo ""
ls -la "$RELEASE_DIR"
echo ""
echo "Checksums:"
cat checksums.sha256
echo ""
echo "========================================"
echo ""
echo "Next steps:"
echo "1. Test the release packages"
echo "2. Create git tag: git tag -a v$VERSION -m 'Release $VERSION'"
echo "3. Push tag: git push origin v$VERSION"
echo "4. Create GitHub release with these files"
echo "========================================"
