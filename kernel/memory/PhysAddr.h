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

/* wrapper for physical addresses
 */

#ifndef KERNEL_MEMORY_PHYSADDR_H
#define KERNEL_MEMORY_PHYSADDR_H 1

#include <sys/cdefs.h>
#include <stdint.h>

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(Memory);

struct PhysAddr {
    explicit constexpr PhysAddr(uint32_t phys) : x(phys) { }
    
    uint32_t x;
};

__END_NAMESPACE(Memory);
__END_NAMESPACE(Kernel);

#endif // #ifndef KERNEL_MEMORY_PHYSADDR_H



