/* syscall.h - syscall interface */
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

#ifndef MOOSE_KERNEL_SYSCALL_H
#define MOOSE_KERNEL_SYSCALL_H

#include <stdint.h>

extern "C" {
    // from entry.S
    uint32_t syscall(uint32_t arg0, uint32_t arg1, uint32_t arg2);
}

namespace Syscall {
    void init();
}

#endif // #ifndef MOOSE_KERNEL_SYSCALL_H
