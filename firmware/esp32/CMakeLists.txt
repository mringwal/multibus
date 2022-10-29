# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

set(PROJECT_VER "0.0.1")

set(MULTIBUS_ROOT       ${CMAKE_SOURCE_DIR}/../..)
set(MULTIBUS_SRC        ${MULTIBUS_ROOT}/host/c)
set(MULTIBUS_PROTOCOL   ${MULTIBUS_ROOT}/protocol)
set(MULTIBUS_PROTOCOL_C ${MULTIBUS_ROOT}/protocol/c)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(multibus-esp32)

