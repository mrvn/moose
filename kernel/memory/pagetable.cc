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
#include "../fixed_addresses.h"
#include "TableEntry.h"
#include "LeafEntry.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(Memory);

struct KernelPageTable {
    TableEntry entry[4096];
} __attribute__((aligned(16384)));

struct KernelLeafTables {
    LeafEntry entry[1024 * 4096];
} __attribute__((aligned(4096)));

static constexpr const KernelPageTable *kernel_pagetable =
    (const KernelPageTable *)KERNEL_PAGETABLE;

static constexpr const KernelLeafTables *kernel_leaftables =
    (const KernelLeafTables *)KERNEL_LEAFTABLES;

const TableEntry table_entry(const void * const virt) {
    return kernel_pagetable->entry[uintptr_t(virt) >> 20];
}

const LeafEntry leaf_entry(const void * const virt) {
    if (table_entry(virt) == TableEntry::FAULT()) {
	return LeafEntry::FAULT();
    }
    return kernel_leaftables->entry[uintptr_t(virt) >> 12];
}

__END_NAMESPACE(Memory);
__END_NAMESPACE(Kernel);
