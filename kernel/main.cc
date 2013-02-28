/* main.cc - the entry point for the kernel */
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

#include <stdint.h>
#include <atags.h>
#include <memory.h>
#include <gpio.h>
#include <uart.h>
#include <list.h>
#include <task.h>
#include <irq.h>
#include <syscall.h>

extern "C" {
    // kernel_main gets called from boot.S. Declaring it extern "C" avoid
    // having to deal with the C++ name mangling.
    void kernel_main(uint32_t zero, uint32_t model, const ATAG::Header *atags);

    // constructors
    typedef void (*constructor_t)(void);
    extern constructor_t _init_array_start[];
    extern constructor_t _init_array_end[];
    void kernel_constructors(void);
    // from entry.S
    void switch_to_current_task(void);
}

#define UNUSED(x) (void)(x)

void kernel_constructors(void) {
    for(constructor_t *fn = _init_array_start; fn != _init_array_end; ++fn) {
	(*fn)();
    }
}

void wait() {
    while(true) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	for(volatile int i = 0; i < 0x10000000; ++i) { }
	syscall(0x23,0x42,0x666);
    }
}

void blink() {
    while(true) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	for(int j = 0; j < 16; ++j) {
	    GPIO::led();
	    for(volatile int i = 0; i < 0x1000000; ++i) { }
	}
	syscall(0x1,0x2,0x3);
    }
}

// kernel main function, it all begins here
void kernel_main(uint32_t zero, uint32_t model, const ATAG::Header *atags) {
    UNUSED(zero);
    UNUSED(model);
    Memory::init(atags);
    GPIO::init();
    UART::init();

    UART::puts("\nMOOSE V0.0\n");
    UART::puts("Memory used: ");
    UART::put_uint32(Memory::allocated());
    UART::puts("\nMemory free: ");
    UART::put_uint32(Memory::available());
    UART::putc('\n');

    Task::init();
    IRQ::init();
    Syscall::init();

    // Test syscall
    UART::puts("Calling syscall...\n");
    uint32_t res = syscall(0x01234567, 0x00112233, 0x01230123);
    UART::puts("Syscall returned: ");
    UART::put_uint32(res);
    UART::putc('\n');
    for(volatile int i = 0; i < 0x20000000; ++i) { }
    
    // Test cooperative multitasking via syscall
    Task::Task *task = new Task::Task("<WAIT>", wait, Task::RUNNING);
    task = new Task::Task("<BLINK>", blink, Task::RUNNING);
    UNUSED(task);

    // we are done, let the other tasks run
    UART::puts("exiting kernel task\n");
    Task::Task *self = Task::Task::current();
    // FIXME: make this a syscall before freeing the stack in die()
    self->die();
    switch_to_current_task();

    // should never reach this
    panic("foo");
}
