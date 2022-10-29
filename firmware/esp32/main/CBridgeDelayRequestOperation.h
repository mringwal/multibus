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

#ifndef MULTIBUS_MAIN_C_BRIDGE_DELAY_REQUEST_OPERATION_INCLUDED
#define MULTIBUS_MAIN_C_BRIDGE_DELAY_REQUEST_OPERATION_INCLUDED

#include "IMultiBusOperation.h"
#include "IMultiBusMessageReaderWriter.h"
#include <esp_log.h>
#include <string>
#include <multibus_protocol.h>
#include <memory>
#include <thread>

class CBridgeDelayRequestOperation : public IMultiBusOperation {
 public:
  explicit CBridgeDelayRequestOperation(std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter)
  : mMultiBusReaderWriter(std::move(aMultiBusReaderWriter)) {}

  ~CBridgeDelayRequestOperation() override = default;

  void execute(const SMultiBusMessage& aMessage) override {
    ESP_LOGI("Bridge", "bridge_delay_request\n");

    auto timeoutMs = mb_bridge_delay_request_get_timeout_ms(aMessage.mPayload.data());
    ESP_LOGI("Bridge", "sleeping for %dms\n", (int)timeoutMs);
    std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));

    auto lLen = mb_bridge_delay_response_setup(sSendBuffer.data(), sSendBuffer.size(), 0x0);

    mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
  }

 private:
  std::shared_ptr<IMultiBusMessageReaderWriter> mMultiBusReaderWriter{};
};

#endif // MULTIBUS_MAIN_C_BRIDGE_DELAY_REQUEST_OPERATION_INCLUDED