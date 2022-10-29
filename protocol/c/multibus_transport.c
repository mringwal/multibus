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

/**
 * MultiBus Transport Abstraction
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "multibus_transport.h"

char char_for_nibble(int nibble){
    static const char * char_to_nibble = "0123456789ABCDEF";
    if (nibble < 16){
        return char_to_nibble[nibble];
    } else {
        return '?';
    }
}

static inline char char_for_high_nibble(int value){
    return char_for_nibble((value >> 4) & 0x0f);
}

static inline char char_for_low_nibble(int value){
    return char_for_nibble(value & 0x0f);
}

int nibble_for_char(char c){
    if ((c >= '0') && (c <= '9')) return c - '0';
    if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
    if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
    return -1;
}

void printf_hexdump(const void * data, int size){
    char buffer[4];
    buffer[2] = ' ';
    buffer[3] =  0;
    const uint8_t * ptr = (const uint8_t *) data;
    while (size > 0){
        uint8_t byte = *ptr++;
        buffer[0] = char_for_high_nibble(byte);
        buffer[1] = char_for_low_nibble(byte);
        printf("%s", buffer);
        size--;
    }
    printf("\n");
}

static inline void mb_transport_block_sent(void * context){
    mb_transport_t * transport = (mb_transport_t *) context;
    assert(transport->tx_state == MB_TRANSPORT_TX_BUSY);
    transport->tx_state = MB_TRANSPORT_TX_IDLE;
}

static void mb_transport_start_reading(mb_transport_t * transport){
    transport->rx_state = MB_TRANSPORT_RX_W4_HEADER;
    transport->driver_impl->receive_block(transport->driver_context, transport->receive_header, MB_HEADER_SIZE);
}

static void mb_transport_message_received(mb_transport_t * transport){
    assert(transport->callback_handler != NULL);
    mb_message_t message;
    message.channel      = mb_header_get_channel(transport->receive_header);
    message.component    = mb_header_get_component(transport->receive_header);
    message.operation    = mb_header_get_operation(transport->receive_header);
    message.payload_len  = mb_header_get_length(transport->receive_header);
    message.payload_data = transport->receive_buffer_storage;
    if (transport->dump_messages ){
        printf("Serial-Response:\n");
        printf("- Header: ");
        printf_hexdump(transport->receive_header, MB_HEADER_SIZE);
        printf("- Payload: ");
        printf_hexdump(transport->receive_buffer_storage, message.payload_len);
    }
    transport->callback_handler(transport->callback_context, &message);
}

static inline void mb_transport_block_received(void * context){
    mb_transport_t * transport = (mb_transport_t *) context;
    uint16_t payload_len;
    switch(transport->rx_state){
        case MB_TRANSPORT_RX_IDLE:
            break;
        case MB_TRANSPORT_RX_W4_HEADER:
            payload_len = mb_header_get_length(transport->receive_header);
            if (payload_len == 0){
                mb_transport_message_received(transport);
                mb_transport_start_reading(transport);
            } else {
                transport->rx_state = MB_TRANSPORT_WX_W4_PAYLOAD;
                transport->driver_impl->receive_block(transport->driver_context,
                                                      transport->receive_buffer_storage,
                                                      payload_len);
            }
            break;
        case MB_TRANSPORT_WX_W4_PAYLOAD:
            mb_transport_message_received(transport);
            mb_transport_start_reading(transport);
            break;
        default:
            assert(false);
            break;
    }
}

/**
 * Setup transport instance with driver and buffers
 * @param transport
 * @param driver_impl
 * @param driver_context
 * @param send_buffer_storage
 * @param send_buffer_size
 * @param receive_buffer_storage
 * @param receive_buffer_size
 */
void mb_transport_create(mb_transport_t * transport, const mb_driver_t * driver_impl, void * driver_context,
                         uint8_t * send_buffer_storage, uint16_t send_buffer_size,
                         uint8_t * receive_buffer_storage, uint16_t receive_buffer_size){
    assert(transport != NULL);

    // setup transport instance
    transport->driver_impl            = driver_impl;
    transport->driver_context         = driver_context;
    transport->send_buffer_size       = send_buffer_size;
    transport->send_buffer_storage    = send_buffer_storage;
    transport->receive_buffer_size    = receive_buffer_size;
    transport->receive_buffer_storage = receive_buffer_storage;

    // state
    transport->tx_state = MB_TRANSPORT_TX_IDLE;
    transport->rx_state = MB_TRANSPORT_RX_IDLE;

    transport->dump_messages = false;

    // register with driver
    driver_impl->set_block_sent(driver_context, &mb_transport_block_sent, transport);
    driver_impl->set_block_received(driver_context, &mb_transport_block_received, transport);
}

void mb_transport_enable_logging(mb_transport_t * transport){
    transport->dump_messages = true;
}

// Async Interface

void mb_transport_register_callback(mb_transport_t * transport,
                                    void (*callback_handler)(void * context, const mb_message_t * message),
                                    void * callback_context){
    assert(transport != NULL);
    transport->callback_handler = callback_handler;
    transport->callback_context = callback_context;
    mb_transport_start_reading(transport);
}

/**
 * @brief Write request
 * @note When complete, callback with component = host, opcode = write_complete will be emitted
 * @param context for transport instance
 * @param buffer
 * @param size
 * @return ok
 */
bool  mb_transport_send(mb_transport_t * transport, const uint8_t * buffer, uint16_t size){
    assert(buffer != NULL);
    assert(size >= MB_HEADER_SIZE);
    assert(transport != NULL);
    assert(transport->tx_state == MB_TRANSPORT_TX_IDLE);
    if (transport->dump_messages ) {
        printf("Serial-Request:\n");
        printf("- Header: ");
        printf_hexdump(buffer, MB_HEADER_SIZE);
        printf("- Payload: ");
        printf_hexdump(&buffer[MB_HEADER_SIZE], size - MB_HEADER_SIZE);
    }
    transport->tx_state = MB_TRANSPORT_TX_BUSY;
    transport->driver_impl->send_block(transport->driver_context, buffer, size);
    return true;
}
