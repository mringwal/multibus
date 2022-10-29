#
# CMake Helper to generate MultiBus Protocol Helper files MULTIBUS_PROTOCOL_SRC
#

# find python interpreter
find_package (Python REQUIRED COMPONENTS Interpreter)

# set MULTIBUS_PROTOCOL_SRC files
set (MULTIBUS_PROTOCOL_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/multibus_protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/multibus_protocol.c
    ${CMAKE_CURRENT_BINARY_DIR}/multibus_transport_protocol.h
)

# custom command to generate them in CMAKE_CURRENT_BINARY_DIR
add_custom_command(
        OUTPUT  ${MULTIBUS_PROTOCOL_SRC}
        DEPENDS ${MULTIBUS_ROOT}/protocol/multibus.yml ${MULTIBUS_ROOT}/protocol/generator-c.py ${MULTIBUS_ROOT}/protocol/parser.py
        COMMAND ${Python_EXECUTABLE}
        ARGS    ${MULTIBUS_ROOT}/protocol/generator-c.py ${CMAKE_CURRENT_BINARY_DIR}
)
