/* message.h - inter process communication structures */
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

#ifndef MOOSE_MESSAGE_H
#define MOOSE_MESSAGE_H

#include <list.h>

namespace Task {
    class Task;
}

namespace Message {
    typedef uint32_t MailboxId;

    class Mailbox;

    union Data {
	void *ptr;
	uint32_t i;
    };

    struct Message {
	List::CDList list;
	MailboxId mailbox_id;
	Data mailbox_data;
	size_t size;
	uint8_t data[];
    };

    class Mailbox {
    public:
	Mailbox() : messages_(), owner_(nullptr), other_(nullptr) { }
	Task::Task * owner() const { return owner_; }
	Mailbox * other() const { return other_; }
	void insert(Message *msg) {
	    messages_.insert_before(msg);
	}
	Message * get_message() {
	    List::CDHead<Message, &Message::list>::Iterator it =
		messages_.begin();
	    if (it != messages_.end()) {
		messages_.remove(*it);
		return &(*it);
	    } else {
		return nullptr;
	    }
	}
	void connect(Mailbox &box) {
	    if (other_ != nullptr || box.other_ != nullptr) {
		panic("Message::Mailbox::connect(): already connected!");
	    }
	    other_ = &box;
	    box.other_ = this;
	}
	Data data;
    private:
	friend Task::Task;
	void set_owner(Task::Task *owner__) { owner_ = owner__; }
	List::CDHead<Message, &Message::list> messages_;
	Task::Task *owner_;
	Mailbox *other_;
    };

/*
    class IdTable {
    public:
	IdTable() : next_id_(1) {
	    for(int i = 0; i < NUM_SLOTS; ++i) {
		slot_[i] = NULL;
	    }
	}
	void insert(Message *msg) {
	    uint32_t id = next_id();
	    UART::puts("IdTable::insert(): id = ");
	    UART::put_uint32(id);
	    UART::putc('\n');
	    msg->id = id;
	    if (slot_[id % NUM_SLOTS] == NULL) {
		slot_[id % NUM_SLOTS] = msg;
	    } else {
		slot_[id % NUM_SLOTS]->list.insert_before(&msg->list);
	    }
	}
	Message * remove(Id id) {
	    UART::puts("IdTable::remove(): id = ");
	    UART::put_uint32(id);
	    UART::putc('\n');
	    Message *first = slot_[id % NUM_SLOTS];
	    if (first == NULL) return NULL;
	    Message *msg = first;
	    do {
		Message *next = (Message*)(msg->list.next());
		if (msg->id == id) {
		    msg->list.remove();
		    if (msg == first) {
			if (msg == next) {
			    slot_[id % NUM_SLOTS] = NULL;
			} else {
			    slot_[id % NUM_SLOTS] = next;
			}
		    }
		    return msg;
		}
		msg = next;
	    } while(msg != first);
	    return NULL;
	}
    private:
	Id next_id() {
	    return next_id_++;
	}
	enum { NUM_SLOTS = 16 };
	Id next_id_;
	Message *slot_[NUM_SLOTS];
    };
*/
}

#endif // #ifndef MOOSE_MESSAGE_H
