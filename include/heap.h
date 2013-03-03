/* heap.h - generic heap datastructure */
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

#ifndef MOOSE_HEAP_H
#define MOOSE_HEAP_H

#include <stdint.h>

namespace Heap {
    enum {
	PAGE_SIZE = 4096,
	MAX_SLOTS = PAGE_SIZE / sizeof(void*),
    };

    template<typename T>
    class Heap;

    template<typename PRI_T>
    class Item {
    public:
	Item(PRI_T priority__)
	: priority_(priority__), heap_slot_(0) {
	}
	PRI_T priority() const { return priority_; }
    private:
	PRI_T priority_;
	uint32_t heap_slot_;
	template <typename> friend class Heap;
    };

    template<typename T>
    class Heap {
    public:
	Heap() : num_entries_(0) {
	    for(int i = 1; i < MAX_SLOTS; ++i) {
		slot_[i] = NULL;
	    }
	}

	bool empty() const { return num_entries_ == 0; }

	const T * peek() const {
	    return slot_[1];
	}

	void push(T *item) {
	    ++num_entries_;
	    if (num_entries_ >= MAX_SLOTS) {
		panic("Heap::push(): Out of slots!");
	    }
	    slot_[num_entries_] = item;
	    item->heap_slot_ = num_entries_;
	    heap_up(num_entries_);
	}

	T *pop() {
	    if (num_entries_ == 0) {
		panic("Heap::pop(): Out of items!");
	    }
	    T *res = slot_[1];
	    slot_[1] = slot_[num_entries_];
	    slot_[1]->heap_slot_ = 1;
	    --num_entries_;
	    heap_down(1);
	    res->heap_slot_ = 0;
	    return res;
	}

	template<typename I>
	void set_priority(T *item, I priority ) {
	    if (item->heap_slot_ == 0) {
		item->priority_ = priority;
		push(item);
	    } else {
		if (item->priority_ < priority) {
		    item->priority_ = priority;
		    heap_up(item->heap_slot_);
		} else {
		    item->priority_ = priority;
		    heap_down(item->heap_slot_);
		}
	    }
	}
    private:
	void heap_up(int child) {
	    if (child == 1) return;
	    int parent = child / 2;
	    T *tc = slot_[child];
	    T *tp = slot_[parent];
	    if (tc->priority_ < tp->priority_) {
		slot_[child] = tp;
		tp->heap_slot_ = child;
		slot_[parent] = tc;
		tc->heap_slot_ = parent;
		heap_up(parent);
	    }
	}

	void heap_down(int parent) {
	    int child1 = parent * 2, child2 = child1 + 1;
	    if (child1 > num_entries_) return;
	    // if there is only one child use it twice
	    if (child2 > num_entries_) --child2;
	    T *tp = slot_[parent];
	    T *tc1 = slot_[child1];
	    T *tc2 = slot_[child2];
	    // pick smallest child
	    if (tc1->priority_ > tc2->priority_) {
		child1 = child2;
		tc1 = tc2;
	    }
	    if (tc1->priority_ < tp->priority_) {
		slot_[child1] = tp;
		tp->heap_slot_ = child1;
		slot_[parent] = tc1;
		tc1->heap_slot_ = parent;
		heap_down(child1);
	    }
	}

	union {
	    int num_entries_;
	    T *slot_[MAX_SLOTS];
	};
    };
}

#endif // #ifndef MOOSE_HEAP_H
