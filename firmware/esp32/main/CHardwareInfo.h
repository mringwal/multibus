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
#include <hal/spi_types.h>

class CHardwareInfo {
public:
    /**
     * Get the model of the current ESP chip.
     *
     * @return chip model as a std::string.
     */
    static std::string getChipModel();

    /**
     * Get the number of general purpose SPI ports for the current chip.
     *
     * @return  number of general purpose SPI ports.
     */
    static uint32_t getNumSpiPorts();

    static spi_host_device_t getSpiHostDeviceForMultibusChannelNumber(uint32_t aMultibusChannelNumber);

    static inline const std::map<const esp_chip_model_t, const std::string> smChipModelMap = {
            {CHIP_ESP32,   "ESP32"},
            {CHIP_ESP32S2, "ESP32S2"},
            {CHIP_ESP32S3, "ESP32S3"},
            {CHIP_ESP32C2, "ESP32C2"},
            {CHIP_ESP32C3, "ESP32C3"},
            {CHIP_ESP32H2, "ESP32H2"},
    };

    static inline const std::map<const esp_chip_model_t, const uint32_t> smChipNumGeneralPurposeSpiPortsMap = {
            {CHIP_ESP32,   2},
            {CHIP_ESP32S2, 2},
            {CHIP_ESP32S3, 2},
            {CHIP_ESP32C2, 1},
            {CHIP_ESP32C3, 1},
            {CHIP_ESP32H2, 0},
    };

    static inline const std::map<const esp_chip_model_t, std::map<uint32_t, spi_host_device_t>> smChipToMultibusSpiNum = {
            {CHIP_ESP32, {{0, SPI2_HOST}, {1, SPI3_HOST}}},
            {CHIP_ESP32S2, {{0, SPI2_HOST}, {1, SPI3_HOST}}},
            {CHIP_ESP32S3, {{0, SPI2_HOST}, {1, SPI3_HOST}}},
            {CHIP_ESP32C2, {{0, SPI2_HOST}}},
            {CHIP_ESP32C2, {{0, SPI2_HOST}}},
    };
};

#endif //MULTIBUS_MAIN_MULTIBUS_HARDWARE_INFO_INCLUDED
