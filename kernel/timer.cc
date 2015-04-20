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

/* System Timer
 */

#include "timer.h"
#include "arch_info.h"
#include "kprintf.h"
#include "irq.h"
#include "led.h"
#include "peripherals.h"

__BEGIN_NAMESPACE(Kernel);
__BEGIN_NAMESPACE(Timer);

enum TIMER_Reg {
    TIMER_CS  = 0x00, // 0x??003000 System Timer Control/Status
    TIMER_CLO = 0x04, // 0x??003004 System Timer Lower 32 bits
    TIMER_CHI = 0x08, // 0x??003008 System Timer Higher 32 bits
    TIMER_C0  = 0x0C, // 0x??00300C System Timer Compare 0
    TIMER_C1  = 0x10, // 0x??003010 System Timer Compare 1
    TIMER_C2  = 0x14, // 0x??003014 System Timer Compare 2
    TIMER_C3  = 0x18, // 0x??003018 System Timer Compare 3
};

static volatile uint32_t * TIMER_reg(enum TIMER_Reg reg) {
    return (volatile uint32_t *)(KERNEL_TIMER + reg);
}

static uint32_t status(PeripheralLock *prev) {
    uint32_t res;
    PERIPHERAL_ENTER(lock, prev, TIMER_BASE);

    volatile uint32_t *ctrl = TIMER_reg(TIMER_CS);
    res = *ctrl & 0x7;
    
    PERIPHERAL_LEAVE(lock);
    return res;
}

static void ctrl_set(PeripheralLock *prev, uint32_t t) {
    PERIPHERAL_ENTER(lock, prev, TIMER_BASE);

    volatile uint32_t *ctrl = TIMER_reg(TIMER_CS);
    *ctrl |= t;

    PERIPHERAL_LEAVE(lock);
}

uint64_t count(PeripheralLock *prev) {
    uint64_t res;
    PERIPHERAL_ENTER(lock, prev, TIMER_BASE);

    volatile uint32_t *hi = TIMER_reg(TIMER_CHI);
    volatile uint32_t *lo = TIMER_reg(TIMER_CLO);
    res = (((uint64_t)*hi) << 32) | *lo;

    PERIPHERAL_LEAVE(lock);
    return res;
}

static uint32_t lowcount(PeripheralLock *prev) {
    uint32_t res;
    PERIPHERAL_ENTER(lock, prev, TIMER_BASE);

    volatile uint32_t *lo = TIMER_reg(TIMER_CLO);
    res = *lo;
    
    PERIPHERAL_LEAVE(lock);
    return res;
}

static uint32_t cmp(PeripheralLock *prev, uint32_t num) {
    uint32_t res;
    PERIPHERAL_ENTER(lock, prev, TIMER_BASE);

    enum TIMER_Reg reg =
	(num < 2) ? ((num < 1) ? TIMER_C0 : TIMER_C1)
	          : ((num < 3) ? TIMER_C2 : TIMER_C3);
    res = *TIMER_reg(reg);

    PERIPHERAL_LEAVE(lock);
    return res;
}

static void set_cmp(PeripheralLock *prev, uint32_t num, uint32_t t) {
    PERIPHERAL_ENTER(lock, prev, TIMER_BASE);

    enum TIMER_Reg reg =
	(num < 2) ? ((num < 1) ? TIMER_C0 : TIMER_C1)
	          : ((num < 3) ? TIMER_C2 : TIMER_C3);
    volatile uint32_t *cmp = TIMER_reg(reg);
    *cmp = t;

    PERIPHERAL_LEAVE(lock);
}

void busy_wait(uint32_t usec) {
    PERIPHERAL_ENTER(lock, NULL, TIMER_BASE);
    uint32_t start = lowcount(&lock);
    uint32_t now = start;
    while (now - start <= usec) {
	now = lowcount(&lock);
    }
    PERIPHERAL_LEAVE(lock);
}

void test(void) {
    PERIPHERAL_ENTER(lock, NULL, TIMER_BASE);

    uint64_t t = count(&lock);
    kprintf("timer CS    = %#lx\n", status(&lock));
    kprintf("timer count = %#18llx\n", t);
    kprintf("timer cmp 0 = %#10lx\n", cmp(&lock, 0));
    kprintf("timer cmp 1 = %#10lx\n", cmp(&lock, 1));
    kprintf("timer cmp 2 = %#10lx\n", cmp(&lock, 2));
    kprintf("timer cmp 3 = %#10lx\n", cmp(&lock, 3));
    kprintf("\n");
    
    // trigger on the second next second (at least one second from now)
    set_cmp(&lock, 1, t / 1000000 * 1000000 + 2000000);
    
    // clear pending bit and enable irq
    ctrl_set(&lock, 1U << 1);
    IRQ::enable_irq(&lock, IRQ::IRQ_TIMER1);
    IRQ::enable_irqs();

    while (1) {
	// chill out
	asm volatile ("wfi");
    }

    PERIPHERAL_LEAVE(lock);
}

void handle_timer1(PeripheralLock *prev) {
    PERIPHERAL_ENTER(lock, prev, TIMER_BASE);

    uint64_t t = count(&lock);
    kprintf("timer CS    = %lu\n", status(&lock));
    kprintf("timer count = %#18llx\n", t);
    kprintf("timer cmp 0 = %#10lx\n", cmp(&lock, 0));
    kprintf("timer cmp 1 = %#10lx\n", cmp(&lock, 1));
    kprintf("timer cmp 2 = %#10lx\n", cmp(&lock, 2));
    kprintf("timer cmp 3 = %#10lx\n", cmp(&lock, 3));

    uint32_t frac, seconds, minutes, hours, next;
    frac = t % 1000000;
    t /= 1000000;
    next = t * 1000000 + 1000000;
    seconds = t % 60;
    t /= 60;
    minutes = t % 60;
    t /= 60;
    hours = t;
    kprintf("time = %lu:%02lu:%02lu.%06lu\n", hours, minutes, seconds, frac);
    kprintf("\n");
    
    // clear pending bit
    ctrl_set(&lock, 1U << 1);
    // trigger one the next second mark
    set_cmp(&lock, 1, next);

    // toggle leds
    switch(seconds % 4) {
    case 0:
	LED::set(&lock, LED::LED_ACT, true);
	break;
    case 1:
	LED::set(&lock, LED::LED_PWR, true);
	break;
    case 2:
	LED::set(&lock, LED::LED_ACT, false);
	break;
    case 3:
	LED::set(&lock, LED::LED_PWR, false);
	break;
    }

    PERIPHERAL_LEAVE(lock);
}

__END_NAMESPACE(Timer);
__END_NAMESPACE(Kernel);
