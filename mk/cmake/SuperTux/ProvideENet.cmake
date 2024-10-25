# - Try to find enet
# Once done this will define
#
#  ENET_FOUND - system has enet
#  ENet_INCLUDE_DIRS - the enet include directory
#  ENet_LIBRARIES - the libraries needed to use enet
#
# $ENETDIR is an environment variable used for finding enet.
#
#  Borrowed from The Mana World
#  https://themanaworld.org/
#
# Several changes and additions by Fabian 'x3n' Landau
# Lots of simplifications by Adrian Friedli
#                 > www.orxonox.net <

FIND_PATH(ENet_INCLUDE_DIRS enet/enet.h
    PATHS
    $ENV{ENETDIR}
    /usr/local
    /usr
    PATH_SUFFIXES include
    )

FIND_LIBRARY(ENet_LIBRARY
    NAMES enet
    PATHS
    $ENV{ENETDIR}
    /usr/local
    /usr
    PATH_SUFFIXES lib
    )

# handle the QUIETLY and REQUIRED arguments and set ENET_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ENet DEFAULT_MSG ENet_LIBRARY ENet_INCLUDE_DIRS)

IF (ENET_FOUND)
    IF(WIN32)
        SET(WINDOWS_ENET_DEPENDENCIES "ws2_32;winmm")
        SET(ENet_LIBRARIES ${ENet_LIBRARY} ${WINDOWS_ENET_DEPENDENCIES})
    ELSE(WIN32)
        SET(ENet_LIBRARIES ${ENet_LIBRARY})
    ENDIF(WIN32)
ENDIF (ENET_FOUND)

MARK_AS_ADVANCED(ENet_LIBRARY ENet_LIBRARIES ENet_INCLUDE_DIRS)

if (ENET_FOUND)
    add_library(LibENet INTERFACE IMPORTED)
    set_target_properties(LibENet PROPERTIES
            INTERFACE_LINK_LIBRARIES "${ENet_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${ENet_INCLUDE_DIRS}"
            )
endif ()
