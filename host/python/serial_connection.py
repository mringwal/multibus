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

import serial  # pip install pyserial
import struct
from multibus_connection import MultibusConnection


class MultibusSerialConnection(MultibusConnection):
    def __init__(self, port: str, baud: int):
        self.port = port
        self.baud = baud
        self.connection = serial.Serial(self.port, self.baud, timeout=None, rtscts=False)

    def send_multibus_message(self, message: bytes):
        self.connection.write(message)

    def receive_multibus_message(self):
        header = self.connection.read(self.MB_HEADER_LEN)
        if len(header) < self.MB_HEADER_LEN:
            raise Exception("Received invalid message. TODO")

        (subsystem, opcode, channel, payload_len) = struct.unpack(">BBBH", header)
        if payload_len > 0:
            payload = self.connection.read(payload_len)
            return header, payload
        else:
            return header
