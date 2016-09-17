#
# for the project to work, THIRDPARTY_HOME must be set to the path where dependencies are located
#

cmake_minimum_required(VERSION 3.5)
project(schlazicontrol)

set(TOOLCHAIN_INCLUDED ON CACHE BOOL "" FORCE)
set(TOOLCHAIN_HOME "C:/Users/lordjaxom/Programme/gcc-4.9.2-glibc-2.19")
set(THIRDPARTY_HOME "C:/Users/lordjaxom/Projekte/_RASPI_THIRDPARTY")

include(Toolchain.cmake)

set(BOOST_ROOT "${THIRDPARTY_HOME}/boost_1_61_0")
include_directories("${BOOST_ROOT}")

set(ASIO_ROOT "${THIRDPARTY_HOME}/asio-1.10.6")
set(asio_DEFINITIONS ASIO_STANDALONE)
include_directories("${ASIO_ROOT}/include")

set(JSONCPP_ROOT "${THIRDPARTY_HOME}/jsoncpp-master")
set(jsoncpp_LIBRARIES jsoncpp)
include_directories("${JSONCPP_ROOT}/include")
link_directories("${JSONCPP_ROOT}/src/lib_json")

set(WIRINGPI_ROOT "${THIRDPARTY_HOME}/wiringPi")
set(wiringPi_LIBRARIES wiringPi)
include_directories("${WIRINGPI_ROOT}/wiringPi")
link_directories("${WIRINGPI_ROOT}/wiringPi")

set(RPIWS281X_ROOT "${THIRDPARTY_HOME}/rpi_ws281x")
set(rpiWS281X_LIBRARIES rpi_ws281x)
include_directories("${RPIWS281X_ROOT}")
link_directories("${RPIWS281X_ROOT}")

set(TYPESTRING_ROOT "${THIRDPARTY_HOME}/typestring-master")
include_directories("${TYPESTRING_ROOT}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -flto")

set(SOURCE_FILES
        component.cpp
        component.hpp
        connection.cpp
        connection.hpp
        event.hpp
        event.cpp
        gpio.cpp
        gpio.hpp
        input_gpio.cpp
        input_gpio.hpp
        input.cpp
        input.hpp
        input_vdcd.cpp
        input_vdcd.hpp
        logging.cpp
        logging.hpp
        manager.cpp
        manager.hpp
        output.cpp
        output.hpp
        output_pwm.cpp
        output_pwm.hpp
        output_ws281x.cpp
        output_ws281x.hpp
        properties.cpp
        properties.hpp
        schlazicontrol.cpp
        types.cpp
        types.hpp
        utility.hpp
        vdcd.cpp
        vdcd.hpp
        ws281x.cpp
        ws281x.hpp
        commandline.cpp
        commandline.hpp
        transition.hpp
        transition.cpp
        transition_triggers.cpp
        transition_triggers.hpp
        expression.cpp
        expression.hpp
        timer.cpp
        timer.hpp
        triggers.cpp
        triggers.hpp
        transition_multiply.cpp
        transition_multiply.hpp
        transition_fade.cpp
        transition_fade.hpp
        typeinfo.hpp
        typetraits.hpp
        scoped.hpp
        scoped.cpp
        statistics.hpp)

add_executable(schlazicontrol ${SOURCE_FILES})
target_compile_definitions(schlazicontrol PRIVATE ${asio_DEFINITIONS})
target_link_libraries(schlazicontrol pthread ${jsoncpp_LIBRARIES} ${wiringPi_LIBRARIES} ${rpiWS281X_LIBRARIES})

add_custom_command(TARGET schlazicontrol
        POST_BUILD
        COMMAND bash -c 'scp `cygpath -u $<TARGET_FILE:schlazicontrol>` osmc@schlazicontrol:')