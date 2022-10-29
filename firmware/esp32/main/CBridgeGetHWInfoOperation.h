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

#ifndef MULTIBUS_MAIN_C_BRIDGE_GET_HW_INFO_OPERATION_INCLUDED
#define MULTIBUS_MAIN_C_BRIDGE_GET_HW_INFO_OPERATION_INCLUDED

#include "IMultiBusOperation.h"
#include "IMultiBusMessageReaderWriter.h"
#include "CHardwareInfo.h"
#include <esp_log.h>
#include <string>
#include <multibus_protocol.h>

class CBridgeGetHWInfoOperation : public IMultiBusOperation {
 public:
  explicit CBridgeGetHWInfoOperation(std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter,
                                     std::shared_ptr<CHardwareInfo> aHwInfo)
      : mMultiBusReaderWriter(std::move(aMultiBusReaderWriter)),
        mHwInfo(aHwInfo) {}

  ~CBridgeGetHWInfoOperation() override = default;

  void execute(const SMultiBusMessage &aMessage) override {
    ESP_LOGI("Bridge", "bridge_get_hw_info\n");

    auto lLen = mb_bridge_hardware_info_response_setup(sSendBuffer.data(),
                                                       sSendBuffer.size(), 0x0, mHwInfo->getChipModel().c_str());
    mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
  }

 private:
  std::shared_ptr<IMultiBusMessageReaderWriter> mMultiBusReaderWriter{};
  std::shared_ptr<CHardwareInfo> mHwInfo{};
};

#endif // MULTIBUS_MAIN_C_BRIDGE_GET_HW_INFO_OPERATION_INCLUDED