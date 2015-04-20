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

/* Pagetable handling
 */

#include "pagetable.h"
#include <stdint.h>
#include "fixed_addresses.h"
#include "TableEntry.h"
#include "LeafEntry.h"
#include "PhysAddr.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(Memory);

struct KernelPageTable {
    TableEntry entry[4096];
    TableEntry & operator[](const void * const virt) {
	return entry[uintptr_t(virt) >> 20];
    }
} __attribute__((aligned(16384)));

struct KernelLeafTables {
    LeafEntry entry[1024 * 4096];
    LeafEntry & operator[](const void * const virt) {
	return entry[uintptr_t(virt) >> 12];
    }
} __attribute__((aligned(4096)));

static constexpr KernelPageTable &kernel_pagetable =
    *(KernelPageTable *)KERNEL_PAGETABLE;

static constexpr KernelLeafTables &kernel_leaftables =
    *(KernelLeafTables *)KERNEL_LEAFTABLES;

const TableEntry & table_entry(const void * const virt) {
    return kernel_pagetable[virt];
}

const LeafEntry & leaf_entry(const void * const virt) {
    if (kernel_pagetable[virt] == TableEntry::FAULT()) {
	return LeafEntry::FAULT();
    }
    return kernel_leaftables[virt];
}

static const char * const MODE_NAME[] = {
    "KERNEL_READ",
    "KERNEL_WRITE",
    "USER_READ",
    "USER_WRITE",
    "FRAMEBUFFER",
    "PERIPHERAL",
    "KERNEL_PERIPHERAL",
};

// FIXME
void panic(const char * const format, ...) {
    (void)format;
}

void map(PhysAddr phys, const void * const virt, Mode mode) {
    if (kernel_pagetable[virt] == TableEntry::FAULT()) {
	panic("%s(%#10.8X, %p, %s): no LeafTable", phys.x, virt, MODE_NAME[mode]);
    }
    LeafEntry & entry = kernel_leaftables[virt];
    if (entry != LeafEntry::FAULT()) {
	panic("%s(%#10.8X, %p, %s): double map", phys.x, virt, MODE_NAME[mode]);
    }

    // access rights to page
    static constexpr const LeafEntry::Ap ACCESS_KERNEL_READ{0b101};
    static constexpr const LeafEntry::Ap ACCESS_KERNEL_WRITE{0b001};
    static constexpr const LeafEntry::Ap ACCESS_USER_READ{0b111};
    static constexpr const LeafEntry::Ap ACCESS_USER_WRITE{0b011};

    // caching behaviour
    static constexpr const LeafEntry::M CACHED{LeafEntry::Tex(0b101),
	    !LeafEntry::CACHED, LeafEntry::BUFFERED};
    static constexpr const LeafEntry::M WRITE_THROUGH{LeafEntry::Tex(0b110),
	    LeafEntry::CACHED, !LeafEntry::BUFFERED};
    static constexpr const LeafEntry::M PERIPHERAL{LeafEntry::Tex(0b000),
	    !LeafEntry::CACHED, LeafEntry::BUFFERED};

    // other
    static constexpr const LeafEntry::Global G = LeafEntry::GLOBAL;
    static constexpr const LeafEntry::Shared S = LeafEntry::SHARED;
    
    // combined mode
    static constexpr const LeafEntry::M LEAF_MODE[] = {
	CACHED + ACCESS_KERNEL_READ + G + S,		// KERNEL_READ
	CACHED + ACCESS_KERNEL_WRITE + G + S,		// KERNEL_WRITE
	CACHED + ACCESS_USER_READ + !G + S,		// USER_READ
	CACHED + ACCESS_USER_WRITE + !G + S,		// USER_WRITE
	WRITE_THROUGH + ACCESS_USER_WRITE + !G + S,	// FRAMEBUFFER
	PERIPHERAL + ACCESS_KERNEL_WRITE + !G + S,	// PERIPHERAL
	PERIPHERAL + ACCESS_KERNEL_WRITE + G + S,	// KERNEL_PERIPHERAL
    };

    entry = LeafEntry(phys, LEAF_MODE[mode]);
}

__END_NAMESPACE(Memory);
__END_NAMESPACE(Kernel);
