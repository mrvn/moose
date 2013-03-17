/* syscall.h - syscall interface */
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

#ifndef MOOSE_KERNEL_SYSCALL_H
#define MOOSE_KERNEL_SYSCALL_H

#include <stdint.h>
#include <message.h>
#include <task.h>

namespace Syscall {
    void init();

    static inline uint32_t sleep(int64_t time) {
	union {
	    uint32_t u[2];
	    int64_t i;
	} arg;
	arg.i = time;
	register uint32_t r0 asm("r0") = arg.u[0];
	register uint32_t r1 asm("r1") = arg.u[1];
	register uint32_t res asm("r0");
	asm volatile("swi #0" : "=r"(res) : "0"(r0), "r"(r1));
	return res;
    }

    static inline Message::MailboxId create_thread(const char *name,
						   Task::start_fn start,
						   void *arg) {
	register const char *r0 asm("r0") = name;
	register Task::start_fn r1 asm("r1") = start;
	register void *r2 asm("r2") = arg;
	register Message::MailboxId res asm("r0");
	asm volatile("swi #1" : "=r"(res) : "0"(r0), "r"(r1), "r"(r2));
	return res;
    }

    static inline void end_thread() {
	asm volatile("swi #2");
    }

    static inline void send_message(Message::MailboxId id, Message::Message *msg) {
	register Message::MailboxId r0 asm("r0") = id;
	register Message::Message *r1 asm("r1") = msg;
	asm volatile("swi #3" : : "r"(r0), "r"(r1));
    }

    static inline Message::Message * recv_message() {
	register Message::Message * res asm("r0");
	asm volatile("swi #4" : "=r"(res));
	return res;
    }
}

#endif // #ifndef MOOSE_KERNEL_SYSCALL_H
