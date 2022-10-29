/**
 *
 * Copyright 2022 Boris Zweimüller
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 * disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "multibus_protocol.h"
#include "CSerialMultiBusMessageReaderWriter.h"
#include <array>

CSerialMultiBusMessageReaderWriter::CSerialMultiBusMessageReaderWriter(std::shared_ptr<ISerial> aSerial) :
    mSerial(std::move(aSerial)) {}

SMultiBusMessage CSerialMultiBusMessageReaderWriter::readMultiBusMessage() const {

  SMultiBusMessage lMessage;

  // read and fill header
  std::array<uint8_t, MB_HEADER_SIZE> lPacket{};
  for (auto i = 0; i < MB_HEADER_SIZE; i++) {
    lPacket[i] = mSerial->readByte();
  }

  lMessage.mSubsystem = mb_header_get_component(lPacket.data());
  lMessage.mOpcode = mb_header_get_operation(lPacket.data());
  lMessage.mChannel = mb_header_get_channel(lPacket.data());
  lMessage.mLength = mb_header_get_length(lPacket.data());

  // read and fill payload
  for (auto i = 0; i < lMessage.mLength; i++) {
    lMessage.mPayload.push_back(mSerial->readByte());
  }
  return lMessage;
}

void CSerialMultiBusMessageReaderWriter::writeMultibusMessageBuffer(const std::span<uint8_t>& aData) const {
  mSerial->writeBytes(aData);
}
