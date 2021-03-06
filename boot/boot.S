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

/* assembly code to map virtual memory and jump to higher half
 *
 * This code is written completely position independent. All physical
 * addresses are automatically corrected for the address the image is
 * actually loaded and executed from at runtime. The code builds a 16k
 * kernel page table, adds an identity mappings for itself, the page
 * tables, the atag data, the peripherals and the boot info structure
 * and maps all the higher half segments into the higher half of the
 * address space. It then enables caching and the MMU and jumps to the
 * higher half entry point. The address of boot info is passed in r0,
 * the system ID in r1 and the atags in r2.
 */

#include "fixed_addresses.h"

.section ".text"
.global _start
.type _start, STT_FUNC

// load a physical address relative to _start
.macro ld_phys,reg,name
	ldr	\reg, =\name
	add	\reg, \reg, r12
.endm

// load physical address of virtual symbol relative to _start
.macro ld_virt,reg,name
	ldr	\reg, =\name
	add	\reg, \reg, r11
.endm

_start:
	// the bootloader passes 3 arguments:
	// r0 = 0
	// r1 = 0xC42 (ID for Raspberry Pi)
	// r2 = ATAGS
	// those must be preserved for the kernel_start function

	// r12 = offset between where _start should be and where it is
	// used to correct constant loading of physical addresses
	sub	r12, pc, #8
	ldr	r3, =_start
	sub	r12, r12, r3
	// r11 = offset for physical address of virtual symbols
	add	r11, r12, #VIRT_TO_PHYS
	
	// construct kernel page table and map everything
	// ==============================================

	// initilize L1 table with leaf tables
construct_kernel_page_table:
	ld_phys	r3, kernel_page_table
	ld_phys	r4, kernel_leaf_tables
        // addr(22) P DOM(4) 0 NS 0 0 1 = addr | 0b0000000001
	orr	r4, r4, #0x001		// coarse leaf table
	add	r5, r3, #16384		// 4096 leaf table entries
1:	str	r4, [r3], #4		// store entry
	cmp	r5, r3			// done?
	add	r4, r4, #0x400		// next leaf table
	bhi	1b

clear_leaf:
	// clear kernel leaf tables
	ld_phys	r3, kernel_leaf_tables
	mov	r4, #0			// translation fault
	add	r5, r3, #1024 * 4096	// 4096 1k tables
1:	str	r4, [r3], #4		// store entry
	cmp	r5, r3			// done=
	bhi	1b

map_memory:
	// map memory
	// r3 = base of leaf tables
	ld_phys	r3, kernel_leaf_tables
map_boot_code:
	// identity map boot code
	ld_phys	r4, _text_boot_start	// PC relative boot code start
	mov	r5, r4			// phys = virt
	ld_phys	r6, _text_boot_end	// PC relative boot code end
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, exec, not global
        // 0b1101 0101 0110
	ldr	r7, =0xD56
	bl	map_range

map_kernel_text:
	// map kernel text segment
	ldr	r4, =_text_kernel_start	// absolute (virtual) code start
	ld_virt	r5, _text_kernel_start  // PC relative (physical) code start
	ld_virt	r6, _text_kernel_end	// PC relative (physical) code end
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-only, exec, global
        // 0b0111 0101 0110
	ldr	r7, =0x756
	bl	map_range

map_common_text:
	// map common text segment
	ldr	r4, =_text_common_start	// absolute (virtual) code start
	ld_virt	r5, _text_common_start	// PC relative (physical) code start
	ld_virt	r6, _text_common_end	// PC relative (physical) code end
        // outer and inner write back, write allocate, shareable,
	// accessed, user read-only, exec, global
        // 0b0111 0111 0110
	ldr	r7, =0x776
	bl	map_range

map_kernel_rodata:
	// map kernel rodata segment
	ldr	r4, =_rodata_kernel_start// absolute (virtual) rodata start
	ld_virt	r5, _rodata_kernel_start// PC relative (physical) rodata start
	ld_virt	r6, _rodata_kernel_end	// PC relative (physical) rodata end
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-only, not exec, global
        // 0b0111 0101 0111
	ldr	r7, =0x757
	bl	map_range

map_kernel_data_bss:
	// map kernel data and bss segment
	ldr	r4, =_data_kernel_start	// absolute (virtual) data start
	ld_virt	r5, _data_kernel_start	// PC relative (physical) data start
	ld_virt	r6, _bss_kernel_end	// PC relative (physical) data end
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, not exec, global
        // 0b0101 0101 0111
	ldr	r7, =0x557
	bl	map_range

map_boot_info:
	// map boot info
	ld_phys	r4, boot_info		// PC relative boot info
	mov	r5, r4			// phys = virt
	add	r6, r5, #4096		// page after boot info
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, not exec, not global
        // 0b1101 0101 0111
	ldr	r7, =0xD57
	bl	map_range

map_atags:
	// map atags
	ldr	r4, =0x00000000		// absolute (physical) atags start
	mov	r5, r4			// phys = virt
	ldr	r6, =0x00008000		// absolute (physical) atags end
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, not exec, not global
        // 0b1101 0101 0111
	ldr	r7, =0xD57
	bl	map_range

	// save atags address
	ld_virt	r4, atags
	str	r2, [r4, #0]

	// map fixed address structures
	//=============================
map_pagetable:
	// map page table
	ldr	r4, =KERNEL_PAGETABLE   // absolute (virtual)
	ld_phys	r5, kernel_page_table	// PC relative kernel page table
	add	r6, r5, #16384		// size = 16k
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, not exec, not global
        // 0b1101 0101 0111
	ldr	r7, =0xD57
	bl	map_range

map_leaftables:
	// map leaf tables
	ldr	r4, =KERNEL_LEAFTABLES  // absolute (virtual)
	ld_phys	r5, kernel_leaf_tables	// PC relative kernel leaf tables
	add	r6, r5, #4096 * 1024	// size = 4M
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, not exec, not global
        // 0b1101 0101 0111
	ldr	r7, =0xD57
	bl	map_range

	// tell the kernel where stuff is
	// ==============================
fill_boot_info:
	// fill out boot info
	ld_phys	r0, boot_info
	ld_phys	r3, kernel_page_table
	str	r3, [r0, #0]
	ld_phys	r3, kernel_leaf_tables
	str	r3, [r0, #4]

	// lets go virtual
	// ===============
enable_mmu:
        // set SMP bit in ACTLR
        mrc	p15, 0, r3, c1, c0, 1
	orr 	r3, #1<<6
        mcr	p15, 0, r3, c1, c0, 1

	// setup domains (CP15 c3)
        // Write Domain Access Control Register
        // use access permissions from TLB entry
	ldr	r3, =0x55555555
        mcr     p15, 0, r3, c3, c0, 0

        // set domain 0 to client
	ldr	r3, =1
        mcr	p15, 0, r3, c3, c0, 0

        // c2, Translation Table Base Control Register
        ldr     r4, =2 // 4K Table 0
        mcr     p15, 0, r4, c2, c0, 2 // Write TTBCR

        // set TTBR0 (page table walk inner and outer write-back,
        // write-allocate, cacheable, shareable memory)
	ld_phys	r3, kernel_page_table
	orr	r3, r3, #0b1001010
	mcr	p15, 0, r3, c2, c0, 0
	// same for TTBR1
	mcr	p15, 0, r3, c2, c0, 1

        /* SCTLR
         * Bit 31: SBZ     reserved
         * Bit 30: TE      Thumb Exception enable (0 - take in ARM state)
         * Bit 29: AFE     Access flag enable (1 - simplified model)
         * Bit 28: TRE     TEX remap enable (0 - no TEX remapping)
         * Bit 27: NMFI    Non-Maskable FIQ (read-only)
         * Bit 26: 0       reserved
         * Bit 25: EE      Exception Endianness (0 - little-endian)
         * Bit 24: VE      Interrupt Vectors Enable (0 - use vector table)
         * Bit 23: 1       reserved
         * Bit 22: 1/U     (alignment model)
         * Bit 21: FI      Fast interrupts (probably read-only)
         * Bit 20: UWXN    (Virtualization extension)
         * Bit 19: WXN     (Virtualization extension)
         * Bit 18: 1       reserved
         * Bit 17: HA      Hardware access flag enable (0 - enable)
         * Bit 16: 1       reserved
         * Bit 15: 0       reserved
         * Bit 14: RR      Round Robin select (0 - normal replacement strategy)
         * Bit 13: V       Vectors bit (0 - remapped base address)
         * Bit 12: I       Instruction cache enable (1 - enable)
         * Bit 11: Z       Branch prediction enable (1 - enable)
         * Bit 10: SW      SWP/SWPB enable (maybe RAZ/WI)
         * Bit 09: 0       reserved
         * Bit 08: 0       reserved
         * Bit 07: 0       endian support / RAZ/SBZP
         * Bit 06: 1       reserved
         * Bit 05: CP15BEN DMB/DSB/ISB enable (1 - enable)
         * Bit 04: 1       reserved
         * Bit 03: 1       reserved
         * Bit 02: C       Cache enable (1 - data and unified caches enabled)
         * Bit 01: A       Alignment check enable (1 - fault when unaligned)
         * Bit 00: M       MMU enable (1 - enable)
         */

        // enable MMU, caches, branch prediction, ... in SCTLR
        ldr     r5, =0x73027827
        ldr     r6, =0x20001827
        mrc     p15, 0, r4, c1, c0, 0 // Read Control Register
        and     r4, r4, r5
        orr     r4, r4, r6
	mcr     p15, 0, r4, c1, c0, 0 // Write Control Register

	// jump to kernel_start in higher half memory
	ldr	r3, =kernel_start
	bx	r3

.global	map_range
.type	map_range, STT_FUNC
map_range:
	// r3 = base of leaf tables
	// r4 = virtual start
	// r5 = physical start
	// r6 = physical end
	// r7 = mode
	lsr	r8, r4, #10		// entry number to map
	add	r8, r8, r3		// entry in leaf table
	orr	r5, r5, r7		// add mode to virtual address
1:	str	r5, [r8], #4		// store entry
	add	r5, r5, #0x1000		// next page
	cmp	r6, r5			// done?
	bhi	1b			// loop while r6 > r5
	bx	lr			// return
	
// constants for ldr macro
constants:
.ltorg

boot_end:

.section ".bss"
// 16k L1 page table aligned to 16k
.balign 16384
.global kernel_page_table
.type kernel_page_table, STT_OBJECT
kernel_page_table:	
	.space	16384

// L2 page tables aligned to 4k
.balign 4096
.global kernel_leaf_tables
.type kernel_leaf_tables, STT_OBJECT
kernel_leaf_tables:
	.space	1024 * 4096

// boot info structure
.balign 4096
.global boot_info
.type boot_info, STT_OBJECT
boot_info:
	.space 4	// kernel_page_table
	.space 4	// kernel_leaf_table
