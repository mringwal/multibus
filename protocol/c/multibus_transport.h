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

#ifndef C_TEST_MULTIBUS_TRANSPORT_H
#define C_TEST_MULTIBUS_TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>

#include "multibus_driver.h"
#include "multibus_protocol.h"

#if defined __cplusplus
extern "C" {
#endif

typedef enum {
    MB_TRANSPORT_RX_IDLE,
    MB_TRANSPORT_RX_W4_HEADER,
    MB_TRANSPORT_WX_W4_PAYLOAD
} mb_transport_rx_state_t;

typedef enum {
    MB_TRANSPORT_TX_BUSY,
    MB_TRANSPORT_TX_IDLE,
} mb_transport_tx_state_t;

typedef struct {
    // driver implementation and context
    const mb_driver_t *driver_impl;
    void * driver_context;

    // callback with context
    void (*callback_handler)(void * context, const mb_message_t * message);
    void * callback_context;

    // send buffer
    uint8_t  * send_buffer_storage;
    uint16_t   send_buffer_size;

    // receive buffer
    uint8_t    receive_header[MB_HEADER_SIZE];
    uint8_t  * receive_buffer_storage;
    uint16_t   receive_buffer_size;

    // state
    mb_transport_rx_state_t rx_state;
    mb_transport_tx_state_t tx_state;

    // logging
    bool dump_messages;
} mb_transport_t;

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
                         uint8_t * receive_buffer_storage, uint16_t receive_buffer_size);

/**
 * Enable Logging of messages
 * @param transport
 */
void mb_transport_enable_logging(mb_transport_t * transport);

// Asynchronous Interface

/**
 * @brief Register callback
 * @param context for transport instance
 * @param callback_handler
 * @param callback_context
 */
void mb_transport_register_callback(mb_transport_t * transport,
                                    void (*callback_handler)(void * context, const mb_message_t * message),
                                    void * callback_context);

/**
 * @brief Write request
 * @note When complete, callback with component = host, opcode = write_complete will be emitted
 * @param context for transport instance
 * @param buffer
 * @param size
 * @return ok
 */
bool  mb_transport_send(mb_transport_t * transport, const uint8_t * buffer, uint16_t size);

#if defined __cplusplus
}
#endif

#endif //C_TEST_MULTIBUS_TRANSPORT_H
