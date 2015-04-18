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

/* constants for structures at fixed addresses
 */

#ifndef KERNEL_FIXED_ADDRESSES_H
#define KERNEL_FIXED_ADDRESSES_H 1

#include <sys/cdefs.h>

__BEGIN_DECLS

#define VIRT_TO_PHYS           0x80000000
#define PHYS_TO_VIRT           0x80000000

#define CORE0_SVC_STACK        0xC0000000 /* 16k stack for SVC mode */
#define CORE0_SYS_STACK        0xC0008000 /* 16k stack for SYS mode */
#define CORE0_ABORT_STACK      0xC0010000 /* 16k stack for ABORT mode */
#define CORE0_OTHER_PAGETABLE  0xC0200000 /* 4k target pagetable on copy */
#define CORE0_OTHER_LEAFTABLES 0xC0400000 /* 1M target leaftables on copy */
#define CORE0_PRIVATE          0xC0800000
    
#define CORE1_SVC_STACK        0xC0000000 /* 16k stack for SVC mode */
#define CORE1_SYS_STACK        0xC0008000 /* 16k stack for SYS mode */
#define CORE1_ABORT_STACK      0xC0010000 /* 16k stack for ABORT mode */
#define CORE1_OTHER_PAGETABLE  0xC1200000 /* 4k target pagetable on copy */
#define CORE1_OTHER_LEAFTABLES 0xC1400000 /* 1M target leaftables on copy */
#define CORE1_PRIVATE          0xC1800000
    
#define CORE2_SVC_STACK        0xC0000000 /* 16k stack for SVC mode */
#define CORE2_SYS_STACK        0xC0008000 /* 16k stack for SYS mode */
#define CORE2_ABORT_STACK      0xC0010000 /* 16k stack for ABORT mode */
#define CORE2_OTHER_PAGETABLE  0xC2200000 /* 4k target pagetable on copy */
#define CORE2_OTHER_LEAFTABLES 0xC2400000 /* 1M target leaftables on copy */
#define CORE2_PRIVATE          0xC2800000
    
#define CORE3_SVC_STACK        0xC0000000 /* 16k stack for SVC mode */
#define CORE3_SYS_STACK        0xC0008000 /* 16k stack for SYS mode */
#define CORE3_ABORT_STACK      0xC0010000 /* 16k stack for ABORT mode */
#define CORE3_OTHER_PAGETABLE  0xC3200000 /* 4k target pagetable on copy */
#define CORE3_OTHER_LEAFTABLES 0xC3400000 /* 1M target leaftables on copy */
#define CORE3_PRIVATE          0xC3800000

#define KERNEL_GPIO            0xD0000000 /* 4k GPIO registers (LED) */
#define KERNEL_UART            0xD0002000 /* 4k UART registers */
#define KERNEL_CORE_MAIL       0xD0004000 /* 4k core mail boxes */
#define KERNEL_VC_MAIL         0xD0006000 /* 4k VC mail boxes */
#define KERNEL_PAGETABLE       0xD0200000 /* 16k (first 8k unused) */
#define KERNEL_LEAFTABLES      0xD0400000 /* 4M (first 2M unmapped) */
#define PER_PAGE_INFO          0xE0000000 /* 4M (size ram / 1024) */

__END_DECLS

#endif // ##ifndef KERNEL_FIXED_ADDRESSES_H
