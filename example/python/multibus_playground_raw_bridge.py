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

import serial
import struct

# port = '/dev/cu.usbserial-110'
PORT = '/dev/cu.usbserial-0001'
BAUDRATE = 115200

# open serial port, use 0.1 timeout for sync
ser = serial.Serial(PORT, BAUDRATE, timeout=None, rtscts=False)
# ser = serial.Serial(port, baud)

# with Nordic devkits, UART is only activated after setting DTR
ser.dtr = True

MB_HEADER_LEN = 5
MB_SUBSYSTEM_BRIDGE = 0x00
MB_BRIDGE_GET_PROTOCOL_VERSION = 0x00
MB_BRIDGE_GET_HW_INFO = 0x01

# reset sniffer
ser.write(struct.pack('>BBBH', MB_SUBSYSTEM_BRIDGE, MB_BRIDGE_GET_PROTOCOL_VERSION, 0, 0))
# ser.write(struct.pack('>BBBHBB', MB_SUBSYSTEM_BRIDGE, MB_BRIDGE_GET_PROTOCOL_VERSION, 0, 2, 0x33, 0x55))

# get protocol version
data = ser.read(MB_HEADER_LEN)
#
if len(data) < MB_HEADER_LEN:
    while (1):
        print("data len %u" % len(data))
(subsystem, opcode, channel, payload_len) = struct.unpack(">BBBH", data)
print(hex(subsystem), hex(opcode), hex(channel), hex(payload_len))

if payload_len > 0:
    payload = ser.read(payload_len)
    print('payload', payload)

# # get hw info
ser.write(struct.pack('>BBBH', MB_SUBSYSTEM_BRIDGE, MB_BRIDGE_GET_HW_INFO, 0, 0))

data = ser.read(MB_HEADER_LEN)

if len(data) < MB_HEADER_LEN:
    while (1):
        print("data len %u" % len(data))
(subsystem, opcode, channel, payload_len) = struct.unpack(">BBBH", data)
print(hex(subsystem), hex(opcode), hex(channel), hex(payload_len))

if payload_len > 0:
    payload = ser.read(payload_len)
    print('payload', payload)

