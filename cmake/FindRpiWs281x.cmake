# FindRpiWs281x.cmake - Try to find the RpiWs281x library
# Once done this will define
#
#  RpiWs281x_FOUND - System has RpiWs281x
#  RpiWs281x_INCLUDE_DIR - The RpiWs281x include directory
#  RpiWs281x_LIBRARIES - The libraries needed to use RpiWs281x
#  RpiWs281x_DEFINITIONS - Compiler switches required for using RpiWs281x

find_library(RpiWs281x_LIBRARIES
        NAMES rpi_ws281x
        CMAKE_FIND_ROOT_PATH_BOTH)

find_path(RpiWs281x_INCLUDE_DIRS
        NAMES ws2811.h
        CMAKE_FIND_ROOT_PATH_BOTH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RpiWs281x DEFAULT_MSG RpiWs281x_LIBRARIES RpiWs281x_INCLUDE_DIRS)

mark_as_advanced(RpiWs281x_LIBRARIES RpiWs281x_INCLUDE_DIRS)