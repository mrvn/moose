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
#include "timer.h"
#include "memory/pagetable.h"
#include "memory/LeafEntry.h"
#include "memory/TableEntry.h"

#define UNUSED(x) (void)(x)

__BEGIN_NAMESPACE(Kernel);

void ARCH_INFO_init(void);

extern "C" {
 // constructors
    typedef void (*constructor_t)(void);
    extern constructor_t _init_array_start[];
    extern constructor_t _init_array_end[];

    EXPORT void kernel_constructors(void) {
	for(constructor_t *fn = _init_array_start;
	    fn != _init_array_end; ++fn) {
	    (*fn)();
	}
    }
};

typedef struct {
    uint32_t *kernel_page_table_phys;
    uint32_t *kernel_leaf_table_phys;
} BootInfo;

// main C function, called from boot.S
extern "C" EXPORT void kernel_main(uint32_t r0, uint32_t id, const Atag *atag) {
    UNUSED(r0); // always 0
    UNUSED(id); // 0xC42 for Raspberry Pi
    UNUSED(atag);
    
    kprintf("r0 = %#10.8lx\n", r0);

    // print boot info
    BootInfo *boot_info = (BootInfo *)r0;
    kprintf("kernel_page_table_phys = %p\n", boot_info->kernel_page_table_phys);
    kprintf("kernel_leaf_table_phys = %p\n", boot_info->kernel_leaf_table_phys);
    
    // print page table
    const char * addr = (char * const)0;
    for (int i = 0; i < 4096; ++i) {
	const Memory::TableEntry te = Memory::table_entry(addr);
	bool printed = false;
	const char * addr2 = addr;
	const char *last_addr = addr;
	Memory::LeafEntry last = Memory::LeafEntry::FAULT();
	const char *sep = "  ...\n";
	for (int j = 0; j < 256; ++j) {
	    const Memory::LeafEntry le = Memory::leaf_entry(addr2);
	    if (le != Memory::LeafEntry::FAULT()) {
		if (!printed) {
		    kprintf("%p = %#10.8lX\n", addr, te.raw());
		    printed = true;
		}
		if (last.raw() + 4096 == le.raw()) {
		    kprintf("%s", sep);
		    sep = "";
		} else {
		    if ((last != Memory::LeafEntry::FAULT())  && (sep[0] == 0)){
			kprintf("  %p = %#10.8lX\n", last_addr, last.raw());
		    }
		    kprintf("  %p = %#10.8lX\n", addr2, le.raw());
		    sep = "  ...\n";
		}
		last_addr = addr2;
		last = le;
	    } else {
		if ((last != Memory::LeafEntry::FAULT()) && (sep[0] == 0)) {
		    kprintf("  %p = %#10.8lX\n", last_addr, last.raw());
		}
		last = Memory::LeafEntry::FAULT();
	    }
	    addr2 += 4096;
	}
	if ((last != Memory::LeafEntry::FAULT())  && (sep[0] == 0)){
	    kprintf("  %p = %#10.8lX\n", last_addr, last.raw());
	}
	addr = addr2;
    }

    Timer::test();

    kprintf("\nGoodbye\n");
}

__END_NAMESPACE(Kernel);
