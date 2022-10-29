#!/usr/bin/env python3

# Copyright 2022 Boris Zweimueller
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
# disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
# following disclaimer in the documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

from time import sleep
import sys

 
sys.path.insert(1, '../../../multibus/host/python')
import serial_connection
import multibus_bridge

# the python bindings are generated by importing the multibus_bridge module
from generated import multibus_protocol

BAUDRATE = 115200


def main(serial_port):
    multibus_connection = serial_connection.MultibusSerialConnection(serial_port, BAUDRATE)
    bridge = multibus_bridge.MultibusBridge(multibus_connection)
    print("Protocol version: %u" % bridge.get_protocol_version())
    print("Firmware version: %u" % bridge.get_firmware_version())
    print("Hardware info:    %s" % bridge.get_hw_info())
    print("Supported components: ")
    for component in bridge.get_supported_components():
        if component == multibus_protocol.MB_COMPONENT_ID_I2C_MASTER:
            print("- I2C Master")
    print("Bridge delay for 2000ms")
    bridge.delay_request(2000)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("Usage: python3 multibus_bridge_synchronous.py <path to serial port>")
    main(sys.argv[1])
