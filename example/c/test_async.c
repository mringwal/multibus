/**
 *
 * Copyright 2022 Matthias Ringwald
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "multibus_protocol.h"
#include "multibus_serial_posix.h"
#include "multibus_transport.h"
#include "multibus_transport_protocol.h"

// static config
static const char * multibus_bridge_path;
static uint32_t     multibus_bridge_baudrate = 115200;
static uint8_t      i2c_master_channel = 0;
static uint8_t      i2c_master_clock_speed = MB_I2C_MASTER_CONFIG_REQUEST_CLOCK_SPEED_100_KHZ;
static uint8_t      i2c_master_pullups_enabled = 1;
static uint8_t      lux_sensor_address = 0x23;
static const uint8_t lux_sensor_config = 0x23;
static bool         run_loop_done;

// transport instance
static uint8_t request_buffer[20];
static uint8_t response_buffer[20];
static mb_transport_t mb_transport;
static mb_serial_posix_context_t mb_serial_posix_context;

// app state
static enum  {
    APP_W2_GET_PROTOCOL,
    APP_W4_GET_PROTOCOL,
    APP_W4_I2C_READY,
    APP_W4_LUX_CONFIGURED,
    APP_W4_LUX_DELAY,
    APP_W4_I2C_LUX,
} app_state;

static void async_callback(void * context, const mb_message_t * message) {
    mb_transport_t * transport = (mb_transport_t *) context;
    const uint8_t * i2c_read_data;
    switch(app_state){
        case APP_W2_GET_PROTOCOL:
            printf("Get protocol version\n");
            app_state = APP_W4_GET_PROTOCOL;
            mb_transport_bridge_protocol_version_request_send(transport, 0);
            break;
        case APP_W4_GET_PROTOCOL:
            printf("Protocol Version: 0x%x\n", mb_message_bridge_protocol_version_response_get_version(message));
            printf("Config I2C Master\n");
            app_state = APP_W4_I2C_READY;
            mb_transport_i2c_master_config_request_send(transport, i2c_master_channel, i2c_master_clock_speed, i2c_master_pullups_enabled, i2c_master_pullups_enabled);
            break;
        case APP_W4_I2C_READY:
            printf("Write configuration\n");
            app_state = APP_W4_LUX_CONFIGURED;
            mb_transport_i2c_master_write_request_send(transport, i2c_master_channel, lux_sensor_address, 1, &lux_sensor_config);
            break;
        case APP_W4_LUX_CONFIGURED:
            printf("Delay 30ms\n");
            app_state = APP_W4_LUX_DELAY;
            mb_transport_bridge_delay_request_send(transport, 0, 30);
            break;
        case APP_W4_LUX_DELAY:
            printf("Read LUX\n");
            app_state = APP_W4_I2C_LUX;
            mb_transport_i2c_master_read_request_send(transport, i2c_master_channel, lux_sensor_address, 2);
            break;
        case APP_W4_I2C_LUX:
            i2c_read_data = mb_message_i2c_master_read_response_get_data(message);
            printf("Lux: %f\n", (i2c_read_data[0] << 8 | i2c_read_data[1]) / 1.2);
            run_loop_done = true;
            break;
        default:
            break;
    }
}

int main(int argc, const char **argv) {
    // get brdige path
    if (argc != 2){
        printf("Usage: %s <path to serial port>\n", argv[0]);
        exit(10);
    }
    multibus_bridge_path = argv[1];

    // open serial transport
    bool ok = mb_serial_posix_open(&mb_serial_posix_context, multibus_bridge_path, multibus_bridge_baudrate);
    if (!ok) return 10;

    // setup transport interface
    const mb_driver_t * driver_impl = mb_serial_posix_get_driver();
    mb_transport_create(&mb_transport, driver_impl, &mb_serial_posix_context,
                        request_buffer, sizeof(request_buffer),
                        response_buffer, sizeof(response_buffer));
    mb_transport_register_callback(&mb_transport, &async_callback, &mb_transport);

    // get started
    app_state = APP_W2_GET_PROTOCOL;
    async_callback(&mb_transport, NULL);

    // run loop
    while (run_loop_done == false){
        mb_serial_posix_process_read(&mb_serial_posix_context);
        mb_serial_posix_process_write(&mb_serial_posix_context);
    }

    // close down
    mb_serial_posix_close(&mb_serial_posix_context);
    return 0;
}
