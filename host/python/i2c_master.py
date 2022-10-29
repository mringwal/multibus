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

from multibus_connection import MultibusConnection

import os
PATH_TO_PYTHON_BINDING_GENERATOR = "../../protocol/generator-python.py"
PYTHON_BINDING_GENERATOR_TARGET = "../../host/python/generated"
PATH_TO_PYTHON_BINDINGS = 'generated/multibus_protocol.py'

if not os.path.exists(PATH_TO_PYTHON_BINDINGS):
    print("Python protocol bindings do not exist: Generating multibus_protocol.py")
    os.system(PATH_TO_PYTHON_BINDING_GENERATOR + ' ' + PYTHON_BINDING_GENERATOR_TARGET)
    print("Generated python protocol bindings successfully.")
    # TODO handle errors during generation of protocol bindings.

from generated import multibus_protocol


class I2CMaster:

    def __init__(self,
                 multibus_connection: MultibusConnection,
                 i2c_channel: int):
        self.multibus_connection = multibus_connection
        self.i2c_channel = i2c_channel

        message = multibus_protocol.mb_i2c_master_config_request_setup(self.i2c_channel, 0x1, 0x1)

        self.multibus_connection.send_multibus_message(message)

        payload = self.multibus_connection.receive_multibus_message()[1]
        status = multibus_protocol.mb_i2c_master_config_response(payload)
        if status != multibus_protocol.MB_STATUS_OK:
            raise Exception("TODO: error during i2c config")

    def write_bytes(self, slave_address: int, bytes_to_write: bytes):
        message = multibus_protocol.mb_i2c_master_write_request_setup(self.i2c_channel, slave_address, len(bytes_to_write), bytes_to_write)

        self.multibus_connection.send_multibus_message(message)

        payload = self.multibus_connection.receive_multibus_message()[1]
        status, address = multibus_protocol.mb_i2c_master_write_response(payload)

        if status != multibus_protocol.MB_STATUS_OK:
            raise Exception("TODO: error during i2c write")

    def read_bytes(self, slave_address: int, num_bytes_to_read: int):
        message = multibus_protocol.mb_i2c_master_read_request_setup(self.i2c_channel, slave_address, num_bytes_to_read)

        self.multibus_connection.send_multibus_message(message)

        payload = self.multibus_connection.receive_multibus_message()[1]
        status, address, data = multibus_protocol.mb_i2c_master_read_response(payload)

        if status != multibus_protocol.MB_STATUS_OK:
            raise Exception("TODO: error during i2c write")
        # todo check slave
        return data
