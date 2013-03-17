/* timer.h - hardware timer */
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

#ifndef MOOSE_KERNEL_TIMER_H
#define MOOSE_KERNEL_TIMER_H

#include <stdint.h>
#include <heap.h>

namespace Task {
    class Task;
}

namespace Timer {
    class Timer : public Heap::Item<uint64_t> {
    public:
	typedef void (*callback_fn)(Timer *timer, void *data);

	Timer(const char *name__, Task::Task *owner__, uint64_t wakeup_time,
	      callback_fn callback__, void *data__);
	const char * name() const { return name_; }
	Task::Task * owner() const { return owner_; }
	void callback() { callback_(this, data_); }
    private:
	const char *name_;
	Task::Task * const owner_;
	const callback_fn callback_;
	void * const data_;
    };

    /*
     * Initialize Timer.
     */
    void init(void);

    /*
     * get system time
     * returns: time in 1/1000000th of seconds since power on.
     */
    uint64_t system_time();

    /*
     * set timer
     * timer:       timer to set
     * wakeup_time: time when timer expires
     */
    void set_timer(Timer *timer, uint64_t wakeup_time);
}

#endif // #ifndef MOOSE_KERNEL_TIMER_H
