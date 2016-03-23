#
# Copyright (c) 2016 Lukas Karas <lukas.karas@centrum.cz>
#
# Structure of MoNav cmake scripts was inspired by DigiKam project
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(monav)

# ==================================================================================================
# Information to update before to release this package.

# MoNav version
set(MONAV_MAJOR_VERSION "0")
set(MONAV_MINOR_VERSION "4")
set(MONAV_PATCH_VERSION "0")

set(MONAV_SUFFIX_VERSION "-alpha1")
set(MONAV_VERSION_SHORT  "${MONAV_MAJOR_VERSION}.${MONAV_MINOR_VERSION}.${MONAV_PATCH_VERSION}")
set(MONAV_VERSION_STRING "${MONAV_VERSION_SHORT}${MONAV_SUFFIX_VERSION}")

# ==================================================================================================

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MIN_VERSION    "2.8.12")
set(QT_MIN_VERSION       "5.2.0")

cmake_minimum_required(VERSION ${CMAKE_MIN_VERSION})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_HOME_DIRECTORY}/CMakeMod")

# Cmake macros
include(FeatureSummary)

# Local macros
include(MacroUtils)

# ==================================================================================================
# Find dependencies

option(ENABLE_MONAV_DAEMON_TEST  "Build daemon test (default=OFF)"  OFF)

find_package( Qt5 ${QT_MIN_VERSION} 
              REQUIRED NO_MODULE COMPONENTS 
              Core
              Network
              Widgets
)

find_package( Qt5 ${QT_MIN_VERSION}
              OPTIONAL_COMPONENTS
              Gui
              Positioning
              Xml
              Svg
)

find_package(Protobuf REQUIRED) # /usr/share/cmake-3.2/Modules/FindProtobuf.cmake
find_package(OpenMP REQUIRED)

# ==================================================================================================
# Dependencies Rules

# TODO: modularize...

if( Qt5_FOUND       AND
    PROTOBUF_FOUND  AND
    OPENMP_FOUND
    )
    set(MONAV_ROUTINGDAEMON_CAN_BE_COMPILED true)

    if (Qt5Xml_FOUND AND 
        Qt5Svg_FOUND
        )
        set(MONAV_CLIENT_CAN_BE_COMPILED true)
    else()
        set(MONAV_CLIENT_CAN_BE_COMPILED false)
    endif()

    set(MONAV_CAN_BE_COMPILED true)
else()
    set(MONAV_ROUTINGDAEMON_CAN_BE_COMPILED false)
    set(MONAV_CLIENT_CAN_BE_COMPILED false)
    set(MONAV_CAN_BE_COMPILED false)
endif()


# ==================================================================================================
# Print build configuration

## print all variables
#message(STATUS "contractionhierarchies: "contractionhierarchies)
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

message(STATUS "")
message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "MoNav ${MONAV_VERSION_STRING} build configuration results")

message(STATUS "")
message(STATUS "Requiered dependencies:")

PRINT_LIBRARY_STATUS("Qt5"                    "http://www.qtsoftware.com"                                        "(version >= ${QT_MIN_VERSION})"     Qt5_FOUND)
PRINT_LIBRARY_STATUS("Qt5::Positioning"       "http://www.qtsoftware.com"                                        ""     Qt5Positioning_FOUND)
PRINT_LIBRARY_STATUS("Qt5::Xml"               "http://www.qtsoftware.com"                                        ""     Qt5Xml_FOUND)
PRINT_LIBRARY_STATUS("Qt5::Svg"               "http://www.qtsoftware.com"                                        ""     Qt5Svg_FOUND)
PRINT_LIBRARY_STATUS("libprotobuf"            "https://github.com/google/protobuf"                               ""     PROTOBUF_FOUND)


# PRINT_OPTIONAL_LIBRARY_STATUS("libmarble"         "https://projects.kde.org/projects/kde/kdeedu/marble"             "(version >= 0.22.0)"                  "digiKam will be compiled without geolocation maps support."                  MARBLE_FOUND)

message(STATUS "")
message(STATUS "Monav components:")

if (MONAV_ROUTINGDAEMON_CAN_BE_COMPILED)
    message(STATUS      " MoNav routing daemon..................... YES")
else()
    message(FATAL_ERROR " MoNav routing daemon..................... NO  (Look README file for more details about dependencies)")
endif()

if (MONAV_CLIENT_CAN_BE_COMPILED)
    message(STATUS      " MoNav client ............................ YES")
else()
    message(FATAL_ERROR " MoNav client ............................ NO  (Look README file for more details about dependencies)")
endif()

message(STATUS "")

if(Qt5_FOUND      AND
  PROTOBUF_FOUND
  )
    message(STATUS      " MoNav can be compiled.................... YES")
    set(MONAV_CAN_BE_COMPILED true)
else()
    message(FATAL_ERROR " MoNav will be compiled................... NO  (Look README file for more details about dependencies)")
    set(MONAV_CAN_BE_COMPILED false)
endif()

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "")

if(MONAV_CAN_BE_COMPILED)

  add_definitions( -Wall -pedantic -fPIC -Wno-unused-function)

  # QT5 definitions
  add_definitions(${QT_DEFINITIONS})
  add_definitions(${Qt5Core_DEFINITIONS})
  
  # OpenMP definitions
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

  # lzma is pure C code
  add_definitions( -std=c11 )
  add_subdirectory(utils/lzma)

  # following code is c++
  remove_definitions(-std=c11)
  add_definitions(-std=c++11)

  add_subdirectory(plugins/contractionhierarchies)
  add_subdirectory(plugins/gpsgrid)
  add_subdirectory(utils/directory)
  add_subdirectory(utils/qtservice-2.6_1-opensource)

  if (MONAV_ROUTINGDAEMON_CAN_BE_COMPILED)
    add_subdirectory(routingdaemon)
  endif()

  if (MONAV_CLIENT_CAN_BE_COMPILED)
    add_subdirectory(client)
  endif()

endif()
