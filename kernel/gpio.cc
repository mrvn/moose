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

#include <stdint.h>
#include <memory.h>

#include <gpio.h>

namespace GPIO {
    enum {
	// base address relative to peripherals region
	BASE = 0x200000,

	// registers
	GPFSEL0   = 0x00,
	GPFSEL1   = 0x04,
	GPFSEL2   = 0x08,
	GPFSEL3   = 0x0C,
	GPFSEL4   = 0x10,
	GPFSEL5   = 0x14,
	GPSET0    = 0x1C,
	GPSET1    = 0x20,
	GPCLR0    = 0x28,
	GPCLR1    = 0x2C,
	GPLEV0    = 0x34,
	GPLEV1    = 0x38,
	GPEDS0    = 0x40,
	GPEDS1    = 0x44,
	GPREN0    = 0x4C,
	GPREN1    = 0x50,
	GPFEN0    = 0x58,
	GPFEN1    = 0x5C,
	GPHEN0    = 0x64,
	GPHEN1    = 0x68,
	GPLEN0    = 0x70,
	GPLEN1    = 0x74,
	GPAREN0   = 0x7C,
	GPAREN1   = 0x80,
	GPAFEN0   = 0x88,
	GPAFEN1   = 0x8C,
	GPPUD     = 0x94,
	GPPUDCLK0 = 0x98,
	GPPUDCLK1 = 0xA0,
	GPTEST    = 0xB0,
	SIZE      = 0xB4,
	
	NUM_PINS = 54,
	PIN_LED  = 16,
    };
    
    uint64_t allocated = 1 << PIN_LED;
    Memory::Peripheral gpio;

    /*
     * delay function
     * int32_t delay: number of cycles to delay
     *
     * This just loops <delay> times in a way that the compiler
     * wont optimize away.
     */
    static inline void delay(int32_t count) {
        asm volatile("1: subs %[count], %[count], #1; bne 1b"
                     : : [count]"r"(count));
    }

    bool led_state;

    void init() {
	// allocate peripheral region
	gpio = Memory::alloc_peripheral(BASE, SIZE);

	// reset registers
	// no event detection
	gpio.set(GPEDS0, 0);
	gpio.set(GPEDS1, 0);
	gpio.set(GPREN0, 0);
	gpio.set(GPREN1, 0);
	gpio.set(GPFEN0, 0);
	gpio.set(GPFEN1, 0);
	gpio.set(GPHEN0, 0);
	gpio.set(GPHEN1, 0);
	gpio.set(GPLEN0, 0);
	gpio.set(GPLEN1, 0);
	gpio.set(GPAREN0, 0);
	gpio.set(GPAREN1, 0);
	gpio.set(GPAFEN0, 0);
	gpio.set(GPAFEN1, 0);
	// all pins input (except 16 (OK LED) is output)
	gpio.set(GPFSEL0, 0);
	gpio.set(GPFSEL1, 0x00040000);
	gpio.set(GPFSEL2, 0);
	gpio.set(GPFSEL3, 0);
	gpio.set(GPFSEL4, 0);
	gpio.set(GPFSEL5, 0);
	// clear output
	gpio.set(GPCLR0, 0xFFFFFFFF);
	gpio.set(GPCLR1, 0x003FFFFF);
	// turn off pull down and pull up
	gpio.set(GPPUD, 0);
	delay(150);
	gpio.set(GPPUDCLK0, 0xFFFFFFFF);
	gpio.set(GPPUDCLK1, 0x003FFFFF);
	delay(150);
	gpio.set(GPPUD, 0);
	gpio.set(GPPUDCLK0, 0);
	gpio.set(GPPUDCLK1, 0);

	led_state = false;
    }

    void set64(uint32_t reg0, uint32_t reg1, int num) {
	if (num < 32) {
	    gpio.set(reg0, 1 << num);
	} else {
	    gpio.set(reg1, 1 << (num - 32));
	}
    }

    /* allocate pins and configure them
     * pins:     array of pin configs
     * num_pins: size of array
     * returns:  true if pins are available
     */
    bool alloc(Pin *pins, int num_pins) {
	uint32_t mask = 0;
	for(int i = 0; i < num_pins; ++i) {
	    if (pins[i].num >= NUM_PINS) {
		// not enough pins for request
		return false;
	    }
	    mask |= 1 << pins[i].num;
	}
	if ((allocated & mask) != 0) {
	    // pins already in use
	    return false;
	}
	allocated |= mask;
	for(int i = 0; i < num_pins; ++i) {
	    // set output
	    if (pins[i].output) {
		set64(GPSET0, GPSET1, pins[i].num);
	    } else {
		set64(GPCLR0, GPCLR1, pins[i].num);
	    }
	    // set pull up/down
	    gpio.set(GPPUD, pins[i].pull);
	    delay(150);
	    set64(GPPUDCLK0, GPPUDCLK1, pins[i].num);
	    delay(150);
	    gpio.set(GPPUD, 0);
	    gpio.set(GPPUDCLK0, 0);
	    gpio.set(GPPUDCLK1, 0);
	    // select function
	    int reg = pins[i].num / 9;
	    int shift = pins[i].num % 9;
	    uint32_t t = gpio.get(GPFSEL0 + reg);
	    t &= ~(7 << shift);
	    t |= pins[i].function << shift;
	    gpio.set(GPFSEL0 + reg, t);
	}
	return true;
    }

    // trun on/off the OK LED on the RPi
    void led(bool on) {
	led_state = on;
	if (on) {
	    gpio.set(GPCLR0, 1 << PIN_LED);
	} else {
	    gpio.set(GPSET0, 1 << PIN_LED);
	}
    }
};
