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

#ifndef C_TEST_MULTIBUS_SERIAL_POSIX_H
#define C_TEST_MULTIBUS_SERIAL_POSIX_H

#include <stdint.h>
#include <stdbool.h>
#include <termios.h>  /* POSIX terminal control definitions */

#include "multibus_transport.h"
#include "multibus_driver.h"

/**
 * MultiBus Serial Transport for POSIX systems
 */

typedef struct {
    // on macOS 12.1, CTS/RTS control flags are always read back as zero.
    // To work around this, we cache our terios settings
    struct termios termios;
    int fd;
    // driver interface
    void (*block_received_callback)(void *context);
    void *block_received_context;
    void (*block_sent_callback)(void *context);
    void *block_sent_context;
    // event loop integration
    void (*write_started_callback)(void * context);
    void *write_started_context;
    const uint8_t * tx_buffer;
    uint16_t  tx_len;
    uint8_t * rx_buffer;
    uint16_t  rx_len;
} mb_serial_posix_context_t;

/**
 * @brief Open serial port as MultiBus transport
 * @param mb_serial_posix_context
 * @param dev_path
 * @param baudrate
 * @return true if successful
 */
bool mb_serial_posix_open(mb_serial_posix_context_t * mb_serial_posix_context, const char *dev_path, uint32_t baudrate);

/**
 * @brief Close serial port
 * @param mb_serial_posix_context
 */
void mb_serial_posix_close(mb_serial_posix_context_t * mb_serial_posix_context);

// Async Interface

/**
 * @brief Get POSIX file descriptor for use with event loop
 * @param mb_serial_posix_context
 * @return file dscriptor of serial device
 */
int mb_serial_posix_get_file_descriptor(mb_serial_posix_context_t * mb_serial_posix_context);

/**
 * @brief Register callback that indicates that data is ready to write
 * @param mb_serial_posix_context
 * @param write_started_callback
 * @param write_started_context
 */
void mb_serial_posix_register_write_started(mb_serial_posix_context_t * mb_serial_posix_context,
                                            void (*write_started_callback)(void * context), void * write_started_context);

/**
 * @brief Query if write operation is active
 * @param mb_serial_posix_context
 * @return true if more data needs to be written
 */
bool mb_serial_posix_write_active(mb_serial_posix_context_t * mb_serial_posix_context);

/**
 * @brief Process incoming data
 * @param mb_serial_posix_context
 */
void  mb_serial_posix_process_read(mb_serial_posix_context_t * mb_serial_posix_context);

/**
 * @brief Continue sending
 * @param mb_serial_posix_context
 */
void mb_serial_posix_process_write(mb_serial_posix_context_t * mb_serial_posix_context);

/**
 * Provide driver implementation
 * @return mb_driver_t implementation
 */
const mb_driver_t * mb_serial_posix_get_driver(void);

#endif //C_TEST_MULTIBUS_SERIAL_POSIX_H
