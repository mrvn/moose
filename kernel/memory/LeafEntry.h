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

/* Second level Pagetable entry bitfield
 */

#ifndef KERNEL_MEMORY_LEAFENTRY_H
#define KERNEL_MEMORY_LEAFENTRY_H 1

#include <sys/cdefs.h>
#include <Bitfield.h>
#include "PhysAddr.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(Memory);

// Bitfield for LeafTable entries
class LeafEntry : public Bitfield<LeafEntry> {
public:
    /*
    struct {
	uint32_t addr       : 20;
	uint32_t not_global : 1;
	uint32_t shared     : 1;
	uint32_t ap2        : 1;
	uint32_t tex        : 3;
	uint32_t ap         : 2;
	uint32_t cached     : 1;
	uint32_t buffered   : 1;
	uint32_t ONE        : 1;
	uint32_t not_exec   : 1;
    };
    */
    using Addr      = Bits<31, 12>;
    using Global    = Bit<11>;
    using Shared    = Bit<10>;
    using Ap        = Field<Bit<9>, Bits<5, 4> >;
    using Tex       = Bits<8, 6>;
    using Cached    = Bit<3>;
    using Buffered  = Bit<2>;
    using Exec      = Bit<0>;

    static constexpr const Global GLOBAL{false};
    static constexpr const Shared SHARED{true};
    static constexpr const Cached CACHED{true};
    static constexpr const Buffered BUFFERED{true};
    static constexpr const Exec EXEC{false};

    explicit constexpr LeafEntry() : Bitfield(RAW, uint32_t(0)) { }

    template<typename ... Ts>
    constexpr LeafEntry(PhysAddr phys, const Ts ... ts)
        : Bitfield(Addr(phys.x >> 12), ts ...) { }
    static constexpr M FIXED() {
	return M(SMALL_PAGE);
    };
    static constexpr M DEFAULT() {
	return M(!GLOBAL, !EXEC);
    };
private:
    using SmallPage = Bit<1>;
    static constexpr const SmallPage SMALL_PAGE{true};
};

__END_NAMESPACE(Memory);
__END_NAMESPACE(Kernel);

#endif // #ifndef KERNEL_MEMORY_LEAFENTRY_H


