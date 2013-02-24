/* gpio.h - General Purpose I/O pins */
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

#ifndef MOOSE_KERNEL_GPIO_H
#define MOOSE_KERNEL_GPIO_H

namespace GPIO {
    void init();

    // Pin allocation
    enum Function { INPUT, OUTPUT, ALT5, ALT4, ALT0, ALT1, ALT2, ALT3 };
    enum Pull { NONE, DOWN, UP };
    struct Pin {
	unsigned int num;
	Function function;
	bool output;
	Pull pull;
    };
    /* allocate pins and configure them
     * pins:     array of pin configs
     * num_pins: size of array
     * returns:  true if pins are available
     */
    bool alloc(Pin *pins, int num_pins);

    // trun on/off the OK LED on the RPi
    extern bool led_state;
    void led(bool on = !led_state);
};

#endif // #ifndef MOOSE_KERNEL_GPIO_H
