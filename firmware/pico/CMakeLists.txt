
cmake_minimum_required(VERSION 3.13)

set(PROJECT multibus-pico)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(${PROJECT} C CXX)

pico_sdk_init()
add_executable(${PROJECT})

set(MULTIBUS_ROOT ${CMAKE_SOURCE_DIR}/../..)
set(MULTIBUS_SRC ${MULTIBUS_ROOT}/host/c)
set(MULTIBUS_PROTOCOL ${MULTIBUS_ROOT}/protocol)
set(MULTIBUS_PROTOCOL_C ${MULTIBUS_ROOT}/protocol/c)

# find python
find_package (Python REQUIRED COMPONENTS Interpreter)

# rule to generate multibus_protocol helper
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/multibus_protocol.h ${CMAKE_CURRENT_BINARY_DIR}/multibus_protocol.c ${CMAKE_CURRENT_BINARY_DIR}/multibus_transport_protocol.h
        DEPENDS ${MULTIBUS_ROOT}/protocol/multibus.yml ${MULTIBUS_ROOT}/protocol/generator-c.py ${MULTIBUS_ROOT}/protocol/parser.py
        COMMAND ${Python_EXECUTABLE}
        ARGS ${MULTIBUS_ROOT}/protocol/generator-c.py ${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(multibus-pico PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        ${CMAKE_CURRENT_BINARY_DIR}/multibus_protocol.c
        ${CMAKE_CURRENT_BINARY_DIR}/multibus_protocol.h
)

target_include_directories(${PROJECT} PRIVATE ${MULTIBUS_SRC} ${MULTIBUS_PROTOCOL_C} ${CMAKE_CURRENT_BINARY_DIR} .)

target_link_libraries(${PROJECT} PUBLIC pico_stdlib tinyusb_device tinyusb_board pico_unique_id hardware_i2c hardware_spi)

pico_add_extra_outputs(${PROJECT})

pico_enable_stdio_usb(${PROJECT} 0)
pico_enable_stdio_uart(${PROJECT} 1)
