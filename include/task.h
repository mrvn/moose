/* task.h - Task management and scheduling */
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

#ifndef MOOSE_KERNEL_TASK_H
#define MOOSE_KERNEL_TASK_H

#include <stdint.h>
#include <list.h>
#include <timer.h>

namespace Task {
    enum State { RUNNING, SLEEPING };
    enum {
	REG_R0, // arg0 / ret0
	REG_R1, // arg1 / ret1
	REG_R2, // arg2
	REG_R3, // arg3
	REG_R4,
	REG_R5,
	REG_R6,
	REG_R7,
	REG_R8,
	REG_R9,
	REG_R10,
	REG_R11,
	REG_R12, // scratch
	REG_SP,
	REG_LR, // return address
	REG_PC,
	REG_CPSR,

	NUM_STATES = SLEEPING + 1,
	NUM_REGS = 17,
    };

    /*
     * Thread ID registers
     */
    static inline uint32_t read_kernel_thread_id() {
        uint32_t id;
        asm volatile("mrc p15, 0, %[id], c13, c0, 4"
                     : [id]"=r"(id));
        return id;
    }

    static inline void write_kernel_thread_id(uint32_t id) {
        asm volatile("mcr p15, 0, %[id], c13, c0, 4"
                     : : [id]"r"(id));
    }

    static inline uint32_t read_ro_thread_id() {
        uint32_t id;
        asm volatile("mrc p15, 0, %[id], c13, c0, 3"
                     : [id]"=r"(id));
        return id;
    }

    static inline void write_ro_thread_id(uint32_t id) {
        asm volatile("mcr p15, 0, %[id], c13, c0, 3"
                     : : [id]"r"(id));
    }

    static inline uint32_t read_rw_thread_id() {
        uint32_t id;
        asm volatile("mrc p15, 0, %[id], c13, c0, 2"
                     : [id]"=r"(id));
        return id;
    }

    static inline void write_rw_thread_id(uint32_t id) {
        asm volatile("mcr p15, 0, %[id], c13, c0, 2"
                     : : [id]"r"(id));
    }

    // Task structure
    typedef void (*start_fn)();
    class Task {
    public:
	Task(const char *name__, start_fn start, State state__ = RUNNING,
	     Timer::Timer::callback_fn timer_callback = timer_callback);
	~Task();
	void * operator new(size_t);
	void operator delete(void *ptr);
	const char *name() const { return name_; }
	static void schedule(State new_state);
	void die();
	static Task * current() { return (Task*)read_kernel_thread_id(); }
	Timer::Timer *timer() { return &timer_; }
	void wakeup();
    private:
	uint32_t regs_[NUM_REGS];
	const char *name_;
	State state_;
	List::DList all_list_;
	List::DList state_list_;
	Timer::Timer timer_;
	uint64_t start_;
	uint32_t remaining_;
	
	void fix_kernel_stack();
	friend void init();
	static void timer_callback(Timer::Timer *timer, void *data);
	static Task * deactivate(uint64_t now);
	void activate(uint64_t now);
    };

    /*
     * Initialize task subsystem
     */
    void init();

    /*
     * put current task to sleep
     * time: nanoseconds to sleep
     */
    uint32_t sys_sleep(int64_t time);
}

#endif // #ifndef MOOSE_KERNEL_TASK_H
