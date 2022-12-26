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

#include <stdio.h>
#include <inttypes.h>

#include <ctype.h>

#include "bsp/board.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "tusb.h"
#include "pico/unique_id.h"

#include "usb_serial.h"
#include "multibus_protocol.h"

#define FIRMWARE_VERSION 0

#define MAX_MESSSAGE_LEN 1024

static enum {
    CDC_W4_HEADER,
    CDC_W4_PAYLOAD,
    CDC_PROCESS_REQUEST,
    CDC_SEND_RESPONSE
} cdc_protocol_state;

//------------- prototypes -------------//
static void cdc_task(void);

//------------- globals -------------//
char usb_serial[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

static const uint8_t cdc_itf = 0;

static uint8_t cdc_request[MAX_MESSSAGE_LEN];
static uint32_t cdc_request_len;
static uint32_t cdc_bytes_to_read;

static uint8_t cdc_response[MAX_MESSSAGE_LEN];
static uint32_t cdc_response_len;
static uint32_t cdc_response_offset;

static const uint8_t supported_components[] = {MB_COMPONENT_I2C_MASTER, MB_COMPONENT_SPI_MASTER};

//------------- utils -------------//
static uint32_t mb_min(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

char char_for_nibble(int nibble) {

    static const char *char_to_nibble = "0123456789ABCDEF";

    if (nibble < 16) {
        return char_to_nibble[nibble];
    } else {
        return '?';
    }
}

static inline char char_for_high_nibble(int value) {
    return char_for_nibble((value >> 4) & 0x0f);
}

static inline char char_for_low_nibble(int value) {
    return char_for_nibble(value & 0x0f);
}

void printf_hexdump(const void *data, int size) {
    char buffer[4];
    buffer[2] = ' ';
    buffer[3] = 0;
    const uint8_t *ptr = (const uint8_t *) data;
    while (size > 0) {
        uint8_t byte = *ptr++;
        buffer[0] = char_for_high_nibble(byte);
        buffer[1] = char_for_low_nibble(byte);
        printf("%s", buffer);
        size--;
    }
    printf("\n");
}

//--------------------------------------------------------------------+
// MultiBus Component Bridge
//--------------------------------------------------------------------+
static bool mb_component_bridge_handle_request(const uint8_t * payload_data, uint16_t payload_len) {
    (void) payload_data;
    (void) payload_len;
    char hardware_info[30];
    switch (mb_header_get_operation(cdc_request)) {
        case MB_OPERATION_BRIDGE_PROTOCOL_VERSION_REQUEST:
            cdc_response_len = mb_bridge_protocol_version_response_setup(cdc_response, sizeof(cdc_response), 0,
                                                                         MB_PROTOCOL_VERSION);
            break;
        case MB_OPERATION_BRIDGE_HARDWARE_INFO_REQUEST:
            hardware_info[0] = '\0';
            strcpy(hardware_info, "Pico ");
            pico_get_unique_board_id_string(&hardware_info[strlen(hardware_info)],  2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);
            cdc_response_len = mb_bridge_hardware_info_response_setup(cdc_response, sizeof(cdc_response), 0, hardware_info);
            break;
        case MB_OPERATION_BRIDGE_FIRMWARE_VERSION_REQUEST:
            cdc_response_len = mb_bridge_firmware_version_response_setup(cdc_response, sizeof(cdc_response), 0,
                                                                         FIRMWARE_VERSION);
            break;
        case MB_OPERATION_BRIDGE_SUPPORTED_COMPONENTS_REQUEST:
            cdc_response_len = mb_bridge_supported_components_response_setup(cdc_response, sizeof(cdc_response), 0,
                                                                             sizeof(supported_components),
                                                                             supported_components);
            break;
        case MB_OPERATION_BRIDGE_DELAY_REQUEST:
            printf("Bridge: Delay %" PRIu32 " ms\n", mb_bridge_delay_request_get_timeout_ms(&cdc_request[MB_HEADER_SIZE]));
            sleep_ms(mb_bridge_delay_request_get_timeout_ms(&cdc_request[MB_HEADER_SIZE]));
            cdc_response_len = mb_bridge_delay_response_setup(cdc_response, sizeof(cdc_response), 0);
            break;
        default:
            printf("Bridge operation 0x%02x not implemented yet, ignore\n", mb_header_get_operation(cdc_request));
            return false;
    }
    cdc_protocol_state = CDC_SEND_RESPONSE;
    return true;
}

//--------------------------------------------------------------------+
// MultiBus Component I2C_Master
//--------------------------------------------------------------------+

#define I2C_MASTER_MAX_READ_LEN 500
#if (I2C_MASTER_MAX_READ_LEN + 10) > MAX_MESSSAGE_LEN
#error "I2C_MASTER_MAX_READ_LEN too larger for MAX_MESSSAGE_LEN"
#endif

static bool mb_i2c_master_configured;
static uint8_t i2c_master_read_buffer[I2C_MASTER_MAX_READ_LEN];

static bool mb_component_i2c_master_handle_request(const uint8_t * payload_data, uint16_t payload_len) {
    uint32_t mb_i2c_master_speed;
    mb_status_t status = MB_STATUS_OK;
    uint16_t i2c_address;
    uint16_t i2c_operation_len;
    int res;
    switch (mb_header_get_operation(cdc_request)) {
        case MB_OPERATION_I2C_MASTER_CONFIG_REQUEST:
            // check parameters
            switch (mb_i2c_master_config_request_get_clock_speed(payload_data)){
                case MB_I2C_MASTER_CONFIG_REQUEST_CLOCK_SPEED_100_KHZ:
                    mb_i2c_master_speed = 100000u;
                    break;
                case MB_I2C_MASTER_CONFIG_REQUEST_CLOCK_SPEED_400_KHZ:
                    mb_i2c_master_speed = 400000u;
                    break;
                default:
                    status = MB_STATUS_INVALID_ARGUMENTS;
                    break;
            }
            if (status == MB_STATUS_OK){
                // unregister
                if (mb_i2c_master_configured){
                    i2c_deinit(i2c_default);
                }
                // config
                i2c_init(i2c_default, mb_i2c_master_speed);
                gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
                gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
                if (mb_i2c_master_config_request_get_enable_sda_pullup(payload_data)){
                    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
                } else {
                    gpio_disable_pulls(PICO_DEFAULT_I2C_SDA_PIN);
                }
                if (mb_i2c_master_config_request_get_enable_scl_pullup(payload_data)){
                    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
                } else {
                    gpio_disable_pulls(PICO_DEFAULT_I2C_SCL_PIN);
                }
                printf("I2C Master Config: speed %u, SDA: GPIO %u, SCL: GPIO %u\n", mb_i2c_master_speed, PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN);
            }
            cdc_response_len = mb_i2c_master_config_response_setup(cdc_response, sizeof(cdc_response), 0, status);
            break;
        case MB_OPERATION_I2C_MASTER_READ_REQUEST:
            i2c_address = mb_i2c_master_read_request_get_address(payload_data);
            i2c_operation_len = mb_i2c_master_read_request_get_num_bytes(payload_data);
            // check size
            if (i2c_operation_len > I2C_MASTER_MAX_READ_LEN){
                status = MB_STATUS_INVALID_ARGUMENTS;
            }
            if (status == MB_STATUS_OK){
                res = i2c_read_blocking(i2c_default, i2c_address, i2c_master_read_buffer, i2c_operation_len, false);
                if (res != i2c_operation_len){
                    status = MB_STATUS_I2C_MASTER_SLAVE_NOT_CONNECTED;
                }
            }
            cdc_response_len = mb_i2c_master_read_response_setup(cdc_response, sizeof(cdc_response), 0, status, i2c_address, i2c_operation_len, i2c_master_read_buffer);
            break;
        case MB_OPERATION_I2C_MASTER_WRITE_REQUEST:
            i2c_address = mb_i2c_master_write_request_get_address(payload_data);
            i2c_operation_len = mb_i2c_master_write_request_get_data_len(payload_len);
            res = i2c_write_blocking(i2c_default, i2c_address, mb_i2c_master_write_request_get_data(payload_data), i2c_operation_len, false);
            if (res != i2c_operation_len){
                status = MB_STATUS_I2C_MASTER_SLAVE_NOT_CONNECTED;
            }
            cdc_response_len = mb_i2c_master_write_response_setup(cdc_response, sizeof(cdc_response), 0, status, i2c_address);
            break;
        default:
            printf("I2C Master operation 0x%02x not implemented yet, ignore\n", mb_header_get_operation(cdc_request));
            return false;
    }
    cdc_protocol_state = CDC_SEND_RESPONSE;
    return true;
}


//--------------------------------------------------------------------+
// MultiBus Component SPI_Master
//--------------------------------------------------------------------+

#define SPI_MASTER_MAX_READ_LEN 500
#if (SPI_MASTER_MAX_READ_LEN + 10) > MAX_MESSSAGE_LEN
#error "SPI_MASTER_MAX_READ_LEN too larger for MAX_MESSSAGE_LEN"
#endif

static bool mb_spi_master_configured;
static uint8_t spi_master_read_buffer[SPI_MASTER_MAX_READ_LEN];

#ifdef PICO_DEFAULT_SPI_CSN_PIN
static inline void mb_spi_master_cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void mb_spi_master_cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}
#endif

static void mb_spi_master_set_gpio_function(enum gpio_function function) {
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, function);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, function);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, function);
}

static bool mb_component_spi_master_handle_request(const uint8_t * payload_data, uint16_t payload_len) {
    mb_status_t status = MB_STATUS_OK;
    uint16_t spi_operation_len;
    // config params
    uint32_t mb_spi_master_speed;
    uint8_t data_bits;
    spi_order_t bit_order;
    spi_cpol_t cpol;
    spi_cpha_t cpha;

    int res;
    switch (mb_header_get_operation(cdc_request)) {
        case MB_OPERATION_SPI_MASTER_CONFIG_REQUEST:
            // check parameters
            switch (mb_spi_master_config_request_get_bit_order(payload_data)){
                case MB_SPI_MASTER_CONFIG_REQUEST_BIT_ORDER_LSB_FIRST:
                    bit_order = SPI_LSB_FIRST;
                    break;
                case MB_I2C_MASTER_CONFIG_REQUEST_CLOCK_SPEED_400_KHZ:
                    bit_order = SPI_MSB_FIRST;
                    break;
                default:
                    status = MB_STATUS_INVALID_ARGUMENTS;
                    break;
            }
            switch(mb_spi_master_config_request_get_cpol(payload_data)){
                case MB_SPI_MASTER_CONFIG_REQUEST_CPOL_0:
                    cpol = SPI_CPOL_0;
                    break;
                case MB_SPI_MASTER_CONFIG_REQUEST_CPOL_1:
                    cpol = SPI_CPOL_1;
                    break;
                default:
                    status = MB_STATUS_INVALID_ARGUMENTS;
                    break;
            }
            switch(mb_spi_master_config_request_get_cpha(payload_data)){
                case MB_SPI_MASTER_CONFIG_REQUEST_CPHA_0:
                    cpol = SPI_CPHA_0;
                    break;
                case MB_SPI_MASTER_CONFIG_REQUEST_CPHA_1:
                    cpol = SPI_CPHA_1;
                    break;
                default:
                    status = MB_STATUS_INVALID_ARGUMENTS;
                    break;
            }
            mb_spi_master_speed = mb_spi_master_config_request_get_baud_rate(payload_data);
            data_bits = mb_spi_master_config_request_get_data_bits(payload_data);
            if ((data_bits > 16) || (data_bits < 4)){
                status = MB_STATUS_INVALID_ARGUMENTS;
            }
            if (status == MB_STATUS_OK){
                // unregister
                if (mb_spi_master_configured){
                    spi_deinit(spi_default);
                }
                // config
                spi_init(spi_default, mb_spi_master_speed);
                spi_set_format(spi_default, data_bits, cpol, cpha, bit_order);
                mb_spi_master_set_gpio_function(GPIO_FUNC_SPI);
                // manually handle chip select
                gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
                gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
                gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
                printf("SPI Master Config: speed %u, data bits: %u, bit order %s first, CPOL %u, CPHA %u\n",
                       mb_spi_master_speed, data_bits, bit_order == SPI_MSB_FIRST ? "MSB" : "LSB", cpol, cpha);
            }
            cdc_response_len = mb_spi_master_config_response_setup(cdc_response, sizeof(cdc_response), 0, status);
            break;
        case MB_OPERATION_SPI_MASTER_READ_REQUEST:
            spi_operation_len = mb_spi_master_read_request_get_num_bytes(payload_data);
            // check size
            if (spi_operation_len > SPI_MASTER_MAX_READ_LEN){
                status = MB_STATUS_INVALID_ARGUMENTS;
            }
            mb_spi_master_cs_select();
            (void) spi_read_blocking(spi_default, 0x00, spi_master_read_buffer, spi_operation_len);
            mb_spi_master_cs_deselect();
            cdc_response_len = mb_spi_master_read_response_setup(cdc_response, sizeof(cdc_response), 0, status, spi_operation_len, spi_master_read_buffer);
            break;
        case MB_OPERATION_I2C_MASTER_WRITE_REQUEST:
            spi_operation_len = mb_spi_master_write_request_get_data_len(payload_len);
            mb_spi_master_cs_select();
            (void) spi_write_blocking(spi_default, mb_spi_master_write_request_get_data(payload_data), spi_operation_len);
            mb_spi_master_cs_deselect();
            cdc_response_len = mb_spi_master_write_response_setup(cdc_response, sizeof(cdc_response), 0, status);
            break;
        case MB_OPERATION_SPI_MASTER_TRANSFER_REQUEST:
            spi_operation_len = mb_spi_master_read_request_get_num_bytes(payload_data);
            // check size
            if (spi_operation_len > SPI_MASTER_MAX_READ_LEN){
                status = MB_STATUS_INVALID_ARGUMENTS;
            }
            mb_spi_master_cs_select();
            spi_write_read_blocking(spi_default, mb_spi_master_transfer_request_get_data(payload_data),
                                    spi_master_read_buffer,spi_operation_len);
            mb_spi_master_cs_deselect();
            cdc_response_len = mb_spi_master_write_response_setup(cdc_response, sizeof(cdc_response), 0, status);
            break;
        default:
            printf("I2C Master operation 0x%02x not implemented yet, ignore\n", mb_header_get_operation(cdc_request));
            return false;
    }
    cdc_protocol_state = CDC_SEND_RESPONSE;
    return true;
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void cdc_reset_rx_state(void){
    cdc_protocol_state = CDC_W4_HEADER;
    cdc_bytes_to_read = MB_HEADER_SIZE;
    cdc_request_len = 0;
}

static void cdc_read(void) {
    if (tud_cdc_n_available(cdc_itf)) {
        uint32_t count = tud_cdc_n_read(cdc_itf, &cdc_request[cdc_request_len], cdc_bytes_to_read);
        cdc_request_len   += count;
        cdc_bytes_to_read -= count;
    }
}

static void cdc_task(void) {
    uint16_t payload_len;
    const uint8_t * payload_data;
    bool valid_request;
    switch (cdc_protocol_state) {
        case CDC_W4_HEADER:
            cdc_read();
            if (cdc_bytes_to_read == 0) {
                cdc_bytes_to_read = mb_header_get_length(cdc_request);
                cdc_protocol_state = CDC_W4_PAYLOAD;
            }
            break;
        case CDC_W4_PAYLOAD:
            cdc_read();
            if (cdc_bytes_to_read == 0){
                cdc_protocol_state = CDC_PROCESS_REQUEST;
            }
            break;
        case CDC_PROCESS_REQUEST:
            printf("Request:  ");
            printf_hexdump(cdc_request, cdc_request_len);
            payload_data = &cdc_request[MB_HEADER_SIZE];
            payload_len = cdc_request_len - MB_HEADER_SIZE;
            cdc_response_offset = 0;
            switch (mb_header_get_component(cdc_request)) {
                case MB_COMPONENT_BRIDGE:
                    valid_request = mb_component_bridge_handle_request(payload_data, payload_len);
                    break;
                case MB_COMPONENT_I2C_MASTER:
                    valid_request = mb_component_i2c_master_handle_request(payload_data, payload_len);
                    break;
                case MB_COMPONENT_SPI_MASTER:
                    valid_request = mb_component_spi_master_handle_request(payload_data, payload_len);
                    break;
                default:
                    printf("Request for unknown component 0x%02x, ignore\n", mb_header_get_component(cdc_request));
                    valid_request = false;
                    break;
            }
            if (valid_request == false){
                cdc_reset_rx_state();
                break;
            }
            if (cdc_protocol_state == CDC_SEND_RESPONSE) {
                printf("Response: ");
                printf_hexdump(cdc_response, cdc_response_len);
            }
            break;
        case CDC_SEND_RESPONSE:
            if (cdc_response_len - cdc_response_offset > 0) {
                uint32_t write_available = tud_cdc_n_write_available(cdc_itf);
                uint32_t bytes_to_write = mb_min(write_available, cdc_response_len - cdc_response_offset);
                tud_cdc_n_write(cdc_itf, &cdc_response[cdc_response_offset], bytes_to_write);
                tud_cdc_n_write_flush(cdc_itf);
                cdc_response_offset += bytes_to_write;
            } else {
                cdc_reset_rx_state();
            }
            break;
        default:
            break;
    }
}

/*------------- MAIN -------------*/
int main(void) {
    board_init();

    pico_get_unique_board_id_string(usb_serial,  2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

    tud_init(0);

    cdc_reset_rx_state();

    printf("MultiBus Bridge started, %s\n", usb_serial);

    while (1) {
        tud_task(); // tinyusb device task
        cdc_task();
    }
    return 0;
}
