/* irq.h - Interrupt request management */
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

#ifndef MOOSE_KERNEL_IRQ_H
#define MOOSE_KERNEL_IRQ_H

/* The interrupts on the Raspberry Pi are split into 3 set of registers, each
 * with a enable, disable and pending register:
 * * IRQ1 - GPU IRQs  0 - 31
 * * IRQ2 - GPU IRQs 32 - 63
 * * IRQ  - ARM IRQs  0 -  7, IRQ1/2 pending bits, special mirrored GPU IRQs
 * The bits are mapped in that order to interrupt numbers 0 - 71 and the
 * remaining bits are ignored.
 */

namespace IRQ {
    enum IRQ_BIT {
	// GPU IRQs 0 - 63
	IRQ_TIMER1      = 1,
	IRQ_TIMER3      = 3,
	IRQ_AUX         = 29,
	IRQ_I2C_SPI_SLV = 43,
	IRQ_PWA0        = 45,
	IRQ_PWA1        = 46,
	IRQ_SMI         = 48,
	IRQ_GPIO0       = 49,
	IRQ_GPIO1       = 50,
	IRQ_GPIO2       = 51,
	IRQ_GPIO3       = 52,
	IRQ_I2C         = 53,
	IRQ_SPI         = 54,
	IRQ_PCM         = 55,
	IRQ_UART        = 57,
	IRQ_FB          = 62,
	// ARM interrupts
	IRQ_ARM_TIMER   = 64,
	IRQ_ARM_MAILBOX,
	IRQ_ARM_DOORBELL0,
	IRQ_ARM_DOORBELL1,
	IRQ_ARM_GPU0_HLT,
	IRQ_ARM_GPU1_HLT,
	IRQ_ARM_ILLEGAL_ACCESS_TYPE0,
	IRQ_ARM_ILLGEAL_ACCESS_TYPE1,
    };
    enum {
	NUM_IRQS = 72,
	MASK_BASIC_IRQ = 0xFF,
	
	// unused IRQ bits
	IRQ_REG1_PENDING = 72,
	IRQ_REG2_PENDING,
	IRQ_COPY_GPU7,
	IRQ_COPY_GPU9,
	IRQ_COPY_GPU10,
	IRQ_COPY_GPU18,
	IRQ_COPY_GPU19,
	IRQ_COPY_I2C,
	IRQ_COPY_SPI,
	IRQ_COPY_PCM,
	IRQ_COPY_GPU_56,
	IRQ_COPY_UART,
	IRQ_COPY_FB,
    };

    typedef void (*handler_t)(void *);
    
    void init();
    
    void set_irq_handler(IRQ_BIT num, handler_t handler, void *data);
}

#endif // #ifndef MOOSE_KERNEL_IRQ_H
