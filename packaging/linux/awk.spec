Name:           awk-interpreter
Version:        1.0.0
Release:        1%{?dist}
Summary:        AWK interpreter with POSIX compliance and gawk extensions

License:        MIT
URL:            https://github.com/yourusername/awk
Source0:        %{url}/archive/refs/tags/v%{version}.tar.gz#/%{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc-c++ >= 8
BuildRequires:  make

Provides:       awk
Conflicts:      gawk mawk

%description
A complete AWK interpreter implemented in C++17 with:
- Full POSIX AWK compliance
- Extensive gawk (GNU AWK) extensions
- 50+ built-in functions
- Cross-platform support

Features include BEGINFILE/ENDFILE, switch/case, namespaces,
coprocesses, and comprehensive regex support.

%package devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Development files for embedding the AWK interpreter in C++ applications.
Includes header files, static library, and CMake configuration.

%prep
%autosetup -n awk-%{version}

%build
%cmake -DBUILD_TESTS=OFF -DAWK_ENABLE_LTO=ON
%cmake_build

%install
%cmake_install

# Install documentation
install -Dm644 README.md %{buildroot}%{_docdir}/%{name}/README.md
install -Dm644 CHANGELOG.md %{buildroot}%{_docdir}/%{name}/CHANGELOG.md
install -Dm644 docs/*.md -t %{buildroot}%{_docdir}/%{name}/

%check
# Basic functionality test
%{buildroot}%{_bindir}/awk 'BEGIN { print "test" }' | grep -q "test"

%files
%license LICENSE
%doc README.md CHANGELOG.md
%{_bindir}/awk
%{_docdir}/%{name}/

%files devel
%{_includedir}/awk/
%{_includedir}/awk.hpp
%{_libdir}/libawk.a
%{_libdir}/cmake/awk/
%{_libdir}/pkgconfig/awk.pc

%changelog
* Mon Jan 01 2024 Your Name <your@email.com> - 1.0.0-1
- Initial package
