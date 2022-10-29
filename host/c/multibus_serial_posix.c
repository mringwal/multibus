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
 * MultiBus Serial Transport for POSIX systems
 */

#include <errno.h>
#include <fcntl.h>    /* File control definitions */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>   /* UNIX standard function definitions */

#ifdef __APPLE__
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#endif

#include "multibus_serial_posix.h"
#include "multibus_protocol.h"

bool mb_serial_posix_open(mb_serial_posix_context_t *mb_serial_posix_context, const char *dev_path, uint32_t baudrate) {

    // open serial port
    int flags = O_RDWR | O_NOCTTY | O_NONBLOCK;
    int fd = open(dev_path, flags);
    if (fd == -1)  {
        printf("Unable to open port %s\n", dev_path);
        perror("Error");
        return false;
    }

    if (tcgetattr(fd, &mb_serial_posix_context->termios) < 0) {
        perror("Couldn't get term attributes");
        return false;
    }
    cfmakeraw(&mb_serial_posix_context->termios);   // make raw

    // 8N1
    mb_serial_posix_context->termios.c_cflag &= ~CSTOPB;
    mb_serial_posix_context->termios.c_cflag |= CS8;

    mb_serial_posix_context->termios.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    mb_serial_posix_context->termios.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    // configure blocking read
    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    mb_serial_posix_context->termios.c_cc[VMIN]  = 1;
    mb_serial_posix_context->termios.c_cc[VTIME] = 0;

    if(tcsetattr(fd, TCSANOW, &mb_serial_posix_context->termios) < 0) {
        perror("Couldn't set term attributes");
        return false;
    }

#ifndef __APPLE__

    speed_t brate = baudrate; // let you override switch below if needed
    switch(baudrate) {
        case    9600: brate=B9600;    break;
        case   19200: brate=B19200;   break;
        case   38400: brate=B38400;   break;
        case 57600:  brate=B57600;  break;
        case 115200: brate=B115200; break;
#ifdef B230400
        case 230400: brate=B230400; break;
#endif
#ifdef B460800
        case 460800: brate=B460800; break;
#endif
#ifdef B921600
        case 921600: brate=B921600; break;
#endif
#ifdef B1000000
        case 1000000: brate=B1000000; break;
#endif
        default:
            printf("can't set baudrate %dn", baudrate );
            return -1;
    }
    cfsetospeed(&mb_serial_posix_context->termios, brate);
    cfsetispeed(&mb_serial_posix_context->termios, brate);
#endif

    // also set options for __APPLE__ to enforce write drain
    // Mac OS Mojave: tcsdrain did not work as expected

    if( tcsetattr(fd, TCSADRAIN, &mb_serial_posix_context->termios) < 0) {
        perror("Couldn't set term attributes");
        return false;
    }

#ifdef __APPLE__
    // From https://developer.apple.com/library/content/samplecode/SerialPortSample/Listings/SerialPortSample_SerialPortSample_c.html

    // The IOSSIOSPEED ioctl can be used to set arbitrary baud rates
    // other than those specified by POSIX. The driver for the underlying serial hardware
    // ultimately determines which baud rates can be used. This ioctl sets both the input
    // and output speed.

    speed_t speed = baudrate;
    if (ioctl(fd, IOSSIOSPEED, &speed) == -1) {
        printf("btstack_uart_posix_set_baudrate: error calling ioctl(..., IOSSIOSPEED, %u) - %s(%d).\n", baudrate, strerror(errno), errno);
        return false;
    }
#endif

    // store fd in context
    mb_serial_posix_context->fd = fd;

    // wait a bit - at least cheap FTDI232 clones might send the first byte out incorrectly
    usleep(100000);

    return true;
}

void mb_serial_posix_close(mb_serial_posix_context_t * mb_serial_posix_context) {
    if (mb_serial_posix_context->fd >= 0){
        close (mb_serial_posix_context->fd);
        mb_serial_posix_context->fd = -1;
    }
}

int mb_serial_posix_get_file_descriptor(mb_serial_posix_context_t * mb_serial_posix_context){
    return mb_serial_posix_context->fd;
}

bool mb_serial_posix_write_active(mb_serial_posix_context_t * mb_serial_posix_context){
    return mb_serial_posix_context->tx_len > 0;
}

void mb_serial_posix_driver_set_block_received(void * driver_context, void (*block_handler)(void * context), void * callback_context){
    mb_serial_posix_context_t * mb_serial_posix_context = (mb_serial_posix_context_t *) driver_context;
    mb_serial_posix_context->block_received_callback = block_handler;
    mb_serial_posix_context->block_received_context = callback_context;
}

void mb_serial_posix_driver_set_block_sent(void * driver_context, void (*block_handler)(void * context), void * callback_context){
    mb_serial_posix_context_t * mb_serial_posix_context = (mb_serial_posix_context_t *) driver_context;
    mb_serial_posix_context->block_sent_callback = block_handler;
    mb_serial_posix_context->block_sent_context = callback_context;
}

void mb_serial_posix_register_write_started(mb_serial_posix_context_t * mb_serial_posix_context,
                                            void (*write_started_callback)(void * context), void * write_started_context){
    mb_serial_posix_context->write_started_callback = write_started_callback;
    mb_serial_posix_context->write_started_context  = write_started_context;
}

void mb_serial_posix_driver_receive_block(void * driver_context, uint8_t *buffer, uint16_t length){
    mb_serial_posix_context_t * mb_serial_posix_context = (mb_serial_posix_context_t *) driver_context;
    mb_serial_posix_context->rx_buffer = buffer;
    mb_serial_posix_context->rx_len = length;
}

void mb_serial_posix_driver_send_block(void * driver_context, const uint8_t *buffer, uint16_t length){
    mb_serial_posix_context_t * mb_serial_posix_context = (mb_serial_posix_context_t *) driver_context;
    mb_serial_posix_context->tx_buffer = buffer;
    mb_serial_posix_context->tx_len = length;
    if (mb_serial_posix_context->write_started_callback != NULL){
        (*mb_serial_posix_context->write_started_callback)(mb_serial_posix_context->write_started_context);
    }
}

void mb_serial_posix_process_read(mb_serial_posix_context_t * mb_serial_posix_context) {
    if (mb_serial_posix_context->rx_len  == 0) {
        return;
    }

    // read up to bytes_to_read data in
    ssize_t bytes_read = read(mb_serial_posix_context->fd, mb_serial_posix_context->rx_buffer, mb_serial_posix_context->rx_len);
    if (bytes_read <= 0) {
        return;
    }

    mb_serial_posix_context->rx_len    -= bytes_read;
    mb_serial_posix_context->rx_buffer += bytes_read;
    if (mb_serial_posix_context->rx_len > 0) return;

    if (mb_serial_posix_context->block_received_callback != NULL){
        mb_serial_posix_context->block_received_callback(mb_serial_posix_context->block_received_context);
    }
}

void mb_serial_posix_process_write(mb_serial_posix_context_t * mb_serial_posix_context) {

    if (mb_serial_posix_context->tx_len == 0) return;

    // write up to write_bytes_len to fd
    ssize_t bytes_written = write(mb_serial_posix_context->fd, mb_serial_posix_context->tx_buffer, mb_serial_posix_context->tx_len);
    if (bytes_written <= 0) {
        return;
    }

    mb_serial_posix_context->tx_len    -= bytes_written;
    mb_serial_posix_context->tx_buffer += bytes_written;
    if (mb_serial_posix_context->tx_len > 0) return;

    if (mb_serial_posix_context->block_sent_callback != NULL){
        mb_serial_posix_context->block_sent_callback(mb_serial_posix_context->block_sent_context);
    }
}

static const mb_driver_t mb_serial_posix_driver_interface = {
        .set_block_received = &mb_serial_posix_driver_set_block_received,
        .set_block_sent     = &mb_serial_posix_driver_set_block_sent,
        .receive_block      = &mb_serial_posix_driver_receive_block,
        .send_block         = &mb_serial_posix_driver_send_block
};

const mb_driver_t * mb_serial_posix_get_driver(void){
    return &mb_serial_posix_driver_interface;
}
