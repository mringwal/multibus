#include "CHardwareInfo.h"

std::string CHardwareInfo::getChipModel() {
  esp_chip_info_t lChipInfo;
  esp_chip_info(&lChipInfo);
  return smChipModelMap.at(lChipInfo.model);
}