# vcpkg portfile for awk-interpreter

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO yourusername/awk
    REF v${VERSION}
    SHA512 0  # Replace with actual SHA512 after release
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTS=OFF
        -DAWK_ENABLE_LTO=OFF
)

vcpkg_cmake_build()

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME awk
    CONFIG_PATH lib/cmake/awk
)

# Install tools if requested
if("tools" IN_LIST FEATURES)
    vcpkg_copy_tools(TOOL_NAMES awk AUTO_CLEAN)
endif()

# Remove duplicate files
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Install license and documentation
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

file(INSTALL "${SOURCE_PATH}/README.md"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
)
