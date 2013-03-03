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
    enum {
//	TIME_SLICE = 0x100000, // schedule every 1.048576 seconds
	TIME_SLICE = 0x800000, // schedule every ~8 seconds
    };
    
    List::DList *all_head;
    List::DList *state_head[NUM_STATES];
    Task *idle;
    Task *sleepy;
    uint64_t end_of_slice;

    void idle_idles() {
	while(true) {
	    asm volatile("wfi");
	}
    }
    
    void sleepy_must_not_wake() {
	panic("Sleepy must not wake!");
    }

    void schedule_callback(Timer::Timer *, void *) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	Task::schedule(RUNNING);
    }

    void Task::timer_callback(Timer::Timer *timer, void *) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	timer->owner()->wakeup();
    }
    
    Task::Task(const char *name__, start_fn start, State state__,
	       Timer::Timer::callback_fn timer_callback__)
	: name_(name__), state_(state__), all_list_(List::DList()),
	  state_list_(List::DList()),
	  timer_(name__, this, 0, timer_callback__, this),
	  start_(0), remaining_(0) {
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

    // Account for the time spend on the current task and return it
    Task * Task::deactivate(uint64_t now) {
	// get current running task
	Task *task = (Task*)read_kernel_thread_id();
	// subtract used time
	int32_t used = now - task->start_;
	int32_t remaining = task->remaining_ - used;
	if (remaining > 0) {
	    task->remaining_ = remaining;
	} else {
	    task->remaining_ = 0;
	}
	return task;
    }

    // activate task, refresh time slice and start timer if needed
    void Task::activate(uint64_t now) {
	write_kernel_thread_id((uint32_t)this);
	start_ = now;

	size_t offset = (uint8_t*)this - (uint8_t*)&(state_list_);
	uint32_t member = (uint32_t)(state_list_.next()->next());
	UART::puts("single task check: ");
	UART::put_uint32((uint32_t)this);
	UART::putc(' ');
	UART::put_uint32((uint32_t)(member + offset));
	UART::putc('\n');

	// only schedule when there is more than one task running
	if (// already cought by the second test: (task != idle) &&
	    ((Task*)(member + offset) != this)) {
	    UART::puts("  # with timer\n");
	    // set timer for end_of_slice
	    if (remaining_ > 0) {
		UART::puts(" continuing: ");
		UART::put_uint32(remaining_);
		UART::putc('\n');
	    } else {
		UART::puts(" new slice\n");
		remaining_ = TIME_SLICE;
	    }
	    end_of_slice = now + remaining_;
	    Timer::set_timer(idle->timer(), end_of_slice);
	} else {
	    UART::puts("  # without timer\n");
	}
    }

    void Task::schedule(State new_state) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	// get current time
	uint64_t now = Timer::system_time();
	Task *task = deactivate(now);
	UART::puts(" remaining: ");
	UART::put_uint32(task->remaining_);
	UART::putc('\n');
	UART::puts(task->name());
	UART::puts(" -> ");
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
	task = (Task*)(member + offset);
	if (task == idle) {
	    UART::puts("### skipping <IDLE>");
	    UART::put_uint32(now);
	    UART::putc('\n');
	    task->start_ = now;
	    member = (uint32_t)(task->state_list_.next());
	    task = (Task*)(member + offset);
	}
	UART::puts(task->name());
	UART::putc('\n');
	task->activate(now);
    }

    /* preempt running task with awoken task
     * go back to the running task after this
     */
    void Task::wakeup() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::puts(name_);
	UART::putc('\n');
	// get current time
	uint64_t now = Timer::system_time();
	// wake up task
	if (state_ != SLEEPING) {
	    // FIXME: can happen once more than one timer is allowed
	    panic("Task::wakeup(): Already awake!\n");
	}
	state_list_.remove();
	state_ = RUNNING;
	Task *task = deactivate(now);
	UART::puts(" old task = ");
	UART::puts(task->name());
	UART::putc('\n');
	UART::puts(" remaining: ");
	UART::put_uint32(task->remaining_);
	UART::putc('\n');

	// Put woken up task before current one
	task->state_list_.insert_before(&state_list_);
	// set timer for end_of_slice
	UART::puts("  ### start_: ");
	UART::put_uint32(idle->start_);
	UART::putc(' ');
	UART::put_uint32(start_);
	UART::putc('\n');
	if ((task == idle) || ((int64_t)(idle->start_ - start_) > 0)) {
	    // task was asleep long enough to deserve a new timeslice
	    UART::puts("  # grant new slice\n");
	    remaining_ = TIME_SLICE;
	}
	// preempt only if the woken task has time left
	if (remaining_ > 0) {
	    // woken up task has time left
	    UART::puts(" new task = ");
	    UART::puts(name());
	    UART::putc('\n');
	    activate(now);
	} else if (task->remaining_ > 0) {
	    // old task has time left
	    UART::puts(" new task = ");
	    UART::puts(task->name());
	    UART::putc('\n');
	    task->activate(now);
	} else {
	    // bummer, both tasks are out of time
	    schedule(RUNNING);
	}
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
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	// create dummy heads so Task::Task() can function
	List::DList all_dummy, running_dummy, sleeping_dummy;
	all_head = &all_dummy;
	state_head[RUNNING] = &running_dummy;
	state_head[SLEEPING] = &sleeping_dummy;
	// create core tasks
	idle = new Task("<IDLE>", idle_idles, RUNNING, schedule_callback);
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

    /*
     * put current task to sleep
     * time: nanoseconds to sleep
     */
    uint32_t sys_sleep(int64_t time) {
	if (time == 0) {
	    // yield
	    Task::schedule(RUNNING);
	} else if (time > 0) {
	    Task *task = (Task*)read_kernel_thread_id();
	    uint64_t wakeup = Timer::system_time() + time;
	    Task::schedule(SLEEPING);
	    Timer::set_timer(task->timer(), wakeup);
	} else {
	    // FIXME: EINVAL
	    return -1;
	}
	return 0;
    }
}

