/**
 *
 * Copyright 2023 Boris Zweimüller
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

#ifndef MULTIBUS_MAIN_SPI_MASTER_WRITE_OPERATION_INCLUDED
#define MULTIBUS_MAIN_SPI_MASTER_WRITE_OPERATION_INCLUDED

#include "IMultiBusOperation.h"
#include "IMultiBusMessageReaderWriter.h"
#include <esp_log.h>
#include "driver/spi_master.h"

class CSPIMasterWriteOperation : public IMultiBusOperation {
 public:
  explicit CSPIMasterWriteOperation(std::shared_ptr<CSPIMaster> aSpiMaster,
                                    std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter)
  : mSpiMaster(std::move(aSpiMaster)),
  mMultiBusReaderWriter(std::move(aMultiBusReaderWriter)) {}

  ~CSPIMasterWriteOperation() override = default;
  
  void execute(const SMultiBusMessage& aMessage) override {
    ESP_LOGI("SPI-MASTER", "spi_master_write\n");

    spi_host_device_t lSpiHost = CHardwareInfo::getSpiHostDeviceForMultibusChannelNumber(aMessage.mChannel);
    if (lSpiHost == SPI_HOST_MAX) {
      // invalid channel -> return error
      ESP_LOGE("SPI-MASTER", "SPI configure error: Received invalid channel configuration");
      auto lLen = mb_spi_master_config_response_setup(sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel,
                                                      MB_STATUS_INVALID_ARGUMENTS);
      mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
    }

    // if spi port not configured -> error
    auto lSpiDeviceHandle = mSpiMaster->getDeviceHandleForHost(lSpiHost);
    if (lSpiDeviceHandle == nullptr) {
      ESP_LOGE("SPI-MASTER", "SPI not configured for host: %d", lSpiHost);

      auto lLen = mb_spi_master_config_response_setup(sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel,
                                                      MB_STATUS_UNKNOWN_ERROR); // TODO status
      mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
    }

    spi_transaction_t lTransaction = {
        .flags = 0,
        .length = 8 * (size_t)mb_spi_master_write_request_get_data_len(aMessage.mLength),
        .tx_buffer = mb_spi_master_write_request_get_data(aMessage.mPayload.data())
    };

    if (auto lRet = spi_device_polling_transmit(lSpiDeviceHandle, &lTransaction) != ESP_OK) {
      ESP_LOGE("SPI-MASTER", "SPI write/transmit error: %d", lRet);

      auto lLen = mb_spi_master_config_response_setup(sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel,
                                                      MB_STATUS_UNKNOWN_ERROR); // TODO status
      mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
    }

    ESP_LOGI("SPI-MASTER", "SPI Write ok");
    auto lLen = mb_spi_master_config_response_setup(
        sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel, MB_STATUS_OK);
    mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
  }

 private:
  std::shared_ptr<CSPIMaster> mSpiMaster;
  std::shared_ptr<IMultiBusMessageReaderWriter> mMultiBusReaderWriter{};
};

#endif // MULTIBUS_MAIN_SPI_MASTER_WRITE_OPERATION_INCLUDED