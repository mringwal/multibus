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


class SPIMaster:
    def __init__(self,
                 multibus_connection: MultibusConnection):
        self.multibus_connection = multibus_connection

    def get_number_of_spi_channels(self):
        message = multibus_protocol.mb_spi_master_get_num_channels_request_setup(0x0)
        self.multibus_connection.send_multibus_message(message)
        payload = self.multibus_connection.receive_multibus_message()[1]
        return multibus_protocol.mb_spi_master_get_num_channels_response(payload)

    def configure_port(self, spi_channel, data_bits, bit_order, cpol, cpha, baud_rate):
        message = multibus_protocol.mb_spi_master_config_request_setup(
            spi_channel, data_bits, bit_order, cpol, cpha, baud_rate)

        self.multibus_connection.send_multibus_message(message)

        payload = self.multibus_connection.receive_multibus_message()[1]
        status = multibus_protocol.mb_i2c_master_config_response(payload)
        if status != multibus_protocol.MB_STATUS_OK:
            raise Exception("TODO: error during spi config")

    def write(self, spi_channel: int, bytes_to_write: bytes):
        message = multibus_protocol.mb_spi_master_write_request_setup(
            spi_channel, len(bytes_to_write), bytes_to_write)

        self.multibus_connection.send_multibus_message(message)

        payload = self.multibus_connection.receive_multibus_message()[1]
        status = multibus_protocol.mb_i2c_master_config_response(payload)
        if status != multibus_protocol.MB_STATUS_OK:
            raise Exception("TODO: error during spi write")