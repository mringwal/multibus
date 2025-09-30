idf_component_register(SRCS
        "main.cpp"
        "CUartSerial.cpp"
        "SMultiBusMessage.cpp"
        "CSerialMultiBusMessageReaderWriter.cpp"
        "CMultiBusOperationExecutor.cpp"
        "CMultiBusOperation.cpp"
        "CBridge.cpp"
        "CI2CMaster.cpp"
        "CSPIMaster.cpp"
        "CComponentFactory.cpp"
        "CHardwareInfo.cpp"
        ${CMAKE_BINARY_DIR}/multibus_protocol.c
        INCLUDE_DIRS "." ${CMAKE_BINARY_DIR})

# rule to generate multibus_protocol helper
add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/multibus_protocol.h ${CMAKE_BINARY_DIR}/multibus_protocol.c ${CMAKE_BINARY_DIR}/multibus_transport_protocol.h
        DEPENDS ${MULTIBUS_ROOT}/protocol/multibus.yml ${MULTIBUS_ROOT}/protocol/generator-c.py ${MULTIBUS_ROOT}/protocol/parser.py
        COMMAND ${Python_EXECUTABLE}
        ARGS ${MULTIBUS_ROOT}/protocol/generator-c.py ${CMAKE_BINARY_DIR}
)
