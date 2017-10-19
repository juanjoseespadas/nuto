# Eigen header files (try to find the header file Eigen/Core)
#
# variables used by this module (can be also defined as environment variables):
#   EIGEN_ROOT - preferred installation prefix for searching for eigen package
#
# variables defined by this module
#   EIGEN_FOUND - defines whether eigen was found or not
#   EIGEN_INCLUDE_DIR - eigen include directory

# initialize variables
message(STATUS "Checking for Eigen3 headers ...")

# check if EIGEN_ROOT is set
if(NOT EIGEN_ROOT AND NOT $ENV{EIGEN_ROOT} STREQUAL "")
    set(EIGEN_ROOT $ENV{EIGEN_ROOT})
endif()

# convert path to unix style path and set search path
if(EIGEN_ROOT)
    file(TO_CMAKE_PATH ${EIGEN_ROOT} EIGEN_ROOT)
    set(_eigen_INCLUDE_SEARCH_DIRS ${EIGEN_ROOT}/include ${EIGEN_ROOT}
        ${_eigen_INCLUDE_SEARCH_DIRS})
endif()

# search for header dSFMT.h
find_path(EIGEN_INCLUDE_DIR
    NAMES eigen3/Eigen/Core
    HINTS ${_eigen_INCLUDE_SEARCH_DIRS})

# automatically parse the version number
file(READ "${EIGEN_INCLUDE_DIR}/eigen3/Eigen/src/Core/util/Macros.h"
    _eigen_version_header)
string(REGEX MATCH "define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)"
    _eigen_world_version_match "${_eigen_version_header}")
set(EIGEN_WORLD_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)"
    _eigen_major_version_match "${_eigen_version_header}")
set(EIGEN_MAJOR_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)"
    _eigen_minor_version_match "${_eigen_version_header}")
set(EIGEN_MINOR_VERSION "${CMAKE_MATCH_1}")
set(EIGEN_VERSION_NUMBER
    ${EIGEN_WORLD_VERSION}.${EIGEN_MAJOR_VERSION}.${EIGEN_MINOR_VERSION})

# handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EIGEN DEFAULT_MSG EIGEN_INCLUDE_DIR)
