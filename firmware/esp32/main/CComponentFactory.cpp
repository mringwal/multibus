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

#include "CComponentFactory.h"
#include <CBridge.h>
#include <CI2CMaster.h>
#include "CSPIMaster.h"
#include "CBridgeGetProtocolVersionOperation.h"
#include "CBridgeGetFirmwareVersionOperation.h"
#include "CBridgeGetHWInfoOperation.h"
#include "CBridgeGetSupportedComponentsOperation.h"
#include "CBridgeDelayRequestOperation.h"
#include "CI2CReadOperation.h"
#include "CI2CWriteOperation.h"
#include "CI2CConfigOperation.h"
#include "CSPIGetNumChannelsOperation.h"
#include "CSPIConfigOperation.h"

std::shared_ptr<IComponent>
CComponentFactory::createBridgeComponent(std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter) {
  auto lBridge = std::make_shared<CBridge>();
  // operations
  auto lBridgeGetProtocolVersionOperation = std::make_shared<CBridgeGetProtocolVersionOperation>(
      aMultiBusReaderWriter);
  auto lBridgeGetFirmwareVersionOperation = std::make_shared<CBridgeGetFirmwareVersionOperation>(
      aMultiBusReaderWriter);
  auto lBridgeGetHWInfoOperation = std::make_shared<CBridgeGetHWInfoOperation>(aMultiBusReaderWriter);
  auto lBridgeGetSupportedComponentsOperation = std::make_shared<CBridgeGetSupportedComponentsOperation>(
      aMultiBusReaderWriter);
  auto lBridgeDelayRequestOperation = std::make_shared<CBridgeDelayRequestOperation>(aMultiBusReaderWriter);

  lBridge->registerOperation(MB_OPERATION_BRIDGE_PROTOCOL_VERSION_REQUEST, lBridgeGetProtocolVersionOperation);
  lBridge->registerOperation(MB_OPERATION_BRIDGE_HARDWARE_INFO_REQUEST, lBridgeGetHWInfoOperation);
  lBridge->registerOperation(MB_OPERATION_BRIDGE_FIRMWARE_VERSION_REQUEST, lBridgeGetFirmwareVersionOperation);
  lBridge->registerOperation(MB_OPERATION_BRIDGE_SUPPORTED_COMPONENTS_REQUEST,
                             lBridgeGetSupportedComponentsOperation);
  lBridge->registerOperation(MB_OPERATION_BRIDGE_DELAY_REQUEST, lBridgeDelayRequestOperation);

  return lBridge;
}

std::shared_ptr<IComponent>
CComponentFactory::createI2CMasterComponent(std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter) {
  std::array<bool, CI2CMaster::I2C_NUMBER_OF_PORTS> lConfiguredI2CPorts{}; // todo
  auto lI2cMaster = std::make_shared<CI2CMaster>(lConfiguredI2CPorts);

  // i2c operations
  auto lI2CReadOperation = std::make_shared<CI2ReadOperation>(aMultiBusReaderWriter);
  auto lI2CWriteOperation = std::make_shared<CI2CWriteOperation>(aMultiBusReaderWriter);
  auto lI2CConfigOperation = std::make_shared<CI2ConfigOperation>(aMultiBusReaderWriter);

  lI2cMaster->registerOperation(MB_OPERATION_I2C_MASTER_READ_REQUEST, lI2CReadOperation);
  lI2cMaster->registerOperation(MB_OPERATION_I2C_MASTER_WRITE_REQUEST, lI2CWriteOperation);
  lI2cMaster->registerOperation(MB_OPERATION_I2C_MASTER_CONFIG_REQUEST, lI2CConfigOperation);
  return lI2cMaster;
}

std::shared_ptr<IComponent>
CComponentFactory::createSPIMasterComponent(std::shared_ptr<IMultiBusMessageReaderWriter> aMultiBusReaderWriter) {
    // todo configued ports??
    auto lSpiMaster = std::make_shared<CSPIMaster>();

    // spi operations
    auto lSpiGetNumChannelsOperation = std::make_shared<CPIGetNumChannelsOperation>(aMultiBusReaderWriter);
    auto lSpiConfigOperation = std::make_shared<CSPIConfigOperation>(lSpiMaster, aMultiBusReaderWriter);

    lSpiMaster->registerOperation(MB_OPERATION_SPI_MASTER_GET_NUM_CHANNELS_REQUEST, lSpiGetNumChannelsOperation);
    lSpiMaster->registerOperation(MB_OPERATION_SPI_MASTER_CONFIG_REQUEST, lSpiConfigOperation);

    return lSpiMaster;
}
