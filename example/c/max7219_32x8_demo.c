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
#include <unistd.h>

#include "multibus_protocol.h"
#include "multibus_serial_posix.h"
#include "multibus_transport.h"
#include "multibus_transport_protocol.h"
#include "font8x8_basic.h"
// This defines how many Max7219 modules we have cascaded together, in this case, we have 4 x 8x8 matrices giving a total of 32x8
#define NUM_MODULES 4

const uint8_t CMD_NOOP = 0;
const uint8_t CMD_DIGIT0 = 1; // Goes up to 8, for each line
const uint8_t CMD_DECODEMODE = 9;
const uint8_t CMD_BRIGHTNESS = 10;
const uint8_t CMD_SCANLIMIT = 11;
const uint8_t CMD_SHUTDOWN = 12;
const uint8_t CMD_DISPLAYTEST = 15;

// framebuffer
#define DISPLAY_WIDTH (NUM_MODULES * 8)
#define DISPLAY_HEIGHT 8
static uint8_t framebuffer[NUM_MODULES * DISPLAY_HEIGHT];

// static config
static const char * multibus_bridge_path;
static uint32_t     multibus_bridge_baudrate = 115200;

// transport instance
static uint8_t request_buffer[30];
static uint8_t response_buffer[20];
static mb_transport_t mb_transport;
static mb_serial_posix_context_t mb_serial_posix_context;

// blocking API
const mb_message_t * test_sync_message;

void test_sync_callback_handler(void * context, const mb_message_t * message){
    test_sync_message = message;
}

const mb_message_t * test_sync_wait_for_response(mb_transport_t * transport){
    test_sync_message = NULL;
    while (test_sync_message == NULL){
        bool can_sleep = true;
        can_sleep &= mb_serial_posix_process_read(&mb_serial_posix_context) == 0;
        can_sleep &= mb_serial_posix_process_write(&mb_serial_posix_context) == 0;
        if (can_sleep){
            // sleep 10 ms if no processing is active
            usleep(10 * 1000);
        }
    }
    return test_sync_message;
}

// minimal framebuffer

static void fb_clear(void){
    memset(framebuffer, 0, sizeof(framebuffer));
}

static void fb_set_pixel(uint16_t x, uint16_t y){
    if (x < NUM_MODULES * 8) {
        framebuffer[ (y*NUM_MODULES) + (x / 8)] |= 1 << ( 7 - (x & 7 ) );
    }
}

static void fb_draw_character(int16_t x, int16_t y, uint8_t code){
    if (code >= 128) return;
    uint8_t i;
    uint8_t j;
    for (i=0;i<8;i++){
        for (j=0;j<8;j++){
            if (x + j < 0) continue;
            if (font8x8_basic[code][i] & (1<<j)){
                fb_set_pixel(x + j, y + i);
            }
        }
    }
}

static void fb_draw_text(uint16_t offset, const char * text, uint16_t text_len){
    uint16_t start_pos = offset / 8;
    uint16_t i;
    for (i=0;i<(NUM_MODULES+1);i++){
        if (start_pos + i >= text_len) continue;
        fb_draw_character((start_pos + i) * 8 - offset, 0, text[start_pos + i]);
    }
}

static void max7219_write_register_all(mb_transport_t * transport, uint8_t address, uint8_t value){
    uint8_t buf[2 * NUM_MODULES];
    uint16_t i;
    for (i = 0; i< NUM_MODULES;i++) {
        buf[2*i] = address;
        buf[2*i+1] = value;
    }
    mb_transport_spi_master_write_request_send(transport, 0, sizeof(buf), buf);
    (void) test_sync_wait_for_response(transport);
}

static void max7219_update_framebuffer(mb_transport_t * transport){
    uint8_t buf[2 * NUM_MODULES];
    uint16_t i;
    uint16_t j;
    for (i = 0; i<DISPLAY_HEIGHT ; i++){
        for (j = 0; j<NUM_MODULES; j++) {
            uint8_t value = framebuffer[(i*NUM_MODULES) + j];
            buf[2*j]   = CMD_DIGIT0 + i;
            buf[2*j+1] = framebuffer[(i*NUM_MODULES) + j];
        }
        printf("\n");
        mb_transport_spi_master_write_request_send(transport, 0, sizeof(buf), buf);
        (void) test_sync_wait_for_response(transport);
    }
}

int main(int argc, const char **argv) {
    // get bridge path
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
    mb_transport_register_callback(&mb_transport, &test_sync_callback_handler, &mb_transport);

    mb_transport_t * transport = &mb_transport;

    // polling / synchronous API
    printf("Config SPI Master\n");
    mb_transport_spi_master_config_request_send(transport,
                                                0,
                                                8,
                                                MB_SPI_MASTER_CONFIG_REQUEST_BIT_ORDER_MSB_FIRST,
                                                MB_SPI_MASTER_CONFIG_REQUEST_CPOL_0,
                                                MB_SPI_MASTER_CONFIG_REQUEST_CPHA_0,
                                                1000000);
    (void) test_sync_wait_for_response(transport);

    // config
    max7219_write_register_all(transport, CMD_SHUTDOWN, 0);
    max7219_write_register_all(transport, CMD_DISPLAYTEST, 0);
    max7219_write_register_all(transport, CMD_SCANLIMIT, 7);  // Use all lines
    max7219_write_register_all(transport, CMD_DECODEMODE, 0); // No BCD decode, just use bit pattern.
    max7219_write_register_all(transport, CMD_SHUTDOWN, 1);
    max7219_write_register_all(transport, CMD_BRIGHTNESS, 8);

    uint16_t i;

    // simple vertical scroller
    for (i=0; i<16; i++) {
        for (int j=0; j<8; j++) {
            max7219_write_register_all(transport, CMD_DIGIT0+j, 1 << (i&7));
        }
        mb_transport_bridge_delay_request_send(transport, 0, 20);
        (void) test_sync_wait_for_response(transport);
    }

    // simple horizontal scroller
    for (i=0; i<16; i++) {
        for (int j=0; j<8; j++) {
            max7219_write_register_all(transport, CMD_DIGIT0+j, ((i&7) == j) ? 255 : 0);
        }
        mb_transport_bridge_delay_request_send(transport, 0, 20);
        (void) test_sync_wait_for_response(transport);
    }

    // chess board blinky
    int bright = 1;
    while (bright < 10) {
        for (i=0; i<8; i++) {
            if (bright & 1) {
                max7219_write_register_all(transport, CMD_DIGIT0 + i, 170 >> (i%2));
            } else {
                max7219_write_register_all(transport, CMD_DIGIT0 + i, (170 >> 1) << (i%2));
            }
        }
        max7219_write_register_all(transport, CMD_BRIGHTNESS, bright % 16);

        mb_transport_bridge_delay_request_send(transport, 0, 250);
        (void) test_sync_wait_for_response(transport);

        bright++;
    }

    // text scroller
    const char * text = "    Hello MultiBus!";
    uint16_t text_len = strlen(text);
    while (1){
        for (i=0;i< (text_len * 8);i++){
            fb_clear();
            fb_draw_text(i, text, text_len);
            max7219_update_framebuffer(transport);
            mb_transport_bridge_delay_request_send(transport, 0, 100);
            (void) test_sync_wait_for_response(transport);
        }
    }

    // close down
    mb_serial_posix_close(&mb_serial_posix_context);
    return 0;
}
