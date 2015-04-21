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

/* Helper for peripherals
 */

#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include <sys/cdefs.h>
#include <stddef.h>
#include "asm.h"
#include "fixed_addresses.h"

__BEGIN_NAMESPACE(Peripheral);

enum Base {
    NONE       = 0,
    GPIO_BASE  = KERNEL_GPIO,
    UART0_BASE = KERNEL_UART,
    IRQ_BASE   = KERNEL_IRQ,
    TIMER_BASE = KERNEL_TIMER,
    CORE_BASE  = KERNEL_CORE_MAIL,
    VC_BASE    = KERNEL_VC_MAIL,
};

/* Barrier protecting peripherals when switching between them
 *
 * A memory barrier is inserted when a Barrier is converted to another base by
 * the source barrier and on destruction of the converted Barrier but not for
 * Barrier<None>. Calling a function with the wrong Barrier type will trigger
 * a implicit conversion befor the call and destruction after the call, thus
 * palcing a memory barrier before and after the function call.
 *
 * Public functions taking a barrier should do so as last parameter with a
 * default = Barrier<NONE>. This will envlose the function with memory
 * barriers when no barrier is given. Private functions should have not
 * default to ensure the barrier of the caller is passed along.
 */
template<Base base>
class Barrier {
public:
    static constexpr const Base BASE = base;

    Barrier() {
	dmb();
    }

    ~Barrier() {
	dmb();
    }

    Barrier(const Barrier &) = delete; // copy constructor
    Barrier(Barrier &&) = delete; // move constructor
    Barrier & operator =(const Barrier &) = delete; // copy assignment
    Barrier & operator =(Barrier &&) = delete; // move assignment
};

#define BASE(B)							\
    static constexpr const Peripheral::Base BASE = Peripheral::B

#define PERIPHERAL(B)							\
    Peripheral::Barrier<Peripheral::B> barrier;				\
    BASE(B)


__END_NAMESPACE(Peripheral);

#endif // ##ifndef PERIPHERALS_H
