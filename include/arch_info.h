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

/* Information about the different Raspberry Pi models
 */

#ifndef KERNEL_ARCHINFO_H
#define KERNEL_ARCHINFO_H 1

#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_NAMESPACE(Kernel);
__BEGIN_DECLS;
enum Model {
    RASPBERRY_PI,
    RASPBERRY_PI_B_PLUS,
    RASPBERRY_PI_2,
};

extern enum Model model;
extern const char *model_name;
extern uint32_t mem_total;
extern uint32_t initrd_start;
extern uint32_t initrd_size;
extern uint32_t peripheral_base;
extern uint32_t led_act_pin;
extern uint32_t led_pwr_pin;

enum {
    NO_LED = ~0U, // pin value when LED does not exist
};

typedef struct Atag Atag;

EXPORT extern Atag *atags; // in start.S

__END_DECLS;
__END_NAMESPACE(Kernel);

#endif // ##ifndef KERNEL_ARCHINFO_H
