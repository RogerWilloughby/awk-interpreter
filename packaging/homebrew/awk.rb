# Homebrew Formula for AWK Interpreter
# Install: brew install --build-from-source awk.rb
# Or add tap: brew tap yourusername/awk && brew install awk

class Awk < Formula
  desc "AWK interpreter with POSIX compliance and gawk extensions"
  homepage "https://github.com/yourusername/awk"
  url "https://github.com/yourusername/awk/archive/refs/tags/v1.0.0.tar.gz"
  sha256 "REPLACE_WITH_ACTUAL_SHA256_AFTER_RELEASE"
  license "MIT"
  head "https://github.com/yourusername/awk.git", branch: "main"

  # Bottle configuration (pre-built binaries)
  # bottle do
  #   sha256 cellar: :any_skip_relocation, arm64_sonoma: "..."
  #   sha256 cellar: :any_skip_relocation, ventura: "..."
  #   sha256 cellar: :any_skip_relocation, monterey: "..."
  #   sha256 cellar: :any_skip_relocation, x86_64_linux: "..."
  # end

  depends_on "cmake" => :build

  # Conflict with system awk - use different name or keg-only
  conflicts_with "gawk", because: "both install `awk` binary"

  def install
    system "cmake", "-S", ".", "-B", "build",
                    "-DCMAKE_BUILD_TYPE=Release",
                    "-DBUILD_TESTS=OFF",
                    *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"

    # Install documentation
    doc.install "README.md", "CHANGELOG.md", "LICENSE"
    doc.install Dir["docs/*.md"]
  end

  def caveats
    <<~EOS
      This AWK interpreter is installed as 'awk'.

      If you have gawk or another AWK installed, you may need to
      use the full path or adjust your PATH.

      Quick start:
        awk 'BEGIN { print "Hello, World!" }'
        awk -F, '{ print $1 }' data.csv

      Documentation is available at:
        #{doc}
    EOS
  end

  test do
    # Test basic functionality
    assert_equal "Hello, World!\n", shell_output("#{bin}/awk 'BEGIN { print \"Hello, World!\" }'")

    # Test field processing
    assert_equal "b\n", pipe_output("#{bin}/awk -F, '{ print $2 }'", "a,b,c")

    # Test math
    assert_equal "8\n", shell_output("#{bin}/awk 'BEGIN { print 2^3 }'")

    # Test string functions
    assert_equal "5\n", shell_output("#{bin}/awk 'BEGIN { print length(\"hello\") }'")

    # Test regex
    assert_equal "matched\n", shell_output("#{bin}/awk 'BEGIN { if (\"test\" ~ /es/) print \"matched\" }'")
  end
end
