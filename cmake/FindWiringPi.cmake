# FindWiringPi.cmake - Try to find the WiringPi library
# Once done this will define
#
#  WiringPi_FOUND - System has WiringPi
#  WiringPi_INCLUDE_DIR - The WiringPi include directory
#  WiringPi_LIBRARIES - The libraries needed to use WiringPi
#  WiringPi_DEFINITIONS - Compiler switches required for using WiringPi

find_library(WiringPi_LIBRARIES
        NAMES wiringPi
        CMAKE_FIND_ROOT_PATH_BOTH)

find_path(WiringPi_INCLUDE_DIRS
        NAMES wiringPi.h
        CMAKE_FIND_ROOT_PATH_BOTH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WiringPi DEFAULT_MSG WiringPi_LIBRARIES WiringPi_INCLUDE_DIRS)

mark_as_advanced(WiringPi_LIBRARIES WiringPi_INCLUDE_DIRS)