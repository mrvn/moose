/* timer.cc - hardware timer */
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

/* Reference material:
 * http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 * Chapter 12: System Timer
 *
 * The system timer has a free running counter with a 1MHz clock and is
 * independent of the system clock (unlike the ARM timer). The timer as 4
 * match register that trigger an interrupt when they match the lower 32bit of
 * the counter. So it is basically a one shot timer but if not reset will
 * trigger again after 1 hour, 11 minutes and 39.967296 seconds (2^32 ticks).
 *
 * As a result of that system time will be kept in nano seconds. The counter
 * can not be set so the Wall clock time will be kept as offset to the
 * counter.
 */

#include <stdint.h>
#include <memory.h>
#include <irq.h>
#include <uart.h>
#include <task.h>

#include <timer.h>

void * operator new(size_t, void *p) { return p; }

namespace Timer {
    enum {
	BASE = 0x3000,

	TIMER_CS  = 0x00,
	TIMER_CLO = 0x04,
	TIMER_CHI = 0x08,
	TIMER_C0  = 0x0C,
	TIMER_C1  = 0x10,
	TIMER_C2  = 0x14,
	TIMER_C3  = 0x18,
	SIZE      = 0x1C,

	FREQUENCY = 1000000,
    };

    // Control and Status register - read for status, write for control
    enum {
	CTRL_BIT_MATCH0, CTRL_BIT_MATCH1, CTRL_BIT_MATCH2, CTRL_BIT_MATCH3
    };
    enum Ctrl_Flags {
	CTRL_MATCH0 = 1 << CTRL_BIT_MATCH0, // GPU used
	CTRL_MATCH1 = 1 << CTRL_BIT_MATCH1,
	CTRL_MATCH2 = 1 << CTRL_BIT_MATCH2, // GPU used
	CTRL_MATCH3 = 1 << CTRL_BIT_MATCH3,
    };

    Timer::Timer(const char *name__, Task::Task *owner__, uint64_t wakeup_time,
		 callback_fn callback__, void *data__)
	: Item(wakeup_time), name_(name__),
	  owner_(owner__), callback_(callback__), data_(data__) { }

    Heap::Heap<Timer> *heap;
    Memory::Peripheral timer;
    
    /*
     * get system time
     * returns: time in 1/1000000th of seconds since power on.
     */
    uint64_t system_time() {
	// FIXME: move to timer.h?
	return (((uint64_t)timer.get(TIMER_CHI)) << 32) | timer.get(TIMER_CLO);
    }

    bool in_irq_handler = false;
    void irq_handler(void *data) {
	// avoid recursion from setting new times from callbacks
	if (in_irq_handler) return;
	in_irq_handler = true;
	UART::puts(__PRETTY_FUNCTION__);
	UART::puts("\n Status = ");
	UART::put_uint32(timer.get(TIMER_CS));
	UART::puts("\n Count  = ");
	UART::put_uint32(timer.get(TIMER_CHI));
	UART::putc(' ');
	UART::put_uint32(timer.get(TIMER_CLO));
	UART::puts("\n CMP1   = ");
	UART::put_uint32(timer.get(TIMER_C1));
	UART::puts("\n CMP2   = ");
	UART::put_uint32(timer.get(TIMER_C3));

	int num = (int)data;
	// process timer
	if (num == 1) {
	    UART::puts("\n timer 1 expired\n");
	    uint64_t next;
	again:
	    uint64_t now = system_time();
	    if (heap->empty()) {
		next = now + 0xFFFFFFFF;
		// FIXME: disable timer interrupt?
		// Option: Insert dummy timer so this never happens?
	    } else {
		next = heap->peek()->priority();
	    }
	    UART::puts(" next = ");
	    UART::put_uint32(next >> 32);
	    UART::putc(' ');
	    UART::put_uint32(next);
	    UART::putc('\n');
	    while((int64_t)(next - now) <= 0) {
		Timer *t = (Timer*)heap->pop();
		t->callback();
		UART::puts("@ after callback\n");
		if (heap->empty()) {
		    UART::puts("@ heap empty\n");
		    next = now + 0xFFFFFFFF;
		    // FIXME: disable timer interrupt?
		    // Option: Insert dummy timer so this never happens?
		} else {
		    next = heap->peek()->priority();
		    UART::puts("@ next = ");
		    UART::put_uint32(next);
		    UART::putc('\n');
		}
	    }
	    timer.set(TIMER_C1, next);
	    timer.set(TIMER_CS, CTRL_MATCH1);
	    // make sure we didn't just miss the time
	    if (((int64_t)(next - system_time())) <= 0) {
		UART::puts("@ again\n");
		goto again;
	    }
	    UART::puts("@ done\n");
	} else {
	    UART::puts("\n timer 2 expired\n");
	    // timer.set(TIMER_C3, timer2);
	    timer.set(TIMER_CS, CTRL_MATCH3);
	}
	UART::putc('\n');
	in_irq_handler = false;
    }

    /*
     * Initialize Timer.
     */
    void init(void) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	
	// allocate peripheral
	timer = Memory::alloc_peripheral(BASE, SIZE);
	if (!timer.valid()) {
	    panic("Timer::init(): peripheral not available\n");
	}

	// allocate heap
	void *mem = Memory::early_malloc(0);
	heap = new(mem)Heap::Heap<Timer>();
	
	UART::puts(" Status = ");
	UART::put_uint32(timer.get(TIMER_CS));
	UART::puts("\n Count  = ");
	UART::put_uint32(timer.get(TIMER_CHI));
	UART::putc(' ');
	UART::put_uint32(timer.get(TIMER_CLO));
	UART::puts("\n CMP1   = ");
	UART::put_uint32(timer.get(TIMER_C1));
	UART::puts("\n CMP2   = ");
	UART::put_uint32(timer.get(TIMER_C3));
	UART::putc('\n');

	/*
	// set up some timeouts
	mem = Memory::early_malloc(0);
	Timer *timer1 =
	    new(mem)Timer("Timer 5", NULL, 0, increment, (void*)0x500000);
	mem = Memory::early_malloc(0);
	Timer *timer2 =
	    new(mem)Timer("Timer 7", NULL, 0, increment, (void*)0x700000);
	heap->push(timer1);
	heap->push(timer2);
	irq_handler((void*)1);
	UART::puts(" timer set\n");
	*/

	// clear match bits in case they were alredy triggered
	timer.set(TIMER_CS, CTRL_MATCH1 | CTRL_MATCH3);
	UART::puts(" timer match cleared\n");

	IRQ::set_irq_handler(IRQ::IRQ_TIMER1, irq_handler, (void *)1);
	UART::puts(" irq handler 1 active\n");
	IRQ::set_irq_handler(IRQ::IRQ_TIMER3, irq_handler, (void *)3);
	UART::puts(" irq handler 3 active\n");

	/*
	while(true) {
	    for(volatile int i = 0; i < 0x2000000; ++i) { }
	    UART::puts("\n Status = ");
	    UART::put_uint32(timer.get(TIMER_CS));
	    UART::puts("\n Count  = ");
	    UART::put_uint32(timer.get(TIMER_CHI));
	    UART::putc(' ');
	    UART::put_uint32(timer.get(TIMER_CLO));
	    UART::puts("\n CMP1   = ");
	    UART::put_uint32(timer.get(TIMER_C1));
	    UART::puts("\n CMP2   = ");
	    UART::put_uint32(timer.get(TIMER_C3));
	    UART::putc('\n');
	    IRQ::debug();
	}
	*/
    }

    /*
     * set timer
     * timer:       timer to set
     * wakeup_time: time when timer expires
     */
    void set_timer(Timer *t, uint64_t wakeup_time) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	UART::put_uint32(wakeup_time >> 32);
	UART::putc(' ');
	UART::put_uint32(wakeup_time);
	UART::putc('\n');
	heap->set_priority(t, wakeup_time);
	// reset timer
	irq_handler((void*)1);
    }
}
