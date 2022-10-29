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

#ifndef MULTIBUS_MAIN_C_BRIDGE_GET_PROTOCOL_VERSION_OPERATION_INCLUDED
#define MULTIBUS_MAIN_C_BRIDGE_GET_PROTOCOL_VERSION_OPERATION_INCLUDED

#include "IMultiBusOperation.h"
#include "CSerialMultiBusMessageReaderWriter.h"
#include "multibus_protocol.h"
#include <esp_log.h>

class CBridgeGetProtocolVersionOperation : public IMultiBusOperation {
 public:
  explicit CBridgeGetProtocolVersionOperation(std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter)
  : mMultiBusReaderWriter(std::move(aMultiBusReaderWriter)) {}

  ~CBridgeGetProtocolVersionOperation() override = default;

  void execute(const SMultiBusMessage& aMessage) override {
    ESP_LOGI("Bridge", "bridge_get_protocol_version\n");

    auto lLen = mb_bridge_protocol_version_response_setup(sSendBuffer.data(), sSendBuffer.size(), 0x0, MB_PROTOCOL_VERSION);
    mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
  }

 private:
  std::shared_ptr<IMultiBusMessageReaderWriter> mMultiBusReaderWriter;
};

#endif // MULTIBUS_MAIN_C_BRIDGE_GET_PROTOCOL_VERSION_OPERATION_INCLUDED