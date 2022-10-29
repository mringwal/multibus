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

#ifndef MULTIBUS_MAIN_MULTIBUS_HARDWARE_INFO_INCLUDED
#define MULTIBUS_MAIN_MULTIBUS_HARDWARE_INFO_INCLUDED

#include <string>
#include <esp_chip_info.h>
#include <map>

class CHardwareInfo {
 public:
  std::string getChipModel();

  static inline const std::map<esp_chip_model_t , std::string> smChipModelMap = {
      {CHIP_ESP32, "ESP32"},
      {CHIP_ESP32S2, "ESP32S2"},
      {CHIP_ESP32S3, "ESP32S3"},
      {CHIP_ESP32C3, "ESP32C3"},
      {CHIP_ESP32H2, "ESP32H2"},
      {CHIP_ESP32C2, "ESP32C2"},
  };
};

#endif //MULTIBUS_MAIN_MULTIBUS_HARDWARE_INFO_INCLUDED
