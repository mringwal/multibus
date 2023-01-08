/**
 *
 * Copyright 2023 Boris Zweimüller
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

#ifndef MULTIBUS_MAIN_SPIMASTER_INCLUDED
#define MULTIBUS_MAIN_SPIMASTER_INCLUDED

#include <driver/spi_master.h>
#include "multibus_protocol.h"
#include "CComponent.h"
#include "CHardwareInfo.h"
#include <map>

class CSPIMaster : public CComponent<mb_operation_spi_master_t> {
public:
    CSPIMaster();

    void configureHost(spi_host_device_t aSpiHost, spi_device_handle_t aDeviceHandle);
    void removeConfiguredHost(spi_host_device_t);
    spi_device_handle_t getDeviceHandleForHost(spi_host_device_t);

private:
    std::map<spi_host_device_t, spi_device_handle_t> mConfiguredSpiHosts;
};

#endif //MULTIBUS_MAIN_SPIMASTER_INCLUDED
