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

#include <panic.h>
#include <list.h>

namespace Task {
    class Task;
}

namespace Message {
    typedef uint32_t MailboxId;
    static const MailboxId NoId = ~0;
    extern const MailboxId NoStdio[3];
    extern const MailboxId DefStdio[3];

    class Mailbox;

    union Data {
	void *ptr;
	uint32_t i;
    };

    enum Type {
	CUSTOM,
	SIG_CHILD,
	SIG_PARENT,
	REQUEST_READ,
	REQUEST_WRITE,
	REPLY_READ,
    };
    
    class Message {
    public:
	Message(Type type__) : list(), mailbox_id(NoId), mailbox_data({.ptr = nullptr}), type(type__) { }
	List::CDList list;
	MailboxId mailbox_id;
	Data mailbox_data;
	Type type;
    };

    class CustomMessage : public Message {
    public:
	CustomMessage() : Message(CUSTOM) { }
    };
    
    class ReadRequest : public Message {
    public:
	ReadRequest(size_t size__) : Message(REQUEST_READ), size(size__), data() { }
	size_t size;
	char data[];
    };

    class WriteRequest : public Message {
    public:
	WriteRequest(const char *str) : Message(REQUEST_WRITE), size(0) {
	    while(*str) {
		data[size] = *str;
		++size;
		++str;
	    }
	    data[size] = 0;
	    ++size;
	}
	size_t size;
	char data[];
    };

    class ReadReply : public Message {
    public:
	ReadReply(size_t size__, const char *data__)
	    : Message(REPLY_READ), size(size__) {
	    // FIXME: use memcpy
	    for (size_t i = 0; i < size; ++i) {
		data[i] = data__[i];
	    }
	}
	size_t size;
	char data[];
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
	bool is_connected() {
	    return other_ != nullptr;
	}
	void disconnect() {
	    if (other_ == nullptr) {
		panic("Message::Mailbox::disconnect(): already disconneced!");
	    }
	    other_->other_ = nullptr;
	    other_ = nullptr;
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
