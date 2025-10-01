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
#include "CSerialMultiBusMessageReaderWriter.h"
#include "CMultiBusOperationExecutor.h"
#include "CComponentFactory.h"
#include "CUartSerial.h"

#define MULTIBUS_UART_NUM UART_NUM_1

extern "C" {
void app_main(void);
}

void app_main(void) {
    auto lUart = std::make_shared<CUartSerial>(MULTIBUS_UART_NUM, 4, 5, 115200, 2048);
    auto lMessageReaderWriter = std::make_shared<CSerialMultiBusMessageReaderWriter>(lUart);
    auto lOperationExecutor = std::make_shared<CMultiBusOperationExecutor>();

    /* BRIDGE */
    auto lBridge = CComponentFactory::createBridgeComponent(lMessageReaderWriter);
    lOperationExecutor->registerComponent(MB_COMPONENT_BRIDGE, lBridge);

    /* I2C MASTER */
    auto lI2CMaster = CComponentFactory::createI2CMasterComponent(lMessageReaderWriter);
    lOperationExecutor->registerComponent(MB_COMPONENT_I2C_MASTER, lI2CMaster);

    /* SPI MASTER */
    auto lSPIMaster = CComponentFactory::createSPIMasterComponent(lMessageReaderWriter);
    lOperationExecutor->registerComponent(MB_COMPONENT_SPI_MASTER, lSPIMaster);

    while (true) {
        auto lMessage = lMessageReaderWriter->readMultiBusMessage();
        ESP_LOGD("Bridge", "---------------------------------------------------");
        ESP_LOGD("Bridge", "Received:");
//        lMessage.print();
        lOperationExecutor->execute(lMessage);
    }
}
