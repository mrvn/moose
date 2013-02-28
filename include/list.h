/* list.h - linked lists */
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

#ifndef MOOSE_LIB_LIST_H
#define MOOSE_LIB_LIST_H

#include <stddef.h>
#include <stdint.h>

#define OFFSETOF(TYPE, MEMBER)  __builtin_offsetof (TYPE, MEMBER)
#define FROMOFFSET(PTR, TYPE, OFF) ((TYPE*)((intptr_t)PTR - OFF))
#define CONTAINEROF(PTR, TYPE, MEMBER) FORMOFFSET(PTR, TYPE, OFFSETOF(TYPE, MEMBER))
#define FROMCONTAINER(PTR, TYPE, OFF) ((TYPE*)((intptr_t)PTR + OFF))

namespace List {
    class DList {
    public:
	DList()  __attribute__((nothrow)) : prev_(this), next_(this) { }

	DList * prev() { return prev_; }
	DList * next() { return next_; }

	void insert_before(DList *item) {
	    // FIXME: assumes item is not already in a list
	    item->prev_  = prev_;
	    item->next_  = this;
	    prev_->next_ = item;
	    this->prev_  = item;
	}

	void insert_after(DList *item) {
	    // FIXME: assumes item is not already in a list
	    item->prev_  = this;
	    item->next_  = next_;
	    next_->prev_ = item;
	    this->next_  = item;
	}

	void remove() {
	    prev_->next_ = next_;
	    next_->prev_ = prev_;
	    next_ = this;
	    prev_ = this;
	}
    private:
	DList *prev_;
	DList *next_;
    };
}

#endif // #ifndef MOOSE_LIB_LIST_H
