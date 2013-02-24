/* memory.cc - Memory management */
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
#include <stddef.h>

#include <memory.h>
#include <panic.h>

extern "C" {
    extern uint8_t _end[];
}

namespace Memory {
    /* Memory map at boot:
     *
     * 0x00000000 - 0x0FFFFFFF: LEGACY - cached, alloc on write memory
     * 0x10000000 - 0x1FFFFFFF: LEGACY - write-through, no alloc on write memory
     * 0x20000000 - 0x3FFFFFFF: LEGACY - device memory (peripherals)
     * 0x40000000 - 0xBFFFFFFF: UNDEF  - not present
     * 0xC0000000 - 0xCFFFFFFF: KERNEL - cached, alloc on write memory
     * 0xD0000000 - 0xDFFFFFFF: KERNEL - write-through, no alloc on write memory
     * 0xE0000000 - 0xFFFFFFFF: KERNEL - device memory (peripherals)
     */
    enum {
	VIRT_BASE = 0xC0000000,		// base address of kernel memory
	PERIPHERAL_BASE = 0xE0000000,	// virtual base of peripheral region
    };
    
    uint8_t *next_free = _end;
    uint8_t *last = NULL;

    volatile uint32_t * Peripheral::last = NULL;

    void init(const ATAG::Header *atags) {
	/****************************************************************
	 * Parse atags to find amount of memory				*
	 ****************************************************************/
	while((atags != NULL) && (atags->tag != ATAG::MEM)) {
            atags = atags->next();
        }
        if (atags == NULL) {
            panic("No memory tag found!");
        }
        ATAG::Mem const &mem = atags->as_mem();
        // with 2G mem this becomes 0 but RPi can only have 1G
        last = (uint8_t*)(0xC0000000 + mem.start + mem.size);
	if (next_free >= last) {
	    panic("Less memory available than being used already!");
	}
    }

    /* allocate a chunk of kernel memory during bootstrap
     * order:   size of memory in pages as power of 2 (2^order pages)
     * returns: pointer to kernel memory
     */
    void *early_malloc(int order) {
        uint32_t size = 1 << order;
        void *res = next_free;
        if (next_free + size >= last) {
            panic("Out of memory!");
        }
        next_free += size;
        return res;
    }

    /* Claim a peripheral region
     * offset:  address or peripheral relative to peripheral region
     * size:    size of region to claim (multiple of PAGE_SIZE)
     * returns: pointer to peripheral mapped into the process address space
     */
    // FIXME:
    // This should mark the peripheral as in use and map it into the processes
    // address space. Since we don't have processes yet simply return the
    // address in kernel space where peripherals are mapped.
    Peripheral alloc_peripheral(uint32_t offset, uint32_t size) {
	return Peripheral((volatile uint32_t *)(PERIPHERAL_BASE + offset),
			  size);
    }
}
