# Common Ambient Variables:
#   CURRENT_BUILDTREES_DIR    = ${VCPKG_ROOT_DIR}\buildtrees\${PORT}
#   CURRENT_PACKAGES_DIR      = ${VCPKG_ROOT_DIR}\packages\${PORT}_${TARGET_TRIPLET}
#   CURRENT_PORT_DIR          = ${VCPKG_ROOT_DIR}\ports\${PORT}
#   PORT                      = current port name (zlib, etc)
#   TARGET_TRIPLET            = current triplet (x86-windows, x64-windows-static, etc)
#   VCPKG_CRT_LINKAGE         = C runtime linkage type (static, dynamic)
#   VCPKG_LIBRARY_LINKAGE     = target library linkage type (static, dynamic)
#   VCPKG_ROOT_DIR            = <C:\path\to\current\vcpkg>
#   VCPKG_TARGET_ARCHITECTURE = target architecture (x64, x86, arm)
#

include(vcpkg_common_functions)
vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO HOST-Oman/libraqm
    REF v0.5.0
    SHA512 7fcde6b7e69d1e465323f42786647a62e257484f182908375263f056cccedc8c764d89be7b2d4e3f73fcbb9eff040a0101ac779a973e8bbe58a8c2e32a23f5af
HEAD_REF master)

file(COPY ${CURRENT_PORT_DIR}/FindFribidi.cmake DESTINATION ${SOURCE_PATH})
file(COPY ${CURRENT_PORT_DIR}/FindHarfbuzz.cmake DESTINATION ${SOURCE_PATH})
file(COPY ${CURRENT_PORT_DIR}/CMakeLists.txt DESTINATION ${SOURCE_PATH})

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
    -DCURRENT_PACKAGES_DIR=${CURRENT_PACKAGES_DIR}
)

vcpkg_install_cmake()

vcpkg_copy_pdbs()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/libraqm RENAME copyright)

# Post-build test for cmake libraries
# vcpkg_test_cmake(PACKAGE_NAME libraqm)
