from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get
import os


class AwkInterpreterConan(ConanFile):
    name = "awk-interpreter"
    version = "1.0.0"
    license = "MIT"
    author = "Your Name <your@email.com>"
    url = "https://github.com/yourusername/awk"
    description = "AWK interpreter with POSIX compliance and gawk extensions"
    topics = ("awk", "interpreter", "text-processing", "scripting")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_tools": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "build_tools": True,
    }
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "LICENSE", "README.md"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.variables["BUILD_TESTS"] = False
        tc.variables["AWK_ENABLE_LTO"] = self.settings.build_type == "Release"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["awk"]
        self.cpp_info.set_property("cmake_file_name", "awk")
        self.cpp_info.set_property("cmake_target_name", "awk::lib")
        self.cpp_info.set_property("pkg_config_name", "awk")

        if self.options.build_tools:
            bin_folder = os.path.join(self.package_folder, "bin")
            self.env_info.PATH.append(bin_folder)

        # Link with standard library
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread"]
