# FindWiringpi.cmake - Try to find the Mosquitto library
# Once done this will define
#
#  Mosquitto_FOUND - System has Mosquitto
#  Mosquitto_INCLUDE_DIR - The Mosquitto include directory
#  Mosquitto_LIBRARIES - The libraries needed to use Mosquitto
#  Mosquitto_DEFINITIONS - Compiler switches required for using Mosquitto

find_library(Mosquitto_LIBRARIES
        NAMES mosquitto
        CMAKE_FIND_ROOT_PATH_BOTH)

find_path(Mosquitto_INCLUDE_DIRS
        NAMES mosquitto.h
        CMAKE_FIND_ROOT_PATH_BOTH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mosquitto DEFAULT_MSG Mosquitto_LIBRARIES Mosquitto_INCLUDE_DIRS)

if(NOT WIN32)
    set(Mosquitto_LIBRARIES ${Mosquitto_LIBRARIES} pthread)
endif()

mark_as_advanced(Mosquitto_LIBRARIES Mosquitto_INCLUDE_DIRS)