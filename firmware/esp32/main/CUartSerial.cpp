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

#include <esp_log.h>
#include "CUartSerial.h"
#include "driver/uart.h"

CUartSerial::CUartSerial(uart_port_t aUartPort, uint32_t aRxPin, uint32_t aTxPin,
                         int aBaudRate, uint32_t aRxBufferSize) : mUartPort(aUartPort) {

  uart_config_t uart_config = {
      .baud_rate = aBaudRate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0, // not used
      .source_clk = UART_SCLK_APB,
  };
  int intr_alloc_flags = 0;

  ESP_ERROR_CHECK(uart_driver_install(aUartPort, aRxBufferSize * 2, 0, 0, nullptr, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(aUartPort, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(aUartPort, aTxPin, aRxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

  ESP_LOGI("UART", "Initialized UART for MultiBus communication successfully.");
}

uint8_t CUartSerial::readByte() {
  uint8_t data[1];
  int len = 0;
  while (len <= 0) {
    len = uart_read_bytes(mUartPort, data, 1, 20 / portTICK_PERIOD_MS);
  }
//  printf("READ 0x%X\n", data[0]);
  return data[0];
}

void CUartSerial::writeBytes(const std::span<uint8_t>& aData) {
//  printf("data to send: %d\n", aData.size());
//  for (auto &b: aData) {
//    printf("0x%X ", b);
//  }
//  printf("\n");

  uart_write_bytes(mUartPort, aData.data(), aData.size());
}