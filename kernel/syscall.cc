/* syscall.cc - Syscall interface */
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

#include <panic.h>
#include <stdint.h>
#include <uart.h>
#include <memory.h>
#include <entry.h>

#include <syscall.h>

namespace Syscall {
    extern "C" {
        // called from entry.S. Declaring it extern "C" avoid
        // having to deal with the C++ name mangling.
        uint32_t exception_syscall_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t opcode);
    }

    void init() {
	// allocate stack for SVC mode
        uint32_t *stack = (uint32_t*)Memory::early_malloc(0); // 4k stack
        set_mode_stack(MODE_SVC, &stack[1024]);
    }

    uint32_t exception_syscall_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t opcode) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	UART::puts("R0 = ");
	UART::put_uint32(r0);
	UART::puts(", R1 = ");
	UART::put_uint32(r1);
	UART::puts(", R2 = ");
	UART::put_uint32(r2);
	UART::puts(", opcode = ");
	UART::put_uint32(opcode);
	UART::putc('\n');
        return ~r0;
    }
}
