/* memory.h - Memory management */
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

#ifndef MOOSE_KERNEL_MEMORY_H
#define MOOSE_KERNEL_MEMORY_H

#include <stdint.h>
#include <atags.h>
#include <panic.h>

namespace Memory {
    enum { PAGE_SIZE = 4096 };

    /* http://infocenter.arm.com/help/topic/com.arm.doc.ddi0360f/I1014942.html
     *
     * mov r3, #0                      # The read register Should Be Zero before
 the call
     * mcr p15, 0, r3, C7, C6, 0       # Invalidate Entire Data Cache
     * mcr p15, 0, r3, c7, c10, 0      # Clean Entire Data Cache
     * mcr p15, 0, r3, c7, c14, 0      # Clean and Invalidate Entire Data Cache
     * mcr p15, 0, r3, c7, c10, 4      # Data Synchronization Barrier
     * mcr p15, 0, r3, c7, c10, 5      # Data Memory Barrier
     */

    // extensive barrier
    static inline void full_barrier(void) {
        // invalidate I cache
        asm volatile("mcr p15, #0, [zero], c7, c5, #0"
                     : : [zero]"r"(0));
        // invalidate BTB
        asm volatile("mcr p15, #0, [zero], c7, c5, #6"
                     : : [zero]"r"(0));
        // drain write buffer
        asm volatile("mcr p15, #0, [zero], c7, c10, #4"
                     : : [zero]"r"(0));
        // prefetch flush
        asm volatile("mcr p15, #0, [zero], c7, c5, #4"
                     : : [zero]"r"(0));
    }

    /*
     * Data memory barrier
     *
     * * Any explicit memory access by an instruction before the DMB is
     *   globally observed before any memory accesses caused by an
     *   instruction after the DMB.
     * * The DMB has no effect on the ordering of any other instructions
     *   executing on the processor.
     *
     * As such, DMB ensures the apparent order of the explicit memory
     * operations before and after the DMB instruction, but does not ensure
     * the completion of those memory operations.
     */
    static inline void data_memory_barrier(void) {
        asm volatile("mcr p15, #0, %[zero], c7, c10, #5"
                     : : [zero]"r"(0));
    }

    /* 
     * Data synchronisation barrier
     *
     * * all explicit reads and writes before the DSB complete
     * * all Cache, Branch predictor and TLB maintenance operations before the
     *   DSB complete. 
     *
     * No instruction after the DSB can execute until the DSB completes.
     */
    static inline void data_sync_barrier(void) {
        asm volatile("mcr p15, #0, %[zero], c7, c10, #4"
                     : : [zero]"r"(0));
    }

    /*
     * Clean and invalidate entire cache
     * Flush pending writes to main memory
     * Remove all data in data cache
     */
    static inline void flush_cache(void) {
        asm volatile("mcr p15, #0, %[zero], c7, c14, #0"
                     : : [zero]"r"(0));
    }

    /**********************************************************************/
    
    class Peripheral {
    public:
	uint32_t get(uint32_t offset) {
	    // memory read barrier after the last read of a peripheral
	    if (base != last) {
		data_sync_barrier();
		last = base;
	    }
	    // FIXME: kill thread
	    if (offset >= size) panic("Memory::Peripheral::get() beyond end!");
	    return base[offset / 4];
	};
	void set(uint32_t offset, uint32_t data) {
	    // memory write barrieri before the first write of a peripheral
	    if (base != last) {
		data_sync_barrier();
		last = base;
	    }
	    // FIXME: kill thread
	    if (offset >= size) panic("Memory::Peripheral::set() beyond end!");
	    base[offset / 4] = data;
	}
	explicit Peripheral() : base(0), size(0) { }
	Peripheral & operator=(const Peripheral& p) {
	    if (size != 0) panic("Memory::Peripheral::operator=() misuse!");
	    base = p.base;
	    size = p.size;
	    return *this;
	}
    private:
	explicit Peripheral(volatile uint32_t *nbase, uint32_t nsize)
	    : base(nbase), size(nsize) { }
	volatile uint32_t *base;
	uint32_t size;
	static volatile uint32_t *last;
	friend Peripheral alloc_peripheral(uint32_t offset, uint32_t size);
    };
    
    /**********************************************************************/
    
    void init(const ATAG::Header *atags);

    /* allocate a chunk of kernel memory during bootstrap
     * order:   size of memory in pages as power of 2 (2^order pages)
     * returns: pointer to kernel memory
     */
    void *early_malloc(int order);

    /* Claim a peripheral region
     * offset:  address or peripheral relative to peripheral region
     * size:    size of region to claim (multiple of 4)
     * returns: pointer to peripheral mapped into the process address space
     */
    Peripheral alloc_peripheral(uint32_t offset, uint32_t size);
}

#endif // #ifndef MOOSE_KERNEL_MEMORY_H
