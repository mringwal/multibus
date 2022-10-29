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
 * MultiBus Driver Abstraction
 * Driver allows to send / receive a block of data over some transport interface
 */

#ifndef C_TEST_MULTIBUS_DRIVER_H
#define C_TEST_MULTIBUS_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#if defined __cplusplus
extern "C" {
#endif

typedef struct {

    /**
     * set callback for block received. NULL disables callback
     * @param driver_context
     * @param callback_handler
     * @param callback_context
     */
    void (*set_block_received)(void * driver_context, void (*callback_handler)(void *context), void * callback_context);

    /**
     * set callback for sent. NULL disables callback
     * @param driver_context
     * @param callback_handler
     * @param callback_context
     */
    void (*set_block_sent)(void * driver_context, void (*callback_handler)(void *context), void * callback_context);

    /**
     * receive block
     * @param driver_context
     * @param buffer
     * @param length
     */
    void (*receive_block)(void * driver_context, uint8_t *buffer, uint16_t length);

    /**
     * send block
     * @param driver_context
     * @param buffer
     * @param length
     */
    void (*send_block)(void * driver_context, const uint8_t *buffer, uint16_t length);

} mb_driver_t;

#if defined __cplusplus
}
#endif

#endif //C_TEST_MULTIBUS_DRIVER_H
