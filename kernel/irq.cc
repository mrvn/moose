/* irq.cc - Interrupt requests management */
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

#include <panic.h>
#include <stdint.h>
#include <uart.h>
#include <memory.h>

#include <entry.h>
#include <irq.h>

namespace IRQ {
    enum {
	BASE = 0xB200,

	PENDING = 0x00,
	PENDING1 = 0x04,
	PENDING2 = 0x08,
	FIQ_CTRL = 0x0C,
	ENABLE1  = 0x10,
	ENABLE2  = 0x14,
	ENABLE   = 0x18,
	DISABLE1 = 0x1C,
	DISABLE2 = 0x20,
	DISABLE  = 0x24,
	SIZE     = 0x28,
    };
    
    extern "C" {
        // called from entry.S. Declaring it extern "C" avoid
        // having to deal with the C++ name mangling.
        void exception_reset_handler();
        void exception_undefined_handler();
        uint32_t exception_syscall_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t opcode);
        void exception_prefetch_abort_handler();
        void exception_data_abort_handler();
        void exception_reserved_handler();
	void exception_irq_handler();
        void exception_fiq_handler();
	extern uint32_t exception_table[];
    }

    struct IRQData {
	handler_t handler;
	void *data;
	uint32_t count;
    };
    IRQData handler[NUM_IRQS];

    Memory::Peripheral irq;
    
    void default_handler(void *data) {
	uint32_t num = (uint32_t)data;
	// FIXME: report IRQ number being called
	panic("Unhandled IRQ being called!");
	(void)num;
    }

    void set_irq_handler(IRQ_BIT num, handler_t fn, void *data) {
	if (handler[num].handler != default_handler) {
	    panic("Irq already taken!");
	}
	handler[num].handler = fn;
	handler[num].data = data;
    }

    // enable interrupts
    void enable_irq(void) {
        uint32_t t;
        asm volatile("mrs %[t],cpsr; bic %[t], %[t], #0x80; msr cpsr_c, %[t]"
                     : [t]"=r"(t));
    }

    // disable interrupts
    void disable_irq(void) {
        uint32_t t;
        asm volatile("mrs %[t],cpsr; orr %[t], %[t], #0x80; msr cpsr_c, %[t]"
                     : [t]"=r"(t));
    }

    void init() {
	// allocate peripheral
	irq = Memory::alloc_peripheral(BASE, SIZE);
	if (!irq.valid()) {
	    panic("IQR::init(): peripheral not available\n");
	}
	
	// initialized interrupt handlers
	for(int i = 0; i < NUM_IRQS; ++i) {
	    handler[i].handler = default_handler;
	    handler[i].data = (void*)i;
	    handler[i].count = 0;
	}

        // allocate stack for IRQ mode
        uint32_t *stack = (uint32_t*)Memory::early_malloc(0); // 4k stack
        set_mode_stack(MODE_IRQ, &stack[1024]);

	// set exception vector base address register
	asm volatile("mcr p15, 0, %[addr], c12, c0, 0"
                     : : [addr]"r"(exception_table));

	// mask all interrupts
	irq.set(DISABLE1, 0xFFFFFFFF);
	irq.set(DISABLE2, 0xFFFFFFFF);
	irq.set(DISABLE,  0xFF);

	// no IRQ can happen until some get unmasked but the kernel is ready
	// for them
	enable_irq();
    }

    void exception_reset_handler() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	panic(__PRETTY_FUNCTION__);
    }

    void exception_undefined_handler() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	panic(__PRETTY_FUNCTION__);
    }

    void exception_prefetch_abort_handler() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	panic(__PRETTY_FUNCTION__);
    }

    void exception_data_abort_handler() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	panic(__PRETTY_FUNCTION__);
    }

    void exception_reserved_handler() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	panic(__PRETTY_FUNCTION__);
    }
    
    void exception_irq_handler() {
        uint32_t pending1 = irq.get(PENDING1);
        uint32_t pending2 = irq.get(PENDING2);
        uint32_t pending  = irq.get(PENDING);
	
        // debug all interrupts
	UART::puts( __PRETTY_FUNCTION__);
	UART::puts("\nIRQ1 pending: ");
	UART::put_uint32(pending1);
	UART::puts("\nIRQ2 pending: ");
	UART::put_uint32(pending2);
	UART::puts("\nIRQ  pending: ");
	UART::put_uint32(pending);
	UART::putc('\n');

	pending &= MASK_BASIC_IRQ;
	
	while(pending1) {
            int num = __builtin_ffs(pending1) - 1;
            handler[num].handler(handler[num].data);
            pending1 = pending1 & ~(1LU << num);
        }
	while(pending2) {
            int num = __builtin_ffs(pending2) - 1;
            handler[num + 32].handler(handler[num + 32].data);
            pending2 = pending2 & ~(1LU << num);
        }
	while(pending) {
            int num = __builtin_ffs(pending) - 1;
            handler[num + 64].handler(handler[num + 64].data);
            pending = pending & ~(1LU << num);
        }
    }

    void exception_fiq_handler() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	panic(__PRETTY_FUNCTION__);
    }
}
