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
import spi_master


BAUDRATE = 115200


def main(serial_port):
    multibus_connection = serial_connection.MultibusSerialConnection(serial_port, BAUDRATE)
    bridge = multibus_bridge.MultibusBridge(multibus_connection)
    spi = spi_master.SPIMaster(multibus_connection)

    print("Num SPI channels: %d on chip: %s" % (spi.get_number_of_spi_channels(), bridge.get_hw_info()))

    spi.configure_port(1, 0, 0, 0, 0, 1000000)
    spi.configure_port(1, 0, 0, 0, 0, 1000000)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("Usage: python3 multibus_i2c_master_synchronous.py <path to serial port>")
    main(sys.argv[1])