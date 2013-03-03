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
#include <timer.h>

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
    uint32_t x = 0;
    while(true) {
	UART::puts("########### ");
	UART::puts(__PRETTY_FUNCTION__);
	UART::put_uint32(x++);
	UART::putc('\n');
	Syscall::sleep(0x500000);
    }
}

void busy1() {
    uint32_t x = 0;
    while(true) {
	UART::puts("########### ");
	UART::puts(__PRETTY_FUNCTION__);
	UART::put_uint32(x++);
	UART::putc('\n');
	for(volatile int i = 0; i < 0x1000000; ++i) { }
    }
}

void busy2() {
    uint32_t x = 0;
    while(true) {
	UART::puts("########### ");
	UART::puts(__PRETTY_FUNCTION__);
	UART::put_uint32(x++);
	UART::putc('\n');
	for(volatile int i = 0; i < 0x1000000; ++i) { }
    }
}

void blink() {
    uint32_t x = 0;
    while(true) {
	UART::puts("########### ");
	UART::puts(__PRETTY_FUNCTION__);
	UART::put_uint32(x++);
	UART::putc('\n');
	GPIO::led();
	Syscall::sleep(0x300000);
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
    Timer::init();

    // Test syscall (starts preemptive multitasking)
    UART::puts("Calling syscall...\n");
    uint32_t res = Syscall::sleep(0x1000000);
    UART::puts("Syscall returned: ");
    UART::put_uint32(res);
    UART::putc('\n');
    for(volatile int i = 0; i < 0x20000000; ++i) { }
    
    // Test multitasking
    Task::Task *task = new Task::Task("<WAIT>", wait, Task::RUNNING);
    task = new Task::Task("<BLINK>", blink, Task::RUNNING);
    task = new Task::Task("<BUSY1>", busy1, Task::RUNNING);
    task = new Task::Task("<BUSY2>", busy2, Task::RUNNING);
    UNUSED(task);

    // we are done, let the other tasks run
    UART::puts("exiting kernel task\n");
    Task::Task *self = Task::Task::current();
    // FIXME: make this a syscall before freeing the stack in die()
    self->die();
    switch_to_current_task();

    // should never reach this
    while(true) { }

    // should never reach this
    panic("foo");
}
