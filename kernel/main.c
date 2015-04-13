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

/* Detect Raspberry Pi model, install exception handlers and try a few of
 * them.
 */

#include <sys/cdefs.h>
#include <stdint.h>
#include "arch_info.h"
#include "kprintf.h"
//#include "exceptions/exceptions.h"
//#include "driver/timer/timer.h"

#define UNUSED(x) (void)(x)

typedef struct {
    uint32_t *kernel_page_table_phys;
    uint32_t *kernel_leaf_table_phys;
} BootInfo;

// main C function, called from boot.S
EXPORT void kernel_main(uint32_t r0, uint32_t id, const Atag *atag) {
    UNUSED(r0); // always 0
    UNUSED(id); // 0xC42 for Raspberry Pi

    arch_info_init(atag);
    kprintf("r0 = %#10.8lx\n", r0);

    // print boot info
    BootInfo *boot_info = (BootInfo *)r0;
    kprintf("kernel_page_table_phys = %p\n", boot_info->kernel_page_table_phys);
    kprintf("kernel_leaf_table_phys = %p\n", boot_info->kernel_leaf_table_phys);

    // print page table
    uint32_t addr = 0;
    uint32_t *l1 = boot_info->kernel_page_table_phys;
    for (int i = 0; i < 4096; ++i) {
	kprintf("%#10.8lx @ %p = %#10.8lx\n", addr, l1, *l1);
	uint32_t *l2 = (uint32_t *)((uintptr_t)*l1 & ~0x3FF);
	for (int j = 0; j < 256; ++j) {
	    if (*l2 != 0) kprintf("  %#10.8lx @ %p = %#10.8lx\n", addr, l2, *l2);
	    addr += 4096;
	    ++l2;
	}
	++l1;
    }
    
//    exceptions_init();

//    timer_test();
    
    kprintf("\nGoodbye\n");
}
