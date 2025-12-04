cmake_minimum_required(VERSION 3.16)

# Project definition
project(awk
    VERSION 1.0.0
    DESCRIPTION "AWK interpreter with POSIX compliance and gawk extensions"
    HOMEPAGE_URL "https://github.com/yourusername/awk"
    LANGUAGES CXX
)

# Build options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_SHARED_LIBS "Build shared library instead of static" OFF)
option(AWK_INSTALL "Generate install target" ON)
option(AWK_ENABLE_LTO "Enable Link-Time Optimization for Release builds" ON)

# C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Output directories - use Release subdirectory for all builds
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)

# For multi-config generators (Visual Studio, Xcode) - force all configs to same directory
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/Release)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/Release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/Release)
endforeach()

# Compiler warnings and options
if(MSVC)
    add_compile_options(/W4 /WX- /utf-8)
    # Disable specific warnings that are too noisy
    add_compile_options(/wd4244 /wd4267)  # Conversion warnings
    if(CMAKE_BUILD_TYPE STREQUAL "Release" AND AWK_ENABLE_LTO)
        add_compile_options(/GL)
        add_link_options(/LTCG)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        if(AWK_ENABLE_LTO)
            add_compile_options(-flto)
            add_link_options(-flto)
        endif()
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    endif()
endif()

# Source files
set(AWK_SOURCES
    src/lexer.cpp
    src/parser.cpp
    src/value.cpp
    src/environment.cpp
    src/interpreter.cpp
    src/interpreter_eval.cpp
    src/interpreter_exec.cpp
    src/interpreter_getline.cpp
    src/interpreter_coprocess.cpp
    src/interpreter_builtins_math.cpp
    src/interpreter_builtins_string.cpp
    src/interpreter_builtins_io.cpp
    src/interpreter_builtins_misc.cpp
    src/regex_cache.cpp
    src/i18n.cpp
    src/space_invaders.cpp
)

# Header files (for IDE project generation)
set(AWK_HEADERS
    include/awk.hpp
    include/awk/ast.hpp
    include/awk/environment.hpp
    include/awk/interpreter.hpp
    include/awk/lexer.hpp
    include/awk/parser.hpp
    include/awk/token.hpp
    include/awk/value.hpp
    include/awk/i18n.hpp
)

# Library target
if(BUILD_SHARED_LIBS)
    add_library(awk_lib SHARED ${AWK_SOURCES} ${AWK_HEADERS})
    set_target_properties(awk_lib PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    if(MSVC)
        target_compile_definitions(awk_lib PRIVATE AWK_EXPORTS)
    endif()
else()
    add_library(awk_lib STATIC ${AWK_SOURCES} ${AWK_HEADERS})
endif()

target_include_directories(awk_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(awk_lib PROPERTIES
    OUTPUT_NAME "awk"
    POSITION_INDEPENDENT_CODE ON
)

# Add alias for use in subdirectories
add_library(awk::lib ALIAS awk_lib)

# Main executable
add_executable(awk_exe src/main.cpp)
target_link_libraries(awk_exe PRIVATE awk_lib)
set_target_properties(awk_exe PROPERTIES
    OUTPUT_NAME "awk"
)

# Add alias
add_executable(awk::awk ALIAS awk_exe)

# Tests
if(BUILD_TESTS)
    enable_testing()

    # Note: tests/main.cpp includes the other test files via #include
    add_executable(awk_tests
        tests/main.cpp
    )
    target_include_directories(awk_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/tests
    )
    target_link_libraries(awk_tests PRIVATE awk_lib)

    add_test(NAME awk_unit_tests COMMAND awk_tests)

    # Integration tests (if Python is available)
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_FOUND)
        add_test(
            NAME awk_integration_tests
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/run_integration_tests.py
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
endif()

# Installation
if(AWK_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install targets
    install(TARGETS awk_exe awk_lib
        EXPORT awkTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Development
    )

    # Install headers
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT Development
        FILES_MATCHING PATTERN "*.hpp"
    )

    # Install documentation
    install(FILES
        README.md
        LICENSE
        CHANGELOG.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        COMPONENT Documentation
    )

    install(DIRECTORY docs/
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs
        COMPONENT Documentation
        FILES_MATCHING PATTERN "*.md"
    )

    # Install man page (Unix only)
    if(UNIX)
        install(FILES packaging/awk.1
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
            COMPONENT Documentation
            OPTIONAL
        )
    endif()

    # CMake package config
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/awkConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/awkConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/awkConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/awk
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/awkConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/awkConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/awk
        COMPONENT Development
    )

    install(EXPORT awkTargets
        FILE awkTargets.cmake
        NAMESPACE awk::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/awk
        COMPONENT Development
    )

    # pkg-config file (Unix only)
    if(UNIX)
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/awk.pc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/awk.pc"
            @ONLY
        )
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/awk.pc"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
            COMPONENT Development
        )
    endif()
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME "awk")
set(CPACK_PACKAGE_VENDOR "AWK Interpreter Project")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AWK interpreter with POSIX compliance and gawk extensions")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AWK")
set(CPACK_PACKAGE_EXECUTABLES "awk" "AWK Interpreter")

# Source package
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    /\\.git/
    /build/
    /\\.vscode/
    /\\.idea/
    \\.gitignore
    \\.DS_Store
)

# Platform-specific CPack settings
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP;WIX")

    # NSIS specific
    set(CPACK_NSIS_DISPLAY_NAME "AWK Interpreter ${PROJECT_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "AWK Interpreter")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_HELP_LINK "https://github.com/yourusername/awk")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/yourusername/awk")
    set(CPACK_NSIS_CONTACT "your@email.com")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/packaging/windows/awk.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/packaging/windows/awk.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\awk.exe")
    set(CPACK_NSIS_MENU_LINKS
        "doc\\\\README.md" "README"
        "doc\\\\docs\\\\MANUAL.md" "User Manual"
    )

    # WiX specific
    set(CPACK_WIX_UPGRADE_GUID "A1B2C3D4-E5F6-7890-ABCD-EF1234567890")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/packaging/windows/awk.ico")
    set(CPACK_WIX_LICENSE_RTF "${CMAKE_SOURCE_DIR}/packaging/windows/license.rtf")
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/packaging/windows/banner.bmp")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/packaging/windows/dialog.bmp")

elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_DMG_VOLUME_NAME "AWK Interpreter")
    set(CPACK_DMG_FORMAT "UDBZ")

else()  # Linux
    set(CPACK_GENERATOR "DEB;RPM;TGZ")

    # DEB specific
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your@email.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "interpreters")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.17), libstdc++6 (>= 8)")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/yourusername/awk")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    # RPM specific
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
    set(CPACK_RPM_PACKAGE_URL "https://github.com/yourusername/awk")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc >= 2.17, libstdc++ >= 8")
    set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
endif()

# Component-based installation
set(CPACK_COMPONENTS_ALL Runtime Development Documentation)
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "AWK Interpreter")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "The AWK interpreter executable")
set(CPACK_COMPONENT_RUNTIME_REQUIRED ON)

set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "Development Files")
set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION "Headers and libraries for embedding AWK")
set(CPACK_COMPONENT_DEVELOPMENT_DEPENDS Runtime)

set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "User manual and API documentation")

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "AWK Interpreter ${PROJECT_VERSION} Configuration:")
message(STATUS "  Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Compiler:      ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build tests:       ${BUILD_TESTS}")
message(STATUS "  Shared library:    ${BUILD_SHARED_LIBS}")
message(STATUS "  Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  LTO enabled:       ${AWK_ENABLE_LTO}")
message(STATUS "")
