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

void blink(void *) {
    for(uint32_t x = 0; x < 10; ++x) {
	UART::puts("########### ");
	UART::puts(__PRETTY_FUNCTION__);
	UART::put_uint32(x);
	UART::putc('\n');
	GPIO::led();
	Syscall::sleep(0x100000);
    }
}

void pong(void *) {
    Message::Message *reply =
	(Message::Message*)Memory::early_malloc(Memory::PAGE_SIZE);
    while(true) {
	reply->data[0] = 'P';
	reply->data[1] = 'o';
	reply->data[2] = 'n';
	reply->data[3] = 'g';
	reply->data[4] = 0;
	reply->size = 5;
	UART::puts("### <PONG>: waiting for message\n");
	Message::Message *msg = Syscall::recv_message();
	if (msg == NULL) {
	    UART::puts("### <PONG>: Got no message!\n");
	    Syscall::sleep(0x1000000);
	} else {
	    UART::puts("### <PONG>: Got message: '");
	    UART::puts((const char*)msg->data);
	    UART::puts("'\n");
	    UART::puts("### <PONG>: Sending reply\n");
	    Syscall::send_message(msg->mailbox_id, reply);
	}
    }
}

void ping(void *) {
    Message::MailboxId pong_mailbox =
	Syscall::create_thread("<PONG>", pong, NULL);
    Message::Message *msg =
	(Message::Message*)Memory::early_malloc(Memory::PAGE_SIZE);
    while(true) {
	msg->data[0] = 'P';
	msg->data[1] = 'i';
	msg->data[2] = 'n';
	msg->data[3] = 'g';
	msg->data[4] = 0;
	msg->size = 5;
	UART::puts("### <PING>: sending ping...\n");
	Syscall::send_message(pong_mailbox, msg);
	UART::puts("### <PING>: waiting for reply...\n");
	Message::Message *reply = Syscall::recv_message();
	UART::puts("### <PING>: Got reply: '");
	UART::puts((const char*)reply->data);
	UART::puts("'\n----------------------------------------\n");
	Syscall::sleep(10000000);
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

    // First syscall starts multitasking, fitting that it is creating a task
    //Syscall::create_thread("<BLINK>", blink, NULL);
    Message::MailboxId ping_mailbox =
	Syscall::create_thread("<PING>", ping, nullptr);
    UNUSED(ping_mailbox);
    
    // we are done, let the other tasks run
    UART::puts("exiting kernel task\n");
    Syscall::end_thread();

    UART::puts("ERROR: should never reach this!");
    panic("ERROR: should never reach this!");
}
