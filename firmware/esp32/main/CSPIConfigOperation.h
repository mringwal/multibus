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

#ifndef MULTIBUS_MAIN_SPI_CONFIG_OPERATION_INCLUDED
#define MULTIBUS_MAIN_SPI_CONFIG_OPERATION_INCLUDED

#include "IMultiBusOperation.h"
#include "IMultiBusMessageReaderWriter.h"
#include "driver/spi_master.h"
#include <esp_log.h>
#include "CHardwareInfo.h"
#include "CSPIMaster.h"

class CSPIConfigOperation : public IMultiBusOperation {
  static constexpr int SPI_MASTER_MOSI_IO = 32;
  static constexpr int SPI_MASTER_MISO_IO = -1;
  static constexpr int SPI_MASTER_CLK_IO = 33;
  static constexpr int SPI_MASTER_CS_IO = 25;

  static constexpr int SPI_MASTER_MAX_TRANSFER_SIZE = 32;

 public:
  explicit CSPIConfigOperation(std::shared_ptr<CSPIMaster> aSpiMaster,
                               std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter)
      : mSpiMaster(std::move(aSpiMaster)),
        mMultiBusReaderWriter(std::move(aMultiBusReaderWriter)) {}

  ~CSPIConfigOperation() override = default;

  void execute(const SMultiBusMessage &aMessage) override {
    ESP_LOGI("SPI-MASTER", "spi_master_config\n");

    spi_host_device_t lSpiHost = CHardwareInfo::getSpiHostDeviceForMultibusChannelNumber(aMessage.mChannel);
    if (lSpiHost == SPI_HOST_MAX) {
      // invalid channel -> return error
      ESP_LOGE("SPI-MASTER", "SPI configure error: Received invalid channel configuration");
      auto lLen = mb_spi_master_config_response_setup(sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel,
                                                      MB_STATUS_INVALID_ARGUMENTS);
      mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
    }

    // if spi port is already configure -> return error
    if (mSpiMaster->getDeviceHandleForHost(lSpiHost) != nullptr) {
      ESP_LOGW("SPI-MASTER", "SPI configure warning: SPI Host: %d is already configured - reset config.", lSpiHost);

      spi_bus_remove_device(mSpiMaster->getDeviceHandleForHost(lSpiHost));
      spi_bus_free(lSpiHost);

      mSpiMaster->removeConfiguredHost(lSpiHost);
    }

    // configure
    spi_bus_config_t lBusCfg = {
        .mosi_io_num = SPI_MASTER_MOSI_IO,
        .miso_io_num = SPI_MASTER_MISO_IO, // TODO not used for now
        .sclk_io_num = SPI_MASTER_CLK_IO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_MASTER_MAX_TRANSFER_SIZE,
    };

    if (auto lRet = spi_bus_initialize(lSpiHost, &lBusCfg, SPI_DMA_CH_AUTO) != ESP_OK) {
      // spi initialization failed - return error
      ESP_LOGE("SPI-MASTER", "SPI initialization error: %d", lRet);

      auto lLen = mb_spi_master_config_response_setup(sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel,
                                                      MB_STATUS_UNKNOWN_ERROR);
      mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
    }
    ESP_LOGI("SPI-MASTER", "SPI Master: %d configured successfully.", lSpiHost);

    spi_device_interface_config_t lDeviceConfig = {
        .mode = 0, // SPI mode 0 // TODO
        .clock_speed_hz = (int)mb_spi_master_config_request_get_baud_rate(aMessage.mPayload.data()),
        .spics_io_num = SPI_MASTER_CS_IO, // TODO support different CS per host
        .flags = SPI_DEVICE_HALFDUPLEX,
        .queue_size = 1,
        .pre_cb = nullptr,
        .post_cb = nullptr,
    };

    spi_device_handle_t lDeviceHandle;
    if (auto lRet = spi_bus_add_device(lSpiHost, &lDeviceConfig, &lDeviceHandle) != ESP_OK) {
      ESP_LOGE("SPI-MASTER", "SPI configure slave error: %d", lRet);

      auto lLen = mb_spi_master_config_response_setup(sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel,
                                                      MB_STATUS_UNKNOWN_ERROR);
      mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
    }

    ESP_LOGI("SPI-MASTER", "SPI Slave attached successfully.");

    mSpiMaster->configureHost(lSpiHost, lDeviceHandle);


    // everything ok
    auto lLen = mb_spi_master_config_response_setup(
        sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel, MB_STATUS_OK);
    mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
  }

 private:
  std::shared_ptr<CSPIMaster> mSpiMaster;
  std::shared_ptr<IMultiBusMessageReaderWriter> mMultiBusReaderWriter{};
};

#endif // MULTIBUS_MAIN_SPI_CONFIG_OPERATION_INCLUDED