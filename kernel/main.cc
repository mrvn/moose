/* main.cc - the entry point for the kernel */
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
#include <atags.h>
#include <memory.h>
#include <gpio.h>

extern "C" {
    // kernel_main gets called from boot.S. Declaring it extern "C" avoid
    // having to deal with the C++ name mangling.
    void kernel_main(uint32_t zero, uint32_t model, const ATAG::Header *atags);

    // constructors
    typedef void (*constructor_t)(void);
    extern constructor_t _init_array_start[];
    extern constructor_t _init_array_end[];
    void kernel_constructors(void);
}

#define UNUSED(x) (void)(x)

void kernel_constructors(void) {
    for(constructor_t *fn = _init_array_start; fn != _init_array_end; ++fn) {
	(*fn)();
    }
}

// kernel main function, it all begins here
void kernel_main(uint32_t zero, uint32_t model, const ATAG::Header *atags) {
    UNUSED(zero);
    UNUSED(model);
    Memory::init(atags);
    GPIO::init();
    
    // blink 10 times 
    for(int i = 0; i < 20; ++i) {
	GPIO::led();
	for(volatile int j = 0; j < 0x3000000; ++j) { }
    }
}

