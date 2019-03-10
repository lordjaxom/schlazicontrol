# FindAsio.cmake - Try to find the N.Lohmann Asio library
# Once done this will define
#
#  Asio_FOUND - System has Asio
#  Asio_INCLUDE_DIR - The Asio include directory
#  Asio_LIBRARIES - The libraries needed to use Asio
#  Asio_DEFINITIONS - Compiler switches required for using Asio

find_path(Asio_INCLUDE_DIRS
        NAMES asio.hpp
        CMAKE_FIND_ROOT_PATH_BOTH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Asio DEFAULT_MSG Asio_INCLUDE_DIRS)

set(Asio_DEFINITIONS ASIO_STANDALONE)

if(WIN32)
    set(Asio_LIBRARIES ws2_32)
endif()

mark_as_advanced(Asio_INCLUDE_DIRS)