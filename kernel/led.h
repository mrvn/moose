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

/* LED related functions
 */

#ifndef KERNEL_LED_H
#define KERNEL_LED_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <stdbool.h>
#include "peripherals.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(LED);

enum LED {
    LED_ACT,
    LED_PWR,
};

template<Peripheral::Base base = Peripheral::NONE>
void set(enum LED led, bool state) {
    PERIPHERAL(GPIO_BASE);
    set<BASE>(led, state);
}

template<>
void set<Peripheral::GPIO_BASE>(enum LED led, bool state);

template<Peripheral::Base base = Peripheral::NONE>
void toggle(enum LED led) {
    PERIPHERAL(GPIO_BASE);
    toggle<BASE>(led);
}

template<>
void toggle<Peripheral::GPIO_BASE>(enum LED led);

__END_NAMESPACE(LED);
__END_NAMESPACE(Kernel);

#endif // ##ifndef KERNEL_LED_H
