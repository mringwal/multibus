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

#ifndef MULTIBUS_MAIN_I2C_READ_OPERATION_INCLUDED
#define MULTIBUS_MAIN_I2C_READ_OPERATION_INCLUDED

#include "IMultiBusOperation.h"
#include "IMultiBusMessageReaderWriter.h"
#include <array>
#include <driver/i2c.h>
#include <esp_log.h>


class CI2ReadOperation : public IMultiBusOperation {
 public:
  explicit CI2ReadOperation(std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter)
  : mMultiBusReaderWriter(std::move(aMultiBusReaderWriter)) {}

  ~CI2ReadOperation() override = default;

  void execute(const SMultiBusMessage& aMessage) override {
    ESP_LOGI("I2C", "i2c_master_read\n");

    // payload format: [2B slave][data]
    int lAckEnable = 0x1; // todo move to protocol into read operation

    const int lNumBytesToRead = mb_i2c_master_read_request_get_num_bytes(aMessage.mPayload.data());
    std::array<uint8_t, 1024> lReadBytes{}; // todo max read len

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // setup address and write mode
    // todo only supported 7bit address here -> correctly read out address for 10bit
    uint16_t lSlaveAddress = mb_i2c_master_read_request_get_address(aMessage.mPayload.data());
    i2c_master_write_byte(cmd, lSlaveAddress << 1 | I2C_MASTER_READ, lAckEnable);

    // all but the last are ACK'ed
    auto lPos = 0;
    for (; lPos < lNumBytesToRead - 1; lPos++) {
      i2c_master_read_byte(cmd, &lReadBytes[lPos], I2C_MASTER_ACK);
    }
    // after last byte, send NACK
    i2c_master_read_byte(cmd, &lReadBytes[lPos], I2C_MASTER_NACK);

    i2c_master_stop(cmd);
    esp_err_t lRet = i2c_master_cmd_begin(aMessage.mChannel, cmd, 1000 / portTICK_PERIOD_MS);
    ESP_LOGI("I2C", "I2C Read Result: 0x%X\n", lRet);
    i2c_cmd_link_delete(cmd);

    auto lLen = mb_i2c_master_read_response_setup(sSendBuffer.data(), sSendBuffer.size(),
                                      aMessage.mChannel,
                                      (lRet == ESP_OK) ? MB_STATUS_OK : MB_STATUS_UNKNOWN_ERROR,
                                      lSlaveAddress,
                                      lNumBytesToRead, lReadBytes.data());

    mMultiBusReaderWriter->writeMultibusMessageBuffer({sSendBuffer.begin(), sSendBuffer.begin() + lLen});
  }

 private:
  std::shared_ptr<IMultiBusMessageReaderWriter> mMultiBusReaderWriter{};
};

#endif // MULTIBUS_MAIN_I2C_READ_OPERATION_INCLUDED