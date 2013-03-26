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
#include <message.h>
#include <task.h>

#include <syscall.h>

namespace Syscall {
    extern "C" {
        // called from entry.S. Declaring it extern "C" avoid
        // having to deal with the C++ name mangling.
        uint32_t syscall_sleep(uint64_t delay);
	Message::MailboxId syscall_create_task(const char *name,
					       Task::start_fn start,
					       void *arg,
					       Message::MailboxId stdio[3]);
	uint32_t syscall_end_task();
	uint32_t syscall_send_message(Message::MailboxId id,
				      Message::Message *msg);
	Message::Message * syscall_recv_message();
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

    uint32_t syscall_sleep(uint64_t delay) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	return Task::sys_sleep(delay);
    }

    Message::MailboxId syscall_create_task(const char *name,
					     Task::start_fn start,
					     void *arg,
					     Message::MailboxId stdio[3]) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	Task::Task *task = Task::read_kernel_thread_id();
	Message::MailboxId id = task->create_task(name, start, arg, stdio);
	return id;
    }

    uint32_t syscall_end_task() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	Task::Task::die();
	return 0;
    }

    uint32_t syscall_send_message(Message::MailboxId id,
				  Message::Message *msg) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	Task::Task *task = Task::read_kernel_thread_id();
	task->send_message(id, msg);
	return 0;
    }
    
    Message::Message * syscall_recv_message() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	return Task::sys_recv_message();
    }
}
