/* entry.h - Exception, Interrupt and syscall entry point */
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

#ifndef MOOSE_KERNEL_ENTRY_H
#define MOOSE_KERNEL_ENTRY_H

// operating modes of CPU
enum Mode { MODE_USR = 0x10, MODE_FIQ = 0x11, MODE_IRQ = 0x12, MODE_SVC = 0x13,
	    MODE_SECURE = 0x16, MODE_ABORT = 0x17, MODE_UNDEF = 0x1B,
	    MODE_SYS = 0x1F };

extern "C" {
    extern void set_mode_stack(Mode mode, uint32_t *stack_top);
}

#endif // #ifndef MOOSE_KERNEL_ENTRY_H
