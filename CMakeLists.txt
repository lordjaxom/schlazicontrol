#
# for the project to work, THIRDPARTY_HOME must be set to the path where dependencies are located
#

cmake_minimum_required(VERSION 3.12)
project(schlazicontrol)

set(TOOLCHAIN_INCLUDED ON CACHE BOOL "" FORCE)
set(TOOLCHAIN_HOME "C:/Users/lordjaxom/Programme/gcc-4.9.2-glibc-2.19")
set(THIRDPARTY_HOME "C:/Users/lordjaxom/Projekte/_RASPI_THIRDPARTY")

include(Toolchain.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(BOOST_ROOT "${THIRDPARTY_HOME}/boost_1_68_0")
set(Boost_INCLUDE_DIRS "${BOOST_ROOT}")

set(ASIO_ROOT "${THIRDPARTY_HOME}/asio-1.12.1")
set(asio_DEFINITIONS ASIO_STANDALONE)
set(asio_INCLUDE_DIRS "${ASIO_ROOT}/include")

set(Json_ROOT "${THIRDPARTY_HOME}/json-master/include")
# for Raspberry Pi:
# set(WiringPi_ROOT "${THIRDPARTY_HOME}/wiringPi")
# for Banana Pi:
set(WiringPi_ROOT "${THIRDPARTY_HOME}/BPI-WiringPi2/wiringPi")
set(Mosquitto_ROOT "${THIRDPARTY_HOME}/mosquitto-1.5.4/lib")
set(RpiWs281x_ROOT "${THIRDPARTY_HOME}/rpi_ws281x")

find_package(Json REQUIRED)

set(TYPESTRING_ROOT "${THIRDPARTY_HOME}/typestring-master")
include_directories("${TYPESTRING_ROOT}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

set(SOURCE_FILES
        core/config.hpp
        core/commandline.cpp
        core/commandline.hpp
        core/component.cpp
        core/component.hpp
        core/logging.cpp
        core/logging.hpp
        core/input.cpp
        core/input.hpp
        core/output.cpp
        core/output.hpp
        utility/string_view.hpp
        utility/optional.hpp)

find_package(WiringPi)
if(WiringPi_FOUND)
    set(SOURCE_FILES ${SOURCE_FILES}
            modules/wiringPi/gpio.cpp
            modules/wiringPi/gpio.hpp
            modules/wiringPi/input_gpio.cpp
            modules/wiringPi/input_gpio.hpp
            modules/wiringPi/output_pwm.cpp
            modules/wiringPi/output_pwm.hpp)
    set(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${WiringPi_INCLUDE_DIRS})
    set(MODULE_LIBRARIES ${MODULE_LIBRARIES} ${WiringPi_LIBRARIES})
    message("Module wiringPi: true")
else()
    message("Module wiringPi: false")
endif()

find_package(Mosquitto)
if(Mosquitto_FOUND)
    set(SOURCE_FILES ${SOURCE_FILES}
            modules/mqtt/mqtt.cpp
            modules/mqtt/mqtt.hpp
            modules/mqtt/input_subscribe.cpp
            modules/mqtt/input_subscribe.hpp
            modules/mqtt/output_publish.cpp
            modules/mqtt/output_publish.hpp)
    set(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${Mosquitto_INCLUDE_DIRS})
    set(MODULE_LIBRARIES ${MODULE_LIBRARIES} ${Mosquitto_LIBRARIES})
    message("Module mqtt: true")
else()
    message("Module mqtt: false")
endif()

find_package(RpiWs281x)
if(RpiWs281x_FOUND)
    set(SOURCE_FILES ${SOURCE_FILES}
            modules/rpi_ws281x/ws281x.cpp
            modules/rpi_ws281x/ws281x.hpp
            modules/rpi_ws281x/output_ws281x.cpp
            modules/rpi_ws281x/output_ws281x.hpp)
    set(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${RpiWs281x_INCLUDE_DIRS})
    set(MODULE_LIBRARIES ${MODULE_LIBRARIES} ${RpiWs281x_LIBRARIES})
    message("Module rpi_ws281x: true")
else()
    message("Module rpi_ws281x: false")
endif()

if(NOT WIN32)
    set(SOURCE_FILES ${SOURCE_FILES}
            modules/console/input_console.cpp
            modules/console/input_console.hpp
            modules/console/output_console.cpp
            modules/console/output_console.hpp
            modules/console/console.cpp
            modules/console/console.hpp)
    message("Module console: true")
else()
    message("Module console: false")
endif()

set(SOURCE_FILES ${SOURCE_FILES}
        connection.cpp
        connection.hpp
        event.hpp
        event.cpp
        manager.cpp
        manager.hpp
        properties.cpp
        properties.hpp
        schlazicontrol.cpp
        types.cpp
        types.hpp
        forward.hpp
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
        scoped.hpp
        scoped.cpp
        statistics.hpp
        transition_shift.cpp
        transition_shift.hpp
        multiconnection.cpp
        multiconnection.hpp
        trackable.cpp
        trackable.hpp
        utility_stream.hpp
        utility_string.hpp
        utility_algorithm.hpp
        utility_math.hpp
        utility_valuetable.hpp
        utility_gamma.hpp
        utility_colorwheel.hpp
        transition_animate.cpp
        transition_animate.hpp
        transition_animate_waves.cpp
        utility_graphics.cpp
        utility_graphics.hpp
        utility_ranged.hpp
        transition_color.cpp
        transition_color.hpp
        transition_color_fill.cpp
        transition_color_gradient.cpp)

add_executable(schlazicontrol ${SOURCE_FILES})
target_compile_definitions(schlazicontrol PRIVATE
        ${asio_DEFINITIONS})
target_include_directories(schlazicontrol PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${Boost_INCLUDE_DIRS}
        ${asio_INCLUDE_DIRS}
        ${Json_INCLUDE_DIRS}
        ${typestring_INCLUDE_DIRS}
        ${MODULE_INCLUDE_DIRS})
target_link_libraries(schlazicontrol
        ${MODULE_LIBRARIES})
if(WIN32)
    target_link_libraries(schlazicontrol ws2_32)
else()
    target_link_libraries(schlazicontrol pthread)
endif()
