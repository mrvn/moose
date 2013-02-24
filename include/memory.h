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

#include <atags.h>

namespace Memory {
    enum { PAGE_SIZE = 4096 };
    
    void init(const ATAG::Header *atags);

    /* allocate a chunk of kernel memory during bootstrap
     * order:   size of memory in pages as power of 2 (2^order pages)
     * returns: pointer to kernel memory
     */
    void *early_malloc(int order);

    /* Claim a peripheral region
     * offset:  address or peripheral relative to peripheral region
     * size:    size of region to claim (multiple of PAGE_SIZE)
     * returns: pointer to peripheral mapped into the process address space
     */
    void *alloc_peripheral(uint32_t offset, uint32_t size = PAGE_SIZE);
}

#endif // #ifndef MOOSE_KERNEL_MEMORY_H
