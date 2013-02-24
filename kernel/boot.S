/* boot.S - assembly startup code */
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

// Mapping between virtual and physical memory
#define VIRT_TO_PHYS 0xC0000000

// To keep this in the first portion of the binary.
.section ".text.boot"

.globl Start
Start:
// Entry point for the kernel
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main

	/****************************************************************
	 * Map kernel to 0xC0000000 and enable paging			*
	 ****************************************************************/

	// Translation Table Base Register 0
	ldr	r4, =kernel_pagetable - VIRT_TO_PHYS
	mcr	p15, 0, r4, c2, c0, 0 // Write TTBR0

	// Translation Table Base Register 1
	ldr	r4, =kernel_pagetable - VIRT_TO_PHYS
	mcr	p15, 0, r4, c2, c0, 1 // Write TTBR1

	// c2, Translation Table Base Control Register
	ldr	r4, =2 // 4K Table 0
	mcr	p15, 0, r4, c2, c0, 2 // Write TTBCR

	// setup domains (CP15 c3)
	ldr	r4, =0x55555555 // use access permissions from TLB entry
	mcr	p15, 0, r4, c3, c0, 0 // Write Domain Access Control Register	

	// c1 control register
	// 29: Force AP disabled (0)
	// 28: TEX remap disabled (0)
	// 23: Subpages AP bits disabled (1)
	// 13: vector base address for exceptions selected (0)
	// 12: enable L1 instruction cache (1)
	// 11: enable branch prediction (1)
	//  9: rom protection off (0)
	//  8: S bit off (0)
	//  2: enable data cache (1)
	//  1: strict alignment (1)
	//  0: enable MMU (1)
	ldr	r5, =0xCF7FCBF8
	ldr	r6, =0x00801807
	mrc	p15, 0, r4, c1, c0, 0 // Read Control Register
	and	r4, r4, r5
	orr	r4, r4, r6
	mcr	p15, 0, r4, c1, c0, 0 // Write Control Register

	/****************************************************************
	 * Switch to higher half (0xC0000000)				*
	 ****************************************************************/
	ldr	pc, =higher_half

higher_half:
	// run in system mode from here on
	mrs	r4, cpsr	// get current mode
	bic	r4, r4, #0x1f	// blank mode
	orr	r5, r4, #0x1f	// sys mode
	msr	cpsr_c, r5

	// Setup the stack.
	ldr	sp, =(VIRT_TO_PHYS + 0x8000)

	// Clear out bss.
	ldr	r4, =_bss_start
	ldr	r9, =_bss_end
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
1:	// store multiple at r4.
	stmia	r4!, {r5-r8}
	// If we're still below bss_end, loop.
	cmp	r4, r9
	blo	1b

	/****************************************************************
	 * Turn on OK LED						*
	 ****************************************************************/
	// Base of GPIO registers
	ldr	r4, =0xE0200000
	// enable pin 16 as output
	ldr	r6, [r4, #4]
	mov	r5, #1
	lsl	r5, #18
	orr	r6, r6, r5
	str	r6, [r4, #4]
	// turn on LED
	mov	r5, #1
	lsl	r5, #16
	str	r5, [r4, #40]

	/****************************************************************
         * Call constructors                                            *
         ****************************************************************/
	push    {r0-r2}
	ldr     r3, =kernel_constructors
	blx     r3
	pop     {r0-r2}

	/****************************************************************
	 * Call kernel_main						*
	 ****************************************************************/
	ldr	r3, =kernel_main
	blx	r3

	/****************************************************************
	 * Turn off OK LED						*
	 ****************************************************************/
	// Base of GPIO registers
	ldr	r4, =0xE0200000
	// enable pin 16 as output
	ldr	r6, [r4, #4]
	mov	r5, #1
	lsl	r5, #18
	orr	r6, r6, r5
	str	r6, [r4, #4]
	// turn off LED
	mov	r5, #1
	lsl	r5, #16
	str	r5, [r4, #28]

	// halt
halt:
	wfe
	b	halt


	/****************************************************************
	 * Blink OK LED							*
	 ****************************************************************/
	// Emergency signal in case something goes horribly wrong
	.globl panic
panic:
	.globl abort
abort:
	// Base of GPIO registers
	ldr	r4, =0xE0200000

	// enable pin 16 as output
	ldr	r6, [r4, #4]
	mov	r5, #1
	lsl	r5, #18
	orr	r6, r6, r5
	str	r6, [r4, #4]

2:
	// turn on LED
	mov	r5, #1
	lsl	r5, #16
	str	r5, [r4, #40]

	// wait
	mov	r6, #0x1000000
1:
	sub	r6, #1
	cmp	r6, #0
	bne	1b
	
	// turn off LED
	mov	r5, #1
	lsl	r5, #16
	str	r5, [r4, #28]

	// wait
	mov	r6, #0x1000000
1:
	sub	r6, #1
	cmp	r6, #0
	bne	1b

	// again
	b	2b
	
	
// store constants
constants:
.ltorg

.section ".data.pagetable"

	.balign 16384
	.global kernel_pagetable
kernel_pagetable:
	// addr(8) SBZ(4) NS 1 nG S | APX TEX(3) AP(2) P IGN(4) XN C B 1 0

	// identity map 1GB
	.fill	16, 4, 0x0004140E // 0x00000000 Outer and Inner Write-Back
	.fill	16, 4, 0x0104140E // TEX 001, C 1, B 1    Allocate on Write
	.fill	16, 4, 0x0204140E
	.fill	16, 4, 0x0304140E
	.fill	16, 4, 0x0404140E
	.fill	16, 4, 0x0504140E
	.fill	16, 4, 0x0604140E
	.fill	16, 4, 0x0704140E
	.fill	16, 4, 0x0804140E
	.fill	16, 4, 0x0904140E
	.fill	16, 4, 0x0A04140E
	.fill	16, 4, 0x0B04140E
	.fill	16, 4, 0x0C04140E
	.fill	16, 4, 0x0D04140E
	.fill	16, 4, 0x0E04140E
	.fill	16, 4, 0x0F04140E
	.fill	16, 4, 0x1004040A // 0x10000000 Outer and Inner Write-Through
	.fill	16, 4, 0x1104040A // TEX 000, C 1, B 0    No Allocate on Write
	.fill	16, 4, 0x1204040A
	.fill	16, 4, 0x1304040A
	.fill	16, 4, 0x1404040A
	.fill	16, 4, 0x1504040A
	.fill	16, 4, 0x1604040A
	.fill	16, 4, 0x1704040A
	.fill	16, 4, 0x1804040A
	.fill	16, 4, 0x1904040A
	.fill	16, 4, 0x1A04040A
	.fill	16, 4, 0x1B04040A
	.fill	16, 4, 0x1C04040A
	.fill	16, 4, 0x1D04040A
	.fill	16, 4, 0x1E04040A
	.fill	16, 4, 0x1F04040A
	.fill	16, 4, 0x20040402 // 0x20000000 peripherals (memory = device)
	.fill	16, 4, 0x21040402 // TEX 000, C 0, B 0    not cached
	.fill	16, 4, 0x22040402
	.fill	16, 4, 0x23040402
	.fill	16, 4, 0x24040402
	.fill	16, 4, 0x25040402
	.fill	16, 4, 0x26040402
	.fill	16, 4, 0x27040402
	.fill	16, 4, 0x28040402
	.fill	16, 4, 0x29040402
	.fill	16, 4, 0x2A040402
	.fill	16, 4, 0x2B040402
	.fill	16, 4, 0x2C040402
	.fill	16, 4, 0x2D040402
	.fill	16, 4, 0x2E040402
	.fill	16, 4, 0x2F040402
	.fill	16, 4, 0x30040402
	.fill	16, 4, 0x31040402
	.fill	16, 4, 0x32040402
	.fill	16, 4, 0x33040402
	.fill	16, 4, 0x34040402
	.fill	16, 4, 0x35040402
	.fill	16, 4, 0x36040402
	.fill	16, 4, 0x37040402
	.fill	16, 4, 0x38040402
	.fill	16, 4, 0x39040402
	.fill	16, 4, 0x3A040402
	.fill	16, 4, 0x3B040402
	.fill	16, 4, 0x3C040402
	.fill	16, 4, 0x3D040402
	.fill	16, 4, 0x3E040402
	.fill	16, 4, 0x3F040402

	// 0x40000000 - 0xBFFFFFFF unmapped
	.fill   4*2048, 1, 0

	// 0xC0000000 - 0xFFFFFFFF linear map to 0 - 1GB
	.fill	16, 4, 0x0004140E // 0xC0000000 Outer and Inner Write-Back
	.fill	16, 4, 0x0104140E // TEX 001, C 1, B 1    Allocate on Write
	.fill	16, 4, 0x0204140E
	.fill	16, 4, 0x0304140E
	.fill	16, 4, 0x0404140E
	.fill	16, 4, 0x0504140E
	.fill	16, 4, 0x0604140E
	.fill	16, 4, 0x0704140E
	.fill	16, 4, 0x0804140E
	.fill	16, 4, 0x0904140E
	.fill	16, 4, 0x0A04140E
	.fill	16, 4, 0x0B04140E
	.fill	16, 4, 0x0C04140E
	.fill	16, 4, 0x0D04140E
	.fill	16, 4, 0x0E04140E
	.fill	16, 4, 0x0F04140E
	.fill	16, 4, 0x1004040A // 0xD0000000 Outer and Inner Write-Through
	.fill	16, 4, 0x1104040A // TEX 000, C 1, B 0    No Allocate on Write
	.fill	16, 4, 0x1204040A
	.fill	16, 4, 0x1304040A
	.fill	16, 4, 0x1404040A
	.fill	16, 4, 0x1504040A
	.fill	16, 4, 0x1604040A
	.fill	16, 4, 0x1704040A
	.fill	16, 4, 0x1804040A
	.fill	16, 4, 0x1904040A
	.fill	16, 4, 0x1A04040A
	.fill	16, 4, 0x1B04040A
	.fill	16, 4, 0x1C04040A
	.fill	16, 4, 0x1D04040A
	.fill	16, 4, 0x1E04040A
	.fill	16, 4, 0x1F04040A
	.fill	16, 4, 0x20040402 // 0xE0000000 peripherals (memory = device)
	.fill	16, 4, 0x21040402 // TEX 000, C 0, B 0    not cached
	.fill	16, 4, 0x22040402
	.fill	16, 4, 0x23040402
	.fill	16, 4, 0x24040402
	.fill	16, 4, 0x25040402
	.fill	16, 4, 0x26040402
	.fill	16, 4, 0x27040402
	.fill	16, 4, 0x28040402
	.fill	16, 4, 0x29040402
	.fill	16, 4, 0x2A040402
	.fill	16, 4, 0x2B040402
	.fill	16, 4, 0x2C040402
	.fill	16, 4, 0x2D040402
	.fill	16, 4, 0x2E040402
	.fill	16, 4, 0x2F040402
	.fill	16, 4, 0x30040402
	.fill	16, 4, 0x31040402
	.fill	16, 4, 0x32040402
	.fill	16, 4, 0x33040402
	.fill	16, 4, 0x34040402
	.fill	16, 4, 0x35040402
	.fill	16, 4, 0x36040402
	.fill	16, 4, 0x37040402
	.fill	16, 4, 0x38040402
	.fill	16, 4, 0x39040402
	.fill	16, 4, 0x3A040402
	.fill	16, 4, 0x3B040402
	.fill	16, 4, 0x3C040402
	.fill	16, 4, 0x3D040402
	.fill	16, 4, 0x3E040402
	.fill	16, 4, 0x3F040402
