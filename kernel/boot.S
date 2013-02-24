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

	// Setup the stack.
	ldr	sp, =0x8000

	// call kernel_main
	bl	kernel_main

	// Blink OK LED
	.globl panic
panic:
	// Base of GPIO registers
	ldr	r4, =0x20200000

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
