/* uart.h - serial console driver */
/* Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* Reference material:
 * http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 * Chapter 13: UART
 */

#ifndef MOOSE_KERNEL_UART_H
#define MOOSE_KERNEL_UART_H

#include <stdint.h>

namespace UART {
    void init();
    
    /*
     * Transmit a byte via UART0.
     * byte: byte to send.
     */
    void putc(uint8_t byte);

    /*
     * Receive a byte via UART0.
     * returns: byte received.
     */
    uint8_t getc(void);

    /*
     * Check if data is available via UART0.
     * returns: data available?
     */
    bool poll(void);

    /*
     * print a string to the UART one character at a time
     * str: 0-terminated string
     */
    void puts(const char *str);

    /*
     * print an uint32_t as hex to the UART one character at a time
     * data: uint32_t to print
     */
    void put_uint32(uint32_t data);
};

#endif // #ifndef MOOSE_KERNEL_UART_H
