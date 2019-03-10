# FindJson.cmake - Try to find the N.Lohmann Json library
# Once done this will define
#
#  Json_FOUND - System has Json
#  Json_INCLUDE_DIR - The Json include directory
#  Json_LIBRARIES - The libraries needed to use Json
#  Json_DEFINITIONS - Compiler switches required for using Json

find_path(Json_INCLUDE_DIRS
        NAMES nlohmann/json.hpp
        CMAKE_FIND_ROOT_PATH_BOTH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Json DEFAULT_MSG Json_INCLUDE_DIRS)

mark_as_advanced(Json_INCLUDE_DIRS)