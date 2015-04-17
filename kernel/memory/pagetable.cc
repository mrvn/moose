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
#include "LeafEntry.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(Memory);

LeafEntry entry(PhysAddr(0x3F000000), LeafEntry::GLOBAL);

__END_NAMESPACE(Memory);
__END_NAMESPACE(Kernel);
