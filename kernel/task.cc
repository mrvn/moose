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
    
    void schedule_callback(Timer::Timer *, void *) {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	Task::schedule(RUNNING);
    }
    
//    Timer::Timer schedule_timer("<schedule>", nullptr, 0, schedule_callback, nullptr);
    List::CDHead<Task, &Task::all_list_> Task::all_head;
    List::CDList *Task::state_head[NUM_STATES];
    Task *idle;
    Task *sleepy;
    Task *waity;

    uint64_t end_of_slice;

    void idle_idles(void *) {
	while(true) {
	    asm volatile("wfi");
	}
    }
    
    void sleepy_must_not_wake(void *) {
       panic("Sleepy must not wake!");
    }

    void waity_must_not_wake(void *) {
       panic("Waity must not wake!");
    }

    /*static*/ void Task::timer_callback(Timer::Timer *timer, void *) {
	timer->owner()->wakeup();
    }

    /*static*/ void Task::starter(start_fn start, void *arg) {
	start(arg);
	Syscall::end_thread();
    }
    
    Task::Task(const char *name__, start_fn start, void *arg,
	       State state__, Timer::Timer::callback_fn timer_callback__)
	: name_(name__), state_(state__), all_list_(List::CDList()),
	  state_list_(List::CDList()),
	  timer_(name__, this, 0, timer_callback__, this),
	  start_(0), remaining_(0), mailboxes_(0) {
	for (int i = 0; i < NUM_REGS; ++i) {
	    regs_[i] = 0xCAFEBABE;
	}
	for (int i = 0; i < NUM_MAILBOXES; ++i) {
	    mailbox_[i].set_owner(this);
	}
	// FIXME: allocate stack in user space
	regs_[REG_SP] =
	    ((uint32_t)Memory::early_malloc(2)) + Memory::PAGE_SIZE * 4;
	regs_[REG_PC] = (uint32_t)starter;
	regs_[REG_R0] = (uint32_t)start;
	regs_[REG_R1] = (uint32_t)arg;
	// set A I F SYS
	regs_[REG_CPSR] = 0b101011111;
	all_head.insert_before(this);
	state_head[state_]->insert_before<Task, &Task::state_list_>(this);
    }

    Task::~Task() {
	if (state_ == RUNNING) {
	    panic("Task::~Task(): still running");
	}
	all_list_.remove();
	state_list_.remove();
	// FIXME: free resources
    }
    
    void * Task::operator new(size_t) {
	void *res = Memory::early_malloc(0);
	return res;
    }
    
    void Task::operator delete(void *addr) {
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
    /*static*/ Task * Task::deactivate(uint64_t now) {
	// get current running task
	Task *task = read_kernel_thread_id();
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
	write_kernel_thread_id(this);
	start_ = now;

	List::CDList::Iterator<Task, &Task::state_list_> it(this);

	// only schedule when there is more than this and the idle task:
	++it;
	++it;
	if (it != it.end()) {
	    // set timer for end_of_slice
	    if (remaining_ <= 0) {
		remaining_ = TIME_SLICE;
	    }
	    end_of_slice = now + remaining_;
	    Timer::set_timer(idle->timer(), end_of_slice);
	} else {
	    // FIXME: remove timer
	    Timer::set_timer(idle->timer(), now + (1LLU << 32));
	}
    }

    /*static*/ void Task::dump_tasks() {
	UART::puts("all tasks: ");
	for (List::CDHead<Task, &Task::all_list_>::Iterator it = all_head.begin(); it != all_head.end(); ++it) {
	    UART::put_uint32((uint32_t)&(*it));
	    UART::puts((*it).name());
	    UART::putc(' ');
	}
	UART::putc('\n');
	/*
	UART::puts("running tasks: ");
	for (List::CDHead<Task, &Task::state_list_>::Iterator it = state_head[RUNNING].begin(); it != state_head[RUNNING].end(); ++it) {
	    UART::put_uint32((uint32_t)&(*it));
	    UART::puts((*it).name());
	    UART::putc(' ');
	}
	UART::putc('\n');
	UART::puts("sleeping tasks: ");
	for (List::CDHead<Task, &Task::state_list_>::Iterator it = state_head[SLEEPING].begin(); it != state_head[SLEEPING].end(); ++it) {
	    UART::put_uint32((uint32_t)&(*it));
	    UART::puts((*it).name());
	    UART::putc(' ');
	}
	UART::putc('\n');
	UART::puts("waiting tasks: ");
	for (List::CDHead<Task, &Task::state_list_>::Iterator it = state_head[WAITING].begin(); it != state_head[WAITING].end(); ++it) {
	    UART::put_uint32((uint32_t)&(*it));
	    UART::puts((*it).name());
	    UART::putc(' ');
	}
	UART::putc('\n');
	*/
    }
    
    /*static*/ void Task::schedule(State new_state) {
//	UART::puts(__PRETTY_FUNCTION__);
//	UART::putc('\n');
	// get current time
	uint64_t now = Timer::system_time();
	Task *task = deactivate(now);
	UART::puts(task->name());
	UART::puts(" -> ");
	// get next running task
	List::CDList::Iterator<Task, &Task::state_list_> it(task);
	++it;
	// update state of old task
	if (task->state_ != new_state) {
	    if (task == idle) {
		panic("Task::schedule(): idle task must not change state!");
	    }
	    task->state_list_.remove();
	    task->state_ = new_state;
	    state_head[new_state]->insert_before<Task, &Task::state_list_>(task);
	}
	// skip over idle
	if (&(*it) == idle) {
//	    UART::puts(" ## skip ##\n");
	    idle->start_ = now;
	    ++it;
	}
	// activate task
	UART::puts((*it).name());
	UART::putc('\n');
	(*it).activate(now);
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
	if (state_ == RUNNING) {
	    UART::puts("Task::wakeup(): Already awake!\n");
	    return;
	}
	state_list_.remove();
	if (state_ == WAITING) {
	    // retrieve message that woke up the task
	    regs_[REG_R0] = (uint32_t)get_message();
	}
	// new task is now ready to run
	state_ = RUNNING;

	// suspend current task
	Task *task = deactivate(now);
	// Put woken up task before current one
	task->state_list_.insert_before<Task, &Task::state_list_>(this);
	// compute time for end_of_slice
	if (task == idle || (int64_t)(idle->start_ - start_) > 0) {
	    // task was asleep long enough to deserve a new timeslice
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
    /*static*/ void Task::die() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	Task *task = read_kernel_thread_id();
	// select a new task
	// change to SLEEPING to prevent geting picked again
	schedule(SLEEPING);
	delete task;
	UART::puts(" task died\n");
    }

    /*
     * create a new task and connect a Mailbox
     * name:    name of task
     * start:   start function
     * arg:     argument to start function
     * returns: id of mailbox connected to new task
     */
    Message::MailboxId Task::create_task(const char *name__, start_fn start, void *arg) {
	if (mailboxes_ == NUM_MAILBOXES) {
	    panic("Task::create_task(): out of mailboxes!");
	}
	Message::MailboxId id = mailboxes_++;
	Task *task = new Task(name__, start, arg);
	++task->mailboxes_;
	mailbox_[id].connect(task->mailbox_[0]);
	return id;
    }
    
    void init() {
	UART::puts(__PRETTY_FUNCTION__);
	UART::putc('\n');
	// create dummy heads so Task::Task() can function
	List::CDList running_dummy, sleeping_dummy, waiting_dummy;
	Task::state_head[RUNNING] = &running_dummy;
	Task::state_head[SLEEPING] = &sleeping_dummy;
	Task::state_head[WAITING] = &waiting_dummy;
	// create core tasks
	idle = new Task("<IDLE>", idle_idles, NULL, RUNNING, schedule_callback);
	sleepy = new Task("<SLEEPY>", sleepy_must_not_wake, NULL, SLEEPING);
	waity = new Task("<WAITY>", waity_must_not_wake, NULL, WAITING);
	if (idle == NULL || sleepy == NULL || waity == NULL) {
	    panic("Out of memory allocating core tasks!");
	}
	// correct heads
	Task::state_head[RUNNING] = &idle->state_list_;
	Task::state_head[SLEEPING] = &sleepy->state_list_;
	Task::state_head[WAITING] = &waity->state_list_;
	// cleanup dummies
	running_dummy.remove();
	sleeping_dummy.remove();
	waiting_dummy.remove();

	// Create kernel thread
	Task *kernel = new Task("<MOOSE>", NULL, NULL, RUNNING);
	kernel->fix_kernel_stack();

    	// load thread register
	write_kernel_thread_id(kernel);
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
	    Task *task = read_kernel_thread_id();
	    uint64_t wakeup = Timer::system_time() + time;
	    Task::schedule(SLEEPING);
	    Timer::set_timer(task->timer(), wakeup);
	} else {
	    // FIXME: EINVAL
	    return -1;
	}
	return 0;
    }

    /*
     * retrun pending message or wait for one
     * returns: pointer to message or NULL
     */
    Message::Message * sys_recv_message() {
	Task *task = read_kernel_thread_id();
	Message::Message *msg = task->get_message();
	if (msg == NULL) {
	    Task::schedule(WAITING);
	}
	return msg;
    }

    void Task::send_message(Message::MailboxId id, Message::Message *msg) {
	if (id >= mailboxes_) {
	    panic("Task::send_message(): illegal id\n");
	}
	Message::Mailbox *dest = mailbox_[id].other();
	Task *task = dest->owner();
	msg->mailbox_id = dest - &task->mailbox_[0];
	task->incoming_.insert_before(msg);
	if (task->state_ != RUNNING) {
	    task->wakeup();
	}
    }

    Message::Message * Task::get_message() {
	List::CDHead<Message::Message, &Message::Message::list>::Iterator it =
	    incoming_.begin();
	if (it != incoming_.end()) {
	    incoming_.remove(*it);
	    return &(*it);
	} else {
	    return nullptr;
	}
    }
}

