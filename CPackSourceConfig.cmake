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


SET(CPACK_BINARY_BUNDLE "")
SET(CPACK_BINARY_CYGWIN "")
SET(CPACK_BINARY_DEB "OFF")
SET(CPACK_BINARY_DRAGNDROP "")
SET(CPACK_BINARY_NSIS "OFF")
SET(CPACK_BINARY_OSXX11 "")
SET(CPACK_BINARY_PACKAGEMAKER "")
SET(CPACK_BINARY_RPM "OFF")
SET(CPACK_BINARY_STGZ "ON")
SET(CPACK_BINARY_TBZ2 "OFF")
SET(CPACK_BINARY_TGZ "ON")
SET(CPACK_BINARY_TZ "ON")
SET(CPACK_BINARY_WIX "")
SET(CPACK_BINARY_ZIP "")
SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
SET(CPACK_COMPONENTS_ALL "Unspecified;squirrel")
SET(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
SET(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.5), libgcc1 (>= 1:4.1), libgl1-mesa-glx | libgl1, libogg0 (>= 1.1.3), libopenal0a, libphysfs-1.0-0, libsdl-image1.2 (>= 1.2.5), libsdl1.2debian (>= 1.2.10-1), libstdc++6 (>= 4.1.2), libvorbis0a (>= 1.1.2), libvorbisfile3 (>= 1.1.2), libcurl3 (>= 7.16)")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux
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
SET(CPACK_DEBIAN_PACKAGE_NAME "supertux2")
SET(CPACK_DEBIAN_PACKAGE_SECTION "games")
SET(CPACK_GENERATOR "TGZ;TBZ2;TZ")
SET(CPACK_IGNORE_FILES "/CVS/;/\\.svn/;/\\.bzr/;/\\.hg/;/\\.git/;\\.swp$;\\.#;/#")
SET(CPACK_INSTALLED_DIRECTORIES "/home/ok1asw/st-vyvoj/supertux;/")
SET(CPACK_INSTALL_CMAKE_PROJECTS "")
SET(CPACK_INSTALL_PREFIX "/usr/local")
SET(CPACK_MODULE_PATH "/home/ok1asw/st-vyvoj/supertux/mk/cmake")
SET(CPACK_NSIS_DISPLAY_NAME "SuperTux 0.3.4-GIT")
SET(CPACK_NSIS_INSTALLER_ICON_CODE "")
SET(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
SET(CPACK_NSIS_PACKAGE_NAME "SuperTux 0.3.4-GIT")
SET(CPACK_OUTPUT_CONFIG_FILE "/home/ok1asw/st-vyvoj/supertux/CPackConfig.cmake")
SET(CPACK_PACKAGE_CONTACT "SuperTux Devel Team <supertux-devel@lists.lethargik.org>")
SET(CPACK_PACKAGE_DEFAULT_LOCATION "/")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "/home/ok1asw/st-vyvoj/supertux/README.md")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Jump'n'Run Game featuring Tux")
SET(CPACK_PACKAGE_EXECUTABLES "..\\supertux2;SuperTux 0.3.4-GIT")
SET(CPACK_PACKAGE_FILE_NAME "SUPERTUX-0.3.4-GIT-Source")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "SuperTux 0.3.4-GIT")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "SuperTux 0.3.4-GIT")
SET(CPACK_PACKAGE_NAME "SUPERTUX")
SET(CPACK_PACKAGE_RELOCATABLE "true")
SET(CPACK_PACKAGE_VENDOR "SuperTux Devel Team")
SET(CPACK_PACKAGE_VERSION "0.3.4-GIT")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "3")
SET(CPACK_PACKAGE_VERSION_PATCH "4-GIT")
SET(CPACK_RESOURCE_FILE_LICENSE "/home/ok1asw/st-vyvoj/supertux/LICENSE")
SET(CPACK_RESOURCE_FILE_README "/usr/share/cmake-2.8/Templates/CPack.GenericDescription.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "/usr/share/cmake-2.8/Templates/CPack.GenericWelcome.txt")
SET(CPACK_RPM_PACKAGE_DESCRIPTION "Classic 2D jump 'n run sidescroller with Tux
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
SET(CPACK_RPM_PACKAGE_GROUP "Amusements/Games/Action/Arcade")
SET(CPACK_RPM_PACKAGE_LICENSE "GNU General Public License (GPL)")
SET(CPACK_RPM_PACKAGE_NAME "supertux2")
SET(CPACK_SET_DESTDIR "OFF")
SET(CPACK_SOURCE_CYGWIN "")
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;TZ")
SET(CPACK_SOURCE_IGNORE_FILES "/CVS/;/\\.svn/;/\\.bzr/;/\\.hg/;/\\.git/;\\.swp$;\\.#;/#")
SET(CPACK_SOURCE_INSTALLED_DIRECTORIES "/home/ok1asw/st-vyvoj/supertux;/")
SET(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/ok1asw/st-vyvoj/supertux/CPackSourceConfig.cmake")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "SUPERTUX-0.3.4-GIT-Source")
SET(CPACK_SOURCE_TBZ2 "ON")
SET(CPACK_SOURCE_TGZ "ON")
SET(CPACK_SOURCE_TOPLEVEL_TAG "Linux-Source")
SET(CPACK_SOURCE_TZ "ON")
SET(CPACK_SOURCE_ZIP "OFF")
SET(CPACK_STRIP_FILES "")
SET(CPACK_SYSTEM_NAME "Linux")
SET(CPACK_TOPLEVEL_TAG "Linux-Source")
SET(CPACK_WIX_SIZEOF_VOID_P "8")
