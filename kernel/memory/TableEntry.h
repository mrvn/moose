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

/* First level pagetable entry bitfield
 */

#ifndef KERNEL_MEMORY_TABLEENTRY_H
#define KERNEL_MEMORY_TABLEENTRY_H 1

#include <sys/cdefs.h>
#include <Bitfield.h>
#include "PhysAddr.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(Memory);

// Bitfield for Pagetable entries
class TableEntry : public Bitfield<TableEntry> {
public:
    /*
    struct {
	uint32_t addr       : 22;
	uint32_t ecc        : 1;
	uint32_t domain     : 4;
	uint32_t SBZ        : 1;
	uint32_t not_secure : 1;
	uint32_t SBZ        : 1;
	uint32_t ZERO       : 1; // Coarse page table
	uint32_t ONE        : 1; // Coarse page table
    };
    */
    using Addr      = Bits<31, 10>;

    explicit constexpr TableEntry() : Bitfield(RAW, uint32_t(0)) { }

    template<typename ... Ts>
    constexpr TableEntry(PhysAddr phys, const Ts ... ts)
        : Bitfield(Addr(phys.x >> 12), ts ...) { }
    static constexpr M FIXED() {
	return M(!ECC, SBZ, COARSE);
    };
    static constexpr M DEFAULT() {
	return M(Domain(0));
    };
    static const TableEntry FAULT() { return TableEntry(); }

    bool operator == (const TableEntry other) const {
	return raw() == other.raw();
    }
    bool operator != (const TableEntry other) const {
	return raw() != other.raw();
    }
private:
    using Ecc       = Bit<9>;
    using Domain    = Bits<8, 5>;
    using Sbz       = Field<Bit<4>, Bit<2> >;
    using Coarse    = Bits<1, 0>;
    static constexpr const Ecc ECC{true};
    static constexpr const Sbz SBZ{0b11};
    static constexpr const Coarse COARSE{0b01};
};

__END_NAMESPACE(Memory);
__END_NAMESPACE(Kernel);

#endif // #ifndef KERNEL_MEMORY_TABLEENTRY_H


