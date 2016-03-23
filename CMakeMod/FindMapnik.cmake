# - Try to find Mapnik
# Once done this will define
#  LIBMAPNIK_FOUND - System has Mapnik
#  LIBMAPNIK_INCLUDE_DIRS - The Mapnik include directories
#  LIBMAPNIK_LIBRARIES - The libraries needed to use Mapnik
#  LIBMAPNIK_DEFINITIONS - Compiler switches required for using Mapnik
#  MAPNIK_VERSION
#  MAPNIK_VERSION_MAJOR
#  MAPNIK_VERSION_MINOR
#  MAPNIK_VERSION_PATCH

find_package(PkgConfig)
pkg_check_modules(PC_LIBMAPNIK QUIET libmapnik)
set(LIBMAPNIK_DEFINITIONS ${PC_LIBMAPNIK_CFLAGS_OTHER})

find_path(LIBMAPNIK_INCLUDE_DIR mapnik/config.hpp
          HINTS ${PC_LIBMAPNIK_INCLUDEDIR} ${PC_LIBMAPNIK_INCLUDE_DIRS}
          PATH_SUFFIXES libmapnik )

find_library(LIBMAPNIK_LIBRARY NAMES mapnik libmapnik
             HINTS ${PC_LIBMAPNIK_LIBDIR} ${PC_LIBMAPNIK_LIBRARY_DIRS} )

set(LIBMAPNIK_LIBRARIES ${LIBMAPNIK_LIBRARY} )
set(LIBMAPNIK_INCLUDE_DIRS ${LIBMAPNIK_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBMAPNIK_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Mapnik  DEFAULT_MSG
                                  LIBMAPNIK_LIBRARY LIBMAPNIK_INCLUDE_DIR)

mark_as_advanced(LIBMAPNIK_INCLUDE_DIR LIBMAPNIK_LIBRARY )

# Detect libgphoto2 version
FIND_PROGRAM(MAPNIK_CONFIG_EXECUTABLE NAMES mapnik-config)
IF(MAPNIK_CONFIG_EXECUTABLE)
  EXEC_PROGRAM(${MAPNIK_CONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE MAPNIK_VERSION)
  string(REGEX REPLACE "([0-9]+).*$" "\\1" MAPNIK_VERSION_MAJOR "${MAPNIK_VERSION}")
  string(REGEX REPLACE "[0-9]+\\.([0-9]+).*$" "\\1" MAPNIK_VERSION_MINOR  "${MAPNIK_VERSION}")
  string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" MAPNIK_VERSION_PATCH "${MAPNIK_VERSION}")

  set(MAPNIK_VERSION_STRING "${MAPNIK_VERSION_MAJOR}.${MAPNIK_VERSION_MINOR}.${MAPNIK_VERSION_PATCH}")
ENDIF(MAPNIK_CONFIG_EXECUTABLE)
