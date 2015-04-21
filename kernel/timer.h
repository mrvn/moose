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

/* System Timer
 */

#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H 1

#include <stdint.h>
#include <sys/cdefs.h>
#include "peripherals.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(Timer);

template<Peripheral::Base base = Peripheral::NONE>
void test(void) {
    PERIPHERAL(TIMER_BASE);
    test<BASE>();
}

template<>
void test<Peripheral::TIMER_BASE>(void);

template<Peripheral::Base base = Peripheral::NONE>
uint64_t count() {
    PERIPHERAL(TIMER_BASE);
    return count<BASE>();
}

template<>
uint64_t count<Peripheral::TIMER_BASE>();

template<Peripheral::Base base = Peripheral::NONE>
void handle_timer1() {
    PERIPHERAL(TIMER_BASE);
    handle_timer1<BASE>();
}

template<>
void handle_timer1<Peripheral::TIMER_BASE>();

/* busily wait at least usec micro seconds
 * busy_wait(0) will wait till the next timer tick
 */
template<Peripheral::Base base = Peripheral::NONE>
void busy_wait(uint32_t usec) {
    PERIPHERAL(TIMER_BASE);
    busy_wait<BASE>(usec);
}

template<>
void busy_wait<Peripheral::TIMER_BASE>(uint32_t usec);

__END_NAMESPACE(Timer);
__END_NAMESPACE(Kernel);

#endif // ##ifndef KERNEL_TIMER_H
