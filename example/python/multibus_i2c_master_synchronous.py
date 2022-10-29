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
import i2c_master
import serial_connection


BAUDRATE = 115200


def main(serial_port):
    multibus_connection = serial_connection.MultibusSerialConnection(serial_port, BAUDRATE)

    i2c = i2c_master.I2CMaster(multibus_connection, i2c_channel=0)
    i2c.write_bytes(slave_address=0x23, bytes_to_write=b'\x23')
    sleep(0.03)
    response = i2c.read_bytes(slave_address=0x23, num_bytes_to_read=2)

    print("\nLUX: ", ((response[0] << 8 | response[1]) / 1.2))
    sleep(0.01)

    # do it again - driver is internally reset
    i2c = i2c_master.I2CMaster(multibus_connection, i2c_channel=0)
    i2c.write_bytes(slave_address=0x23, bytes_to_write=b'\x23')
    sleep(0.03)
    response = i2c.read_bytes(slave_address=0x23, num_bytes_to_read=2)

    print("\nLUX: ", ((response[0] << 8 | response[1]) / 1.2))


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("Usage: python3 multibus_i2c_master_synchronous.py <path to serial port>")
    main(sys.argv[1])