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
from time import sleep

PORT = '/dev/cu.usbserial-0001'
BAUDRATE = 115200

# open serial port, use 0.1 timeout for sync
ser = serial.Serial(PORT, BAUDRATE, timeout=None, rtscts=False)

# with Nordic devkits, UART is only activated after setting DTR
ser.dtr = True

MB_HEADER_LEN = 5
MB_SUBSYSTEM_I2C = 0x01
MB_I2C_READ = 0x02
MB_I2C_WRITE = 0x01

DEMO_I2C_CHANNEL_1 = 0
DEMO_I2C_BH1750_ADDR = 0x23
DEMO_I2C_BH1750_OPERATION_MODE = 0x23  # not an error - the same as the address

# TODO CONFIG MESSAGE
# Master
# Channel
# Pullup enable
# Enable/Disable ACK? -> Better into payload of read command?


# 1. set operation mode(e.g One time L-resolution mode)
# _________________________________________________________________
# | start | slave_addr + wr_bit + ack | write 1 byte + ack  | stop |
# --------|---------------------------|---------------------|------|
#
# 2. wait more than 24 ms
#
# 3. read data
# ______________________________________________________________________________________
# | start | slave_addr + rd_bit + ack | read 1 byte + ack  | read 1 byte + nack | stop |
# --------|---------------------------|--------------------|--------------------|------|

# 1. trigger measurement: I2C write payload: | 2B Slave Address | 1B sensor operation mode ]
ser.write(struct.pack('>BBBHHB', MB_SUBSYSTEM_I2C, MB_I2C_WRITE, DEMO_I2C_CHANNEL_1, 3,
                      DEMO_I2C_BH1750_ADDR, DEMO_I2C_BH1750_OPERATION_MODE))
data = ser.read(MB_HEADER_LEN)

if len(data) < MB_HEADER_LEN:
    while (1):
        print("data len %u" % len(data))
(subsystem, opcode, channel, payload_len) = struct.unpack(">BBBH", data)
print('Received:', 'System:', hex(subsystem), 'Opcode:', hex(opcode), 'Channel:', hex(channel), 'Payload len:', hex(payload_len))

if payload_len > 0:
    payload = ser.read(payload_len)
    # print("".join("0x%02x " % b for b in payload))
    print('Status:', ''.join("0x%02x " % payload[0]))
    print('Slave Address:', hex(payload[1]), hex(payload[2]))
    print(''.join("0x%02x " % b for b in payload))

# 2. delay 24ms...
sleep(0.03)

# 3. trigger read result: I2C read payload: | 2B Slave Address | 1B number of bytes to read | // todo mode?? enable ACK?
ser.write(struct.pack('>BBBHHB', MB_SUBSYSTEM_I2C, MB_I2C_READ, DEMO_I2C_CHANNEL_1, 3,
                      DEMO_I2C_BH1750_ADDR, 2))
data = ser.read(MB_HEADER_LEN)


if len(data) < MB_HEADER_LEN:
    while (1):
        print("data len %u" % len(data))
(subsystem, opcode, channel, payload_len) = struct.unpack(">BBBH", data)
print('Received:', 'System:', hex(subsystem), 'Opcode:', hex(opcode), 'Channel:', hex(channel), 'Payload len:', hex(payload_len))

if payload_len > 0:
    payload = ser.read(payload_len)
    # print("".join("0x%02x " % b for b in payload))
    print('Status:', ''.join("0x%02x " % payload[0]))
    print('Slave Address:', hex(payload[1]), hex(payload[2]))
    print(''.join("0x%02x " % b for b in payload))
    print("\nLUX: ", ((payload[3] << 8 | payload[4]) / 1.2))
