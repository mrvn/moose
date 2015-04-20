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

void test(Peripheral::Barrier<Peripheral::TIMER_BASE> barrier
	  = Peripheral::Barrier<Peripheral::NONE>());

uint64_t count(Peripheral::Barrier<Peripheral::TIMER_BASE> barrier
	       = Peripheral::Barrier<Peripheral::NONE>());

void handle_timer1(Peripheral::Barrier<Peripheral::TIMER_BASE> barrier
		   = Peripheral::Barrier<Peripheral::NONE>());

/* busily wait at least usec micro seconds
 * busy_wait(0) will wait till the next timer tick
 */
void busy_wait(uint32_t usec,
	       Peripheral::Barrier<Peripheral::TIMER_BASE> barrier
	       = Peripheral::Barrier<Peripheral::NONE>());

__END_NAMESPACE(Timer);
__END_NAMESPACE(Kernel);

#endif // ##ifndef KERNEL_TIMER_H
