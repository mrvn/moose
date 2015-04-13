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

/* assembly code to map virtual memory and jump to higher half */

// Mapping between virtual and physical memory
#define VIRT_TO_PHYS 0x80000000

.section ".text"
.global _start
.type _start, STT_FUNC

// load an address relative to the PC
.macro load,reg,name
1:
	ldr	\reg, 2f
	add	\reg, \reg, pc
	b	3f
2:
	.word	\name - 1b - 12
3:
.endm

_start:
	// the bootloader passes 3 arguments:
	// r0 = 0
	// r1 = 0xC42 (ID for Raspberry Pi)
	// r2 = ATAGS
	// those must be preserved to the kernel_start function

	// construct kernel page table
        // addr(22) P DOM(4) 0 NS 0 0 1
	load	r3, kernel_page_table
	load	r4, kernel_leaf_tables
	orr	r4, r4, #0x001		// coarse leaf table
	add	r5, r3, #16384		// 4096 leaf table entries
1:	str	r4, [r3], #4		// store entry
	cmp	r3, r5			// done?
	add	r4, r4, #0x400		// next leaf table
	bne	1b

	// clear kernel leaf tables
	load	r3, kernel_leaf_tables
	mov	r4, #0			// translation fault
	add	r5, r3, #1024 * 4096	// 4096 1k tables
1:	str	r4, [r3], #4		// store entry
	cmp	r3, r5			// done=
	bne	1b

	// map memory
	// r3 = base of leaf tables
	load	r3, kernel_leaf_tables
	// identity map boot code
	load	r4, _text_boot_start	// PC relative boot code start
	load	r5, _text_boot_end	// PC relative boot code end
	lsr	r6, r4, #10		// entry number to map
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, exec, not global
        // 0b1101 0101 0110
	orr	r4, r4, #0xD00
	orr	r4, r4, #0x056
1:	str	r4, [r3, r6]		// store entry
	cmp	r4, r5			// done?
	add	r6, r6, #4		// next entry
	add	r4, r4, #0x1000		// next page
	blt	1b

	// map kernel text segment
	ldr	r4, =_text_kernel_start	// absolute (virtual) code start
	ldr	r5, =_text_kernel_end	// absolute (virtual) code end
	lsr	r6, r4, #10		// entry number to map
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-only, exec, global
        // 0b0111 0101 0110
	orr	r4, r4, #0x700
	orr	r4, r4, #0x056
1:	str	r4, [r3, r6]		// store entry
	cmp	r4, r5			// done?
	add	r6, r6, #4		// next entry
	add	r4, r4, #0x1000		// next page
	blt	1b
	
	// map common text segment
	ldr	r4, =_text_common_start	// absolute (virtual) code start
	ldr	r5, =_text_common_end	// absolute (virtual) code end
	lsr	r6, r4, #10		// entry number to map
        // outer and inner write back, write allocate, shareable,
	// accessed, user read-only, exec, global
        // 0b0111 0111 0110
	orr	r4, r4, #0x700
	orr	r4, r4, #0x076
1:	str	r4, [r3, r6]		// store entry
	cmp	r4, r5			// done?
	add	r6, r6, #4		// next entry
	add	r4, r4, #0x1000		// next page
	blt	1b
	
	// map rodata segment
	ldr	r4, =_rodata_kernel_start // absolute (virtual) rodata start
	ldr	r5, =_rodata_kernel_end	// absolute (virtual) rodata end
	lsr	r6, r4, #10		// entry number to map
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-only, not exec, global
        // 0b0111 0101 0111
	orr	r4, r4, #0x700
	orr	r4, r4, #0x057
1:	str	r4, [r3, r6]		// store entry
	cmp	r4, r5			// done?
	add	r6, r6, #4		// next entry
	add	r4, r4, #0x1000		// next page
	blt	1b
	
	// map data and bss segment
	ldr	r4, =_data_kernel_start	// absolute (virtual) data start
	ldr	r5, =_bss_kernel_end	// absolute (virtual) bss end
	lsr	r6, r4, #10		// entry number to map
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, not exec, global
        // 0b0101 0101 0111
	orr	r4, r4, #0x500
	orr	r4, r4, #0x057
1:	str	r4, [r3, r6]		// store entry
	cmp	r4, r5			// done?
	add	r6, r6, #4		// next entry
	add	r4, r4, #0x1000		// next page
	blt	1b
	
	// map page tables
	load	r4, kernel_page_table	// PC relative kernel page tables
	load	r5, boot_info		// PC relative boot info
	add	r5, r5, #4096		// page after boot info
	lsr	r6, r4, #10		// entry number to map
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, not exec, not global
        // 0b1101 0101 0111
	orr	r4, r4, #0xD00
	orr	r4, r4, #0x057
1:	str	r4, [r3, r6]		// store entry
	cmp	r4, r5			// done?
	add	r6, r6, #4		// next entry
	add	r4, r4, #0x1000		// next page
	blt	1b

	// map atags
	ldr	r4, =0x00000000		// absolute (physical) atags start
	ldr	r5, =0x00008000		// absolute (physical) atags end
	lsr	r6, r4, #10		// entry number to map
        // outer and inner write back, write allocate, shareable,
	// accessed, kernel read-write, not exec, not global
        // 0b1101 0101 0111
	orr	r4, r4, #0xD00
	orr	r4, r4, #0x057
1:	str	r4, [r3, r6]		// store entry
	cmp	r4, r5			// done?
	add	r6, r6, #4		// next entry
	add	r4, r4, #0x1000		// next page
	blt	1b

	// map peripherals
	ldr	r4, =0x3F000000		// absolute (physical) peripherals start
	ldr	r5, =0x40000000		// absolute (physical) peripherals end
	lsr	r6, r4, #10		// entry number to map
        // Shareable Device
	// accessed, kernel read-write, not exec, global
        // 0b0100 0001 0111
	orr	r4, r4, #0x400
	orr	r4, r4, #0x017
1:	str	r4, [r3, r6]		// store entry
	cmp	r4, r5			// done?
	add	r6, r6, #4		// next entry
	add	r4, r4, #0x1000		// next page
	blt	1b

	// fill out boot info
	load	r0, boot_info
	load	r3, kernel_page_table
	str	r3, [r0, #0]
	load	r3, kernel_leaf_tables
	str	r3, [r0, #4]

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
	load	r3, kernel_page_table
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
