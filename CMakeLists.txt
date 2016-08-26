cmake_minimum_required(VERSION 3.5)
project(schlazicontrol)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(RASPBERRY_HOME ${CMAKE_SOURCE_DIR}/../..)
set(TOOLCHAIN_HOME ${RASPBERRY_HOME}/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian)

set(SSH_HOST speedstar)
set(SSH_USER lordjaxom)
set(SSH_SCRIPT
    export LANG=de_DE@euro $<SEMICOLON>
    source \\$$HOME/etc/raspirc $<SEMICOLON>
    cd src/schlazicontrol $<SEMICOLON>
    make -j5)

set(
    SOURCE_FILES
        component.cpp
        component.hpp
        connection.cpp
        connection.hpp
        events.hpp
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
        cmdline.cpp
        cmdline.hpp
        transition.hpp
        transition.cpp
        transition_triggers.cpp
        transition_triggers.hpp
        expression.cpp
        expression.hpp
        timer.cpp
        timer.hpp
        triggers.cpp
        triggers.hpp)

include_directories(
        SYSTEM
        ${TOOLCHAIN_HOME}/arm-linux-gnueabihf/libc/usr/include
        ${TOOLCHAIN_HOME}/arm-linux-gnueabihf/include/c++/4.8.3)

include_directories(
        ${RASPBERRY_HOME}/src/wiringPi/wiringPi
        ${RASPBERRY_HOME}/src/rpi_ws281x
        ${RASPBERRY_HOME}/src/asio/include
        ${RASPBERRY_HOME}/src/boost
        ${RASPBERRY_HOME}/src/jsoncpp/include)

add_compile_options(-std=c++11)

add_custom_target(
    schlazicontrol
    COMMAND bash -c 'ssh ${SSH_USER}@${SSH_HOST} \"${SSH_SCRIPT}\"'
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    SOURCES ${SOURCE_FILES})

add_executable(dummy ${SOURCE_FILES})