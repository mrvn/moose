/* syscall.cc - Syscall interface */
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
#include <task.h>

#include <syscall.h>

namespace Syscall {
    extern "C" {
        // called from entry.S. Declaring it extern "C" avoid
        // having to deal with the C++ name mangling.
        uint32_t exception_syscall_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t opcode);
    }

    void init() {
	// allocate stack for SVC mode
        uint32_t *stack = (uint32_t*)Memory::early_malloc(0); // 4k stack
        set_mode_stack(MODE_SVC, &stack[1024]);
    }

    union Arg64 {
	uint32_t u[2];
	int64_t i;
    };

    uint32_t exception_syscall_handler(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t opcode) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	UART::puts("  R0 = ");
	UART::put_uint32(r0);
	UART::puts(", R1 = ");
	UART::put_uint32(r1);
	UART::puts(", R2 = ");
	UART::put_uint32(r2);
	UART::puts(", opcode = ");
	UART::put_uint32(opcode);
	UART::putc('\n');
	switch(opcode) {
	case 0xEF000000: { // sleep
	    union {
		uint32_t u[2];
		int64_t i;
	    } arg;
	    arg.u[0] = r0;
	    arg.u[1] = r1;	    
	    return Task::sys_sleep(arg.i);
	}
	case 0xEF000001: { // create_thread
	    const char *name = (const char*)r0;
	    Task::start_fn start = (Task::start_fn)r1;
	    void *arg = (void*)r2;
	    UART::puts("  create_thread(");
	    UART::puts(name);
	    UART::puts(", ");
	    UART::put_uint32(r1);
	    UART::puts(", ");
	    UART::put_uint32(r2);
	    UART::puts(")\n");
	    Task::Task *task = Task::read_kernel_thread_id();
	    Message::MailboxId id = task->create_task(name, start, arg);
	    UART::puts("  id = ");
	    UART::put_uint32(id);
	    UART::putc('\n');
	    return id;
	}
	case 0xEF000002: { // end_thread
	    Task::Task::die();
	    return 0;
	}
	case 0xEF000003: { // send_message
	    Message::MailboxId id = r0;
	    Message::Message *msg = (Message::Message*)r1;
	    Task::Task *task = Task::read_kernel_thread_id();
	    task->send_message(id, msg);
	    return 0;
	}
	case 0xEF000004: { // recv_message
	    return (uint32_t)Task::sys_recv_message();
	}
	default:
	    // FIXME: illegal syscall
	    return -1;
	}
    }
}
