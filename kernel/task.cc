/* task.cc - Task management and scheduling */
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

#include <memory.h>
#include <uart.h>
#include <syscall.h>

#include <task.h>

namespace Task {
    List::DList *all_head;
    List::DList *state_head[NUM_STATES];
    Task *idle;
    Task *sleepy;
    
    void idle_idles() {
	while(true) {
	    asm volatile("wfi");
	}
    }
    
    void sleepy_must_not_wake() {
	panic("Sleepy must not wake!");
    }
    
    Task::Task(const char *name__, start_fn start, State state__)
	: name_(name__), state_(state__), all_list_(List::DList()),
	  state_list_(List::DList()) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::puts(name_);
	UART::puts(state_ == RUNNING ? ", RUNNING\n" : ", SLEEPING\n");
	for(int i = 0; i < NUM_REGS; ++i) {
	    regs_[i] = 0xCAFEBABE;
	}
	// FIXME: allocate stack in user space
	regs_[REG_SP] =
	    ((uint32_t)Memory::early_malloc(2)) + Memory::PAGE_SIZE * 4;
	regs_[REG_PC] = (uint32_t)start;
	// set A I F SYS
	regs_[REG_CPSR] = 0b101011111;
	all_head->insert_before(&all_list_);
	state_head[state_]->insert_before(&state_list_);
    }

    Task::~Task() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	if (state_ == RUNNING) {
	    panic("Task::~Task(): still running");
	}
	all_list_.remove();
	state_list_.remove();
	// FIXME: free resources
    }
    
    void * Task::operator new(size_t) {
	UART::puts(__PRETTY_FUNCTION__);
	void *res = Memory::early_malloc(0);
	UART::put_uint32((uint32_t)res);
	UART::putc('\n');
	return res;
    }
    
    void Task::operator delete(void *addr) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	early_free(addr, Memory::PAGE_SIZE);
    }

    void Task::fix_kernel_stack() {
	// The kernel task already running on a stack when tasks are
	// initialized. Free the stack that was allocated in the constructor
	// and replace it with the stack being already used.
	Memory::early_free((void*)regs_[REG_SP], Memory::PAGE_SIZE * 4);
	regs_[REG_SP] = 0xC0008000;
    }

    void Task::schedule(State new_state) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	// reload RO thread register with next running task
	Task *task = (Task*)read_kernel_thread_id();
	UART::puts(task->name());
	UART::putc('\n');
	for(int i = 0; i < NUM_REGS; ++i) {
	    UART::put_uint32(task->regs_[i]);
	    if (i % 4 == 3) {
		UART::putc('\n');
	    } else {
		UART::putc(' ');
	    }
	}
	UART::puts("\n-> ");
	// get next running task
	size_t offset = (uint8_t*)task - (uint8_t*)&(task->state_list_);
	uint32_t member = (uint32_t)(task->state_list_.next());
	// update state of old task
	if (task->state_ != new_state) {
	    if (task == idle) {
		panic("Task::schedule(): idle task must not change state!");
	    }
	    task->state_list_.remove();
	    task->state_ = new_state;
	    state_head[new_state]->insert_before(&task->state_list_);
	}
	// skip over idle task (does not skip if it is the only one)
	task = (Task*)(member - - offset);
	if (task == idle) {
	    member = (uint32_t)(task->state_list_.next());
	    task = (Task*)(member - - offset);
	}
	// update RO thread register, return from interrupt will switch task
	UART::puts(task->name());
	UART::putc('\n');
	for(int i = 0; i < NUM_REGS; ++i) {
	    UART::put_uint32(task->regs_[i]);
	    if (i % 4 == 3) {
		UART::putc('\n');
	    } else {
		UART::putc(' ');
	    }
	}
	UART::putc('\n');
	write_kernel_thread_id((uint32_t)task);
    }

    // remove the current task from the scheduler and remove it
    void Task::die() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	// select a new task
	// change to SLEEPING to prevent geting picked again
	schedule(SLEEPING);
	delete this;
    }
    
    void init() {
	// create dummy heads so Task::Task() can function
	List::DList all_dummy, running_dummy, sleeping_dummy;
	all_head = &all_dummy;
	state_head[RUNNING] = &running_dummy;
	state_head[SLEEPING] = &sleeping_dummy;
	// create core tasks
	idle = new Task("<IDLE>", idle_idles, RUNNING);
	sleepy = new Task("<SLEEPY>", sleepy_must_not_wake, SLEEPING);
	if (idle == NULL || sleepy == NULL) {
	    panic("Out of memory allocating core tasks!");
	}
	// correct heads
	all_head = &idle->all_list_;
	state_head[RUNNING] = &idle->state_list_;
	state_head[SLEEPING] = &sleepy->state_list_;
	// cleanup dummies
	all_dummy.remove();
	running_dummy.remove();
	sleeping_dummy.remove();

	// Create kernel thread
	Task *kernel = new Task("<MOOSE>", NULL, RUNNING);
	kernel->fix_kernel_stack();

    	// load thread register
	write_kernel_thread_id((uint32_t)kernel);
    }

    extern "C" {
	void schedule() {
	    Task::schedule(RUNNING);
	}
    }
}

