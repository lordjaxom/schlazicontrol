#
# for the setup to work, TOOLCHAIN_HOME must be set to the base path of gcc-4.9.2-glibc-2.19
#

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

# always use mingw makefiles
SET(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "" FORCE)

SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(MAKE_ROOT "${TOOLCHAIN_HOME}/mingw-make")
SET(TOOLCHAIN_ROOT "${TOOLCHAIN_HOME}/arm-linux-gnueabihf")

# set the search path for toolchain
#SET(ENV{PATH} "${MAKE_ROOT};${TOOLCHAIN_ROOT}/bin")

# specify the make tool
SET(CMAKE_MAKE_PROGRAM "${MAKE_ROOT}/make.exe" CACHE FILEPATH "")

# specify the cross compiler
SET(CMAKE_C_COMPILER "${TOOLCHAIN_ROOT}/bin/arm-linux-gnueabihf-gcc.exe")
SET(CMAKE_CXX_STANDARD_LIBRARIES "")
SET(CMAKE_CXX_COMPILER "${TOOLCHAIN_ROOT}/bin/arm-linux-gnueabihf-g++.exe")
SET(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
SET(CMAKE_AR "${TOOLCHAIN_ROOT}/bin/arm-linux-gnueabihf-gcc-ar.exe" CACHE FILEPATH "")
SET(CMAKE_RANLIB "${TOOLCHAIN_ROOT}/bin/arm-linux-gnueabihf-gcc-ranlib.exe" CACHE FILEPATH "")

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_ROOT}")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# system include paths for CLion
INCLUDE_DIRECTORIES(SYSTEM
        "${TOOLCHAIN_ROOT}/arm-linux-gnueabihf/include"
        "${TOOLCHAIN_ROOT}/arm-linux-gnueabihf/include/c++/4.9.2"
        "${TOOLCHAIN_ROOT}/arm-linux-gnueabihf/include/c++/4.9.2/arm-linux-gnueabihf"
        "${TOOLCHAIN_ROOT}/lib/gcc/arm-linux-gnueabihf/4.9.2/include"
        "${THIRDPARTY_HOME}/boost_1_61_0"
        "${THIRDPARTY_HOME}/asio-1.10.6/include"
        "${THIRDPARTY_HOME}/jsoncpp-master/include"
        "${THIRDPARTY_HOME}/wiringPi/wiringPi"
        "${THIRDPARTY_HOME}/rpi_ws281x")