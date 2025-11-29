# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BINARY_DEB "OFF")
set(CPACK_BINARY_FREEBSD "OFF")
set(CPACK_BINARY_IFW "OFF")
set(CPACK_BINARY_NSIS "OFF")
set(CPACK_BINARY_RPM "OFF")
set(CPACK_BINARY_STGZ "ON")
set(CPACK_BINARY_TBZ2 "OFF")
set(CPACK_BINARY_TGZ "ON")
set(CPACK_BINARY_TXZ "OFF")
set(CPACK_BINARY_TZ "ON")
set(CPACK_BUILD_SOURCE_DIRS "/home/runner/work/supertux/supertux;/home/runner/work/supertux/supertux/_codeql_build_dir")
set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
set(CPACK_COMPONENTS_ALL "Development;Libraries;Runtime;Unspecified")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.5), libgcc1 (>= 1:4.1), libgl1-mesa-glx | libgl1, libogg0 (>= 1.1.3), libopenal0a, libphysfs-1.0-0, libsdl-image1.2 (>= 1.2.5), libsdl1.2debian (>= 1.2.10-1), libstdc++6 (>= 4.1.2), libvorbis0a (>= 1.1.2), libvorbisfile3 (>= 1.1.2), libcurl3 (>= 7.16)")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux
 SuperTux is a classic 2D jump 'n run sidescroller game in a similar
 style like the original SuperMario games. This release of SuperTux
 features 9 enemies, 26 playable levels, software and OpenGL rendering
 modes, configurable joystick and keyboard input, new music and
 completely redone graphics.
 .
 This is a development snapshot of SuperTux. It may suffer from
 critical bugs and has not been fully tested. 
 .
 Homepage: http://supertux.lethargik.org/")
set(CPACK_DEBIAN_PACKAGE_NAME "supertux2")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "/usr/local/share/cmake-3.31/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "SUPERTUX built using CMake")
set(CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE "ON")
set(CPACK_GENERATOR "TBZ2;TGZ;TXZ;TZ")
set(CPACK_IGNORE_FILES "/\\.git/;/home/runner/work/supertux/supertux/_codeql_build_dir;/\\..*")
set(CPACK_INNOSETUP_ARCHITECTURE "x64")
set(CPACK_INSTALLED_DIRECTORIES "/home/runner/work/supertux/supertux;/")
set(CPACK_INSTALL_CMAKE_PROJECTS "")
set(CPACK_INSTALL_PREFIX "/usr/local")
set(CPACK_MODULE_PATH "/home/runner/work/supertux/supertux/mk/cmake")
set(CPACK_NSIS_DISPLAY_NAME "SuperTux")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_PACKAGE_NAME "SuperTux")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OBJCOPY_EXECUTABLE "/usr/bin/objcopy")
set(CPACK_OBJDUMP_EXECUTABLE "/usr/bin/objdump")
set(CPACK_OUTPUT_CONFIG_FILE "/home/runner/work/supertux/supertux/_codeql_build_dir/CPackConfig.cmake")
set(CPACK_PACKAGE_CONTACT "SuperTux Devel Team <supertux-devel@lists.lethargik.org>")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "/home/runner/work/supertux/supertux/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jump'n'Run Game featuring Tux")
set(CPACK_PACKAGE_EXECUTABLES "supertux2;SuperTux")
set(CPACK_PACKAGE_FILE_NAME "SuperTux-copilot/improve-supertux-performance-4f7fb24-Source")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SuperTux")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "SuperTux")
set(CPACK_PACKAGE_NAME "SuperTux")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "SuperTux Devel Team")
set(CPACK_PACKAGE_VERSION "3.2")
set(CPACK_PACKAGE_VERSION_MAJOR "3")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_READELF_EXECUTABLE "/usr/bin/readelf")
set(CPACK_RESOURCE_FILE_LICENSE "/home/runner/work/supertux/supertux/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "/usr/local/share/cmake-3.31/Templates/CPack.GenericDescription.txt")
set(CPACK_RESOURCE_FILE_WELCOME "/usr/local/share/cmake-3.31/Templates/CPack.GenericWelcome.txt")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux
 SuperTux is a classic 2D jump 'n run sidescroller game in a similar
 style like the original SuperMario games. This release of SuperTux
 features 9 enemies, 26 playable levels, software and OpenGL rendering
 modes, configurable joystick and keyboard input, new music and
 completely redone graphics.
 .
 This is a development snapshot of SuperTux. It may suffer from
 critical bugs and has not been fully tested. 
 .
 Homepage: http://supertux.lethargik.org/")
set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games/Action/Arcade")
set(CPACK_RPM_PACKAGE_LICENSE "GNU General Public License (GPL)")
set(CPACK_RPM_PACKAGE_NAME "supertux2")
set(CPACK_RPM_PACKAGE_SOURCES "ON")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_GENERATOR "TBZ2;TGZ;TXZ;TZ")
set(CPACK_SOURCE_IGNORE_FILES "/\\.git/;/home/runner/work/supertux/supertux/_codeql_build_dir;/\\..*")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "/home/runner/work/supertux/supertux;/")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/runner/work/supertux/supertux/_codeql_build_dir/CPackSourceConfig.cmake")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "SuperTux-copilot/improve-supertux-performance-4f7fb24-Source")
set(CPACK_SOURCE_RPM "OFF")
set(CPACK_SOURCE_TBZ2 "ON")
set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TOPLEVEL_TAG "Linux-Source")
set(CPACK_SOURCE_TXZ "ON")
set(CPACK_SOURCE_TZ "ON")
set(CPACK_SOURCE_ZIP "OFF")
set(CPACK_STRIP_FILES "")
set(CPACK_SYSTEM_NAME "Linux")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "Linux-Source")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "/home/runner/work/supertux/supertux/_codeql_build_dir/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
