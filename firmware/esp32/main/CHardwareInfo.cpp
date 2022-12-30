#include "CHardwareInfo.h"

std::string CHardwareInfo::getChipModel() {
  esp_chip_info_t lChipInfo;
  esp_chip_info(&lChipInfo);
  return smChipModelMap.at(lChipInfo.model);
}

uint32_t CHardwareInfo::getNumSpiPorts() {
    esp_chip_info_t lChipInfo;
    esp_chip_info(&lChipInfo);
    return smChipNumGeneralPurposeSpiPortsMap.at(lChipInfo.model);
}

spi_host_device_t CHardwareInfo::getSpiHostDeviceForMultibusChannelNumber(uint32_t aMultibusChannelNumber) {
    esp_chip_info_t lChipInfo;
    esp_chip_info(&lChipInfo);
    if (aMultibusChannelNumber >= smChipNumGeneralPurposeSpiPortsMap.at(lChipInfo.model)) {
        return SPI_HOST_MAX; // invalid host
    }
    return smChipToMultibusSpiNum.at(lChipInfo.model).at(aMultibusChannelNumber);
}
