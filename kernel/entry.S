/* entry.S - exception, interrupt and syscall entry point */
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

.section ".text"

.balign 32
.globl exception_table
exception_table:
	ldr	pc, addr_exception_reset
	ldr	pc, addr_exception_undefined
	ldr	pc, addr_exception_syscall
	ldr	pc, addr_exception_prefetch_abort
	ldr	pc, addr_exception_data_abort
	ldr	pc, addr_exception_reserved
	ldr	pc, addr_exception_irq
	ldr	pc, addr_exception_fiq

addr_exception_reset:		.word exception_reset
addr_exception_undefined:	.word exception_undefined
addr_exception_syscall:		.word exception_syscall
addr_exception_prefetch_abort:	.word exception_prefetch_abort
addr_exception_data_abort:	.word exception_data_abort
addr_exception_reserved:	.word exception_reserved
addr_exception_irq:		.word exception_irq
addr_exception_fiq:		.word exception_fiq
	
/* Save context before jumping to C code
 * At exit:
 * r0 - current Task
 * r1 - Task->regs[17] (CPSR)
 * r2 - CPSR
 * lr - return address
 */
.macro save, offset
	// Adjust LR and save it
	sub	lr, #\offset
	push	{lr}

	// load current Task->regs_ as lr
	mrc	p15, 0, lr, c13, c0, 4

	// save registers
	stmia	lr, {r0-r14}^
	add	r1, lr, #60 // 4byte * 15

	// store current Task for later
	mov	r0, lr

	// store user/sys PC (was in lr)
	pop	{lr}
	stmia	r1!, {lr}

	// store user/sys CPSR
	mrs	r2, spsr
	stmia	r1, {r2}
.endm

// Restore context before returning from exception/interrupt/syscall
.macro restore
	// load current Task->regs_ as lr
	mrc	p15, 0, lr, c13, c0, 4

	// load registers (leave PC and CPSR)
	ldmia	lr, {r0-r14}^
	add	lr, #60	// 4 * 15

	// return to whence we came from
	rfeia	lr
.endm

.balign 4
.globl switch_to_current_task
switch_to_current_task:
	mrs	r4, cpsr	// get current mode
	bic	r4, r4, #0x1f	// blank mode
	orr	r5, r4, #0x13	// svc mode
	msr	cpsr_c, r5
	restore

.globl exception_reset
exception_reset:
	save	4
	bl	exception_reset_handler
	// no way to return from reset
	// restore
	b	panic

.globl exception_undefined
exception_undefined:
	save	4
	bl	exception_undefined_handler
	restore

.globl exception_syscall
exception_syscall:
	save	0
	// lookup opcode of syscall
	ldr	r3, [lr, #-4]
	// restore arg0 - arg2
	mov	r4, r0
	ldmia	r4, {r0-r2}
	bl	exception_syscall_handler
	// store result
	stmia	r4, {r0}
	// schedule after every syscall for now
	// bl	schedule
	restore

.globl exception_prefetch_abort
exception_prefetch_abort:
	save	4
	bl	exception_prefetch_abort_handler
	restore

.globl exception_data_abort
exception_data_abort:
	save	8
	bl	exception_data_abort_handler
	restore

.globl	exception_reserved
exception_reserved:
	b	panic

.globl	exception_irq
exception_irq:
	save	4
	bl	exception_irq_handler
	restore

.globl	exception_fiq
exception_fiq:
	save	4
	bl	exception_fiq_handler
	restore

// Syscall stubs
.globl syscall
syscall:
	push	{lr}
	swi	#0
	pop	{lr}
	bx	lr

// set stack for mode
// void set_mode_stack(Mode mode, uint32_t *stack_top);
.globl set_mode_stack
set_mode_stack:
	mrs	r2, cpsr	// get old mode
        bic	r3, r2, #0xdf	// mask out mode bits and disable interrupts
	orr	r3, r3, r0	// set new mode bits
	msr	cpsr_c, r3	// switch mode
        mov	sp, r1		// load mode stack
        msr	cpsr_c, r2	// switch mode back
	bx	lr

