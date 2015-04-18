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
#include "timer.h"
#include "memory/pagetable.h"
#include "memory/LeafEntry.h"
#include "memory/TableEntry.h"

#define UNUSED(x) (void)(x)

__BEGIN_NAMESPACE(Kernel);

typedef struct {
    uint32_t *kernel_page_table_phys;
    uint32_t *kernel_leaf_table_phys;
} BootInfo;

static inline void isb(void) {
    // asm volatile ("isb");
    asm volatile ("mcr p15, 0, r12, c7, c5, 4");
}
static inline void dsb(void) {
    // asm volatile ("dsb");
    asm volatile ("mcr p15, 0, r12, c7, c10, 4");
}
static inline void dmb(void) {
    // asm volatile ("dmb");
    asm volatile ("mcr p15, 0, r12, c7, c10, 5");
}

void delay(uint32_t count) {
//    uint32_t a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    while(count--) {
// branch icache dcache cycles/loop
// no     no     no     ~507     (47.602 RPi)  252
// no     no     yes      43.005                24.000
// no     yes    no        1.005                 1.500
// no     yes    yes       1.005                 1.500
// yes    no     no     ~507                   252
// yes    no     yes      43.005                24.000
// yes    yes    no        1.005                 1.500
// yes    yes    yes       1.005 (44.802 RPi)    1.500
asm volatile ("");

// branch icache dcache cycles/loop
// no     no     no     ~750
// no     no     yes      67.500
// no     yes    no       16.500
// no     yes    yes      16.500
// yes    no     no     ~750
// yes    no     yes      67.500
// yes    yes    no       16.500
// yes    yes    yes      16.500
// asm ("nop");
// asm ("nop");
// asm ("nop");
// asm ("nop");
// asm ("nop");
// asm ("nop");
// asm ("nop");
// asm ("nop");
// asm ("nop");
// asm ("nop");

// branch icache dcache cycles/loop
// no     no     no     ~505     (98.005 RPi)
// no     no     yes      46.500 (98.005 RPi)
// no     yes    no       10.500 (98.005 RPi)
// no     yes    yes      10.500 (98.005 RPi)
// yes    no     no     ~505     (91.289 RPi)
// yes    no     yes      46.500 (91.289 RPi)
// yes    yes    no       10.500 (91.289 RPi)
// yes    yes    yes      10.500 (91.117 RPi)
// asm volatile ("orr %0, %0, %0" : "=r" (a) : "r" (a));
// asm volatile ("add %0, %0, %0" : "=r" (b) : "r" (b));
// asm volatile ("and %0, %0, %0" : "=r" (c) : "r" (c));
// asm volatile ("mov %0, %0" : "=r" (d) : "r" (d));
// asm volatile ("orr %0, %0, %0" : "=r" (e) : "r" (e));
// asm volatile ("add %0, %0, %0" : "=r" (f) : "r" (f));
// asm volatile ("and %0, %0, %0" : "=r" (g) : "r" (g));
// asm volatile ("mov %0, %0" : "=r" (h) : "r" (h));

// branch icache dcache cycles/loop
// no     no     no     ~1010
// no     no     yes       85.005
// no     yes    no        18.000
// no     yes    yes       18.000
// yes    no     no     ~1010
// yes    no     yes       85.005
// yes    yes    no        18.000
// yes    yes    yes       18.000
// isb();

// branch icache dcache cycles/loop
// no     no     no     ~5075
// no     no     yes      481.501
// no     yes    no       141.000
// no     yes    yes      141.000
// yes    no     no     ~5075
// yes    no     yes      481.501
// yes    yes    no       141.000
// yes    yes    yes      141.000
// isb();
// isb();
// isb();
// isb();
// isb();
// isb();
// isb();
// isb();
// isb();
// isb();
    }
}

// main C function, called from boot.S
extern "C" EXPORT void kernel_main(uint32_t r0, uint32_t id, const Atag *atag) {
    UNUSED(r0); // always 0
    UNUSED(id); // 0xC42 for Raspberry Pi

    arch_info_init(atag);
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

//    exceptions_init();

//    timer_test();

    enum {
	CYCLES_PER_TICK = 900,
    };
    
    for (int i = 1; i < 1000000000; i *= 10) {
	uint64_t last = count();
	delay(i);
	uint64_t now = count();
	uint64_t cycles = (now -last) * CYCLES_PER_TICK;
	uint64_t t = cycles * 1000 / i;
	kprintf("cycles per loop [%10d loops] = %lld.%03lld\n", i, t / 1000, t % 1000);
    }

    kprintf("\nGoodbye\n");
}

__END_NAMESPACE(Kernel);
