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

import sys
import time

sys.path.insert(1, '../../../multibus/host/python')
import serial_connection
import multibus_bridge
import spi_master


BAUDRATE = 115200

NUM_MODULES = 4

DECODE_MODE_REG = 0x09
BRIGHTNESS_REG = 0x0A
SCAN_LIMIT_REG = 0x0B
SHUTDOWN_REG = 0x0C
DISPLAY_TEST_REG = 0x0F

def write_reg(spi: spi_master.SPIMaster, reg: int, value: int):
    data = bytearray([reg, value])
    data = data * NUM_MODULES
    spi.write(1, data)


def clear_display(spi):
    for i in range(1,9):
        write_reg(spi, i, 0x00)


def set_col(spi, col_index):
    for i in range(1, 9):
        write_reg(spi, i, 0x01 << col_index);


def main(serial_port):
    multibus_connection = serial_connection.MultibusSerialConnection(serial_port, BAUDRATE)
    bridge = multibus_bridge.MultibusBridge(multibus_connection)
    spi = spi_master.SPIMaster(multibus_connection)

    print("Num SPI channels: %d on chip: %s" % (spi.get_number_of_spi_channels(), bridge.get_hw_info()))

    spi.configure_port(1, 0, 0, 0, 0, 1000000)

    write_reg(spi, SHUTDOWN_REG, 0x0)
    write_reg(spi, DISPLAY_TEST_REG, 0x0)
    write_reg(spi, SCAN_LIMIT_REG, 0x7)
    write_reg(spi, DECODE_MODE_REG, 0x0)
    write_reg(spi, SHUTDOWN_REG, 0x1)
    write_reg(spi, BRIGHTNESS_REG, 0x2 & 0x0f)
    clear_display(spi)
    
    for j in range(0, 10):
        for i in range(0, 8):
            set_col(spi, i)
            bridge.delay_request(20)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("Usage: python3 multibus_spi_master_synchronous.py <path to serial port>")
    main(sys.argv[1])