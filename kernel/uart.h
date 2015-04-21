/* Copyright (C) 2015 Goswin von Brederlow <goswin-v-b@web.de>

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

/* Uart related functions
 */

#ifndef KERNEL_UART_H
#define KERNEL_UART_H

#include <stdint.h>
#include <sys/cdefs.h>
#include "peripherals.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(UART);

template<Peripheral::Base = Peripheral::NONE>
void putc(const char c) {
    PERIPHERAL(UART0_BASE);
    putc<BASE>(c);
}

template<>
void putc<Peripheral::UART0_BASE>(const char c);

template<Peripheral::Base = Peripheral::NONE>
char getc() {
    PERIPHERAL(UART0_BASE);
    return getc<BASE>();
}

template<>
char getc<Peripheral::UART0_BASE>();

template<Peripheral::Base = Peripheral::NONE>
void puts(const char *str) {
    PERIPHERAL(UART0_BASE);
    puts<BASE>(str);
}

template<>
void puts<Peripheral::UART0_BASE>(const char *str);

__END_NAMESPACE(UART)
__END_NAMESPACE(Kernel)

#endif // ##ifndef KERNEL_UART_H
