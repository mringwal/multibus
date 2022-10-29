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

#ifndef MULTIBUS_MAIN_I2C_CONFIG_OPERATION_INCLUDED
#define MULTIBUS_MAIN_I2C_CONFIG_OPERATION_INCLUDED

#include "IMultiBusOperation.h"
#include "IMultiBusMessageReaderWriter.h"
#include <array>
#include <driver/i2c.h>
#include <esp_log.h>
#include "CHardwareInfo.h"

class CI2ConfigOperation : public IMultiBusOperation {
  static constexpr int I2C_MASTER_SCL_IO = 19;
  static constexpr int I2C_MASTER_SDA_IO = 18;

 public:
  explicit CI2ConfigOperation(std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter)
      : mMultiBusReaderWriter(std::move(aMultiBusReaderWriter)) {}

  ~CI2ConfigOperation() override = default;

  void execute(const SMultiBusMessage &aMessage) override {
    ESP_LOGI("I2C", "i2c_master_config\n");

    esp_err_t lRet = ESP_OK;
    if (isPortConfigured(aMessage.mChannel)) {
      ESP_LOGI("I2C", "Reset I2C driver for channel: %d", aMessage.mChannel);
      lRet = i2c_driver_delete(aMessage.mChannel);
    }

    if (lRet == ESP_OK) {

      // TODO check arguments (0x1, 0x2 for speed etc., valid port etc.)
      i2c_config_t conf = {
          .mode = I2C_MODE_MASTER,
          .sda_io_num = I2C_MASTER_SDA_IO, // todo choose fixed pins for given i2c port number
          .scl_io_num = I2C_MASTER_SCL_IO, // todo choose fixed pins for given i2c port number
          .sda_pullup_en = static_cast<bool>(mb_i2c_master_config_request_get_enable_sda_pullup(aMessage.mPayload.data())),
          .scl_pullup_en = static_cast<bool>(mb_i2c_master_config_request_get_enable_scl_pullup(aMessage.mPayload.data())),
          .master = {.clk_speed = I2C_CLK_SPEED_100_KHZ},
          // Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here
          .clk_flags = 0,
      };

      lRet = i2c_param_config(aMessage.mChannel, &conf);

      if (lRet == ESP_OK) {
        // no rx and tx buffers needed for master mode
        lRet = i2c_driver_install(aMessage.mChannel, conf.mode, 0, 0, 0);
      }
    }

    uint16_t lLen;
    // status
    // TODO use constants for state
    if (lRet == ESP_OK) {
      mConfiguredI2cPorts[aMessage.mChannel] = true;
      lLen = mb_i2c_master_config_response_setup(sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel,MB_STATUS_OK);
    } else {
      ESP_LOGE("I2C", "I2C configure error: %d", lRet);
      lLen = mb_i2c_master_config_response_setup(sSendBuffer.data(), sSendBuffer.size(), aMessage.mChannel, MB_STATUS_UNKNOWN_ERROR);
    }

    mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
  }

 private:
  static constexpr int I2C_CLK_SPEED_100_KHZ = 100000;

  bool isPortConfigured(uint8_t aChannel) {
    return mConfiguredI2cPorts[aChannel];
  }

  std::shared_ptr<IMultiBusMessageReaderWriter> mMultiBusReaderWriter{};
  std::array<bool, CI2CMaster::I2C_NUMBER_OF_PORTS> mConfiguredI2cPorts{false};
};

#endif // MULTIBUS_MAIN_I2C_CONFIG_OPERATION_INCLUDED