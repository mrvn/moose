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

template<typename T, typename S, S T::*M>
T * container_of(S *obj) {
    return (T*)((intptr_t)obj - (intptr_t)&(((T*)nullptr)->*M));
}

namespace List {
    class List {
    public:
	template<typename T, List T::*L>
	class Iterator {
	public:
	    Iterator(T &t) : pos_(&(t.*L)) { }
	    Iterator(T *t) : pos_(&(t->*L)) { }
	    T & operator*() { return *container_of<T, List, L>(pos_); }
	    const T & operator*() const { return *container_of<T, List, L>(pos_); }
	    Iterator & operator++() {
		pos_ = pos_->next_;
		return *this;
	    }
	    static const Iterator end() { return Iterator(); }
	    bool operator !=(const Iterator &it) const {
		return pos_ != it.pos_;
	    }
	    bool operator ==(const Iterator &it) const {
		return pos_ == it.pos_;
	    }
	    void insert_after(T &t) {
		pos_->insert_after<T, L>(t);
	    }
	    void insert_after(T *t) {
		pos_->insert_after<T, L>(t);
	    }
	protected:
	    explicit Iterator(List *pos = nullptr) : pos_(pos) { }
	    List *pos_;
	};
	List(List *next = nullptr) : next_(next) { }

	template<typename T, List T::*L>
	void insert_after(T &t) {
	    (t.*L).next_ = next_;
	    next_ = &(t.*L);
	}

    	template<typename T, List T::*L>
	void insert_after(T *t) {
	    (t->*L).next_ = next_;
	    next_ = &(t->*L);
	}
    protected:
	List *next_;
    };

    template<typename T, List T::*L>
    class Head : public List {
    public:
	class Iterator : public List::Iterator<T, L> {
	public:
	    Iterator(T &t) : List::Iterator<T, L>(t) { }
	    Iterator(T *t) : List::Iterator<T, L>(t) { }
	    static Iterator end() {
		return Iterator();
	    }
	    Iterator & operator++() {
		List::Iterator<T, L>::operator++();
		return *this;
	    }
	protected:
	    explicit Iterator(List *pos = nullptr) : List::Iterator<T, L>(pos) { }
	};
	void insert_after(T &t) {
	    List::insert_after<T, L>(t);
	}
	void insert_after(T *t) {
	    List::insert_after<T, L>(t);
	}
	Iterator begin() {
	    return ++Iterator(container_of<T, List, L>(this));
	}
	Iterator end() {
	    return Iterator::end();
	}
    };
    
    class CList : public List {
    public:
	template<typename T, CList T::*L>
	class Iterator : public List::Iterator<T, (List T::*)L> {
	public:
	    Iterator(T &h) : List::Iterator<T, (List T::*)L>(&(h.*L)),
			     head_(&(h.*L)) { }
	    Iterator(T *h) : List::Iterator<T, (List T::*)L>(&(h->*L)),
			     head_(&(h->*L)) { }
	    static Iterator begin(CList *h) { return ++Iterator(h); }
	    const Iterator end() { return Iterator(head_); }
	    void insert_after(T &t) {
		List::Iterator<T, (List T::*)L>::insert_after(t);
	    }
	    void insert_after(T *t) {
		List::Iterator<T, (List T::*)L>::insert_after(t);
	    }
	protected:
	    explicit Iterator(CList *head)
		: List::Iterator<T, (List T::*)L>(head), head_(head) { }
	    CList *head_;
	};
	CList() : List(this) { }

	template<typename T, CList T::*L>
	void insert_after(T &t) {
	    List::insert_after<T, (List T::*)L>(t);
	}

    	template<typename T, CList T::*L>
	void insert_after(T *t) {
	    List::insert_after<T, (List T::*)L>(t);
	}

    	template<typename T, CList T::*L>
	Iterator<T, L> begin() {
	    return Iterator<T, L>::begin(this);
	}
    };

    template<typename T, CList T::*L>
    class CHead : public CList {
    public:
	class Iterator : public CList::Iterator<T, L> {
	public:
	    Iterator(CHead *h) : CList::Iterator<T, L>(h) { }
	    Iterator & operator++() {
		CList::Iterator<T, L>::operator++();
		return *this;
	    }
	};
	void insert_after(T &t) {
	    CList::insert_after<T, L>(t);
	}
	void insert_after(T *t) {
	    CList::insert_after<T, L>(t);
	}
	Iterator begin() {
	    return ++iterator();
	}
	Iterator end() {
	    return iterator();
	}
    protected:
	Iterator iterator() {
	    return Iterator(this);
	}
    };

    class DList {
    public:
	template<typename T, DList T::*L>
	class Iterator {
	public:
	    Iterator(T &t) : pos_(&(t.*L)) { }
	    Iterator(T *t) : pos_(&(t->*L)) { }
	    T & operator*() { return *container_of<T, DList, L>(pos_); }
	    const T & operator*() const { return *container_of<T, DList, L>(pos_); }
	    Iterator & operator++() {
		pos_ = pos_->next_;
		return *this;
	    }
	    Iterator & operator--() {
		pos_ = pos_->prev_;
		return *this;
	    }
	    static const Iterator end() { return Iterator(); }
	    bool operator !=(const Iterator &it) const {
		return pos_ != it.pos_;
	    }
	    bool operator ==(const Iterator &it) const {
		return pos_ == it.pos_;
	    }
	    void insert_after(T &t) {
		pos_->insert_after<T, L>(t);
	    }
	    void insert_after(T *t) {
		pos_->insert_after<T, L>(t);
	    }
	    void insert_before(T &t) {
		pos_->insert_before<T, L>(t);
	    }
	    void insert_before(T *t) {
		pos_->insert_before<T, L>(t);
	    }
	protected:
	    explicit Iterator(DList *pos = nullptr) : pos_(pos) { }
	    DList *pos_;
	};

	template<typename T, DList T::*L>
	class RevIterator {
	public:
	    RevIterator(T &t) : pos_(&(t.*L)) { }
	    RevIterator(T *t) : pos_(&(t->*L)) { }
	    T & operator*() { return *container_of<T, DList, L>(pos_); }
	    const T & operator*() const { return *container_of<T, DList, L>(pos_); }
	    RevIterator & operator++() {
		pos_ = pos_->prev_;
		return *this;
	    }
	    RevIterator & operator--() {
		pos_ = pos_->next_;
		return *this;
	    }
	    static const RevIterator end() { return RevIterator(); }
	    bool operator !=(const RevIterator &it) const {
		return pos_ != it.pos_;
	    }
	    bool operator ==(const RevIterator &it) const {
		return pos_ == it.pos_;
	    }
	    void insert_after(T &t) {
		pos_->insert_after<T, L>(t);
	    }
	    void insert_after(T *t) {
		pos_->insert_after<T, L>(t);
	    }
	    void insert_before(T &t) {
		pos_->insert_before<T, L>(t);
	    }
	    void insert_before(T *t) {
		pos_->insert_before<T, L>(t);
	    }
	protected:
	    explicit RevIterator(DList *pos = nullptr) : pos_(pos) { }
	    DList *pos_;
	};
	DList(DList *prev = nullptr, DList *next = nullptr)
	    : prev_(prev), next_(next) { }
	
	template<typename T, DList T::*L>
	void insert_after(T &t) {
	    (t.*L).next_ = next_;
	    (t.*L).prev_ = this;
	    if (next_ != nullptr) next_->prev_ = &(t.*L);
	    next_ = &(t.*L);
	}

	template<typename T, DList T::*L>
	void insert_after(T *t) {
	    (t->*L).next_ = next_;
	    (t->*L).prev_ = this;
	    if (next_ != nullptr) next_->prev_ = &(t->*L);
	    next_ = &(t->*L);
	}

	template<typename T, DList T::*L>
	void insert_before(T &t) {
	    (t.*L).next_ = this;
	    (t.*L).prev_ = prev_;
	    if (prev_ != nullptr) prev_->next_ = &(t.*L);
	    prev_ = &(t.*L);
	}

	template<typename T, DList T::*L>
	void insert_before(T *t) {
	    (t->*L).next_ = this;
	    (t->*L).prev_ = prev_;
	    if (prev_ != nullptr) prev_->next_ = &(t->*L);
	    prev_ = &(t->*L);
	}

	void remove(DList *val = nullptr) {
	    if (prev_ != nullptr) prev_->next_ = next_;
	    if (next_ != nullptr) next_->prev_ = prev_;
	    next_ = prev_ = val;
	}

	template<typename T, DList T::*L>
	Iterator<T, L> iterator() {
	    return Iterator<T, L>(container_of<T, DList, L>(this));
	}

	template<typename T, DList T::*L>
	RevIterator<T, L> reviterator() {
	    return RevIterator<T, L>(container_of<T, DList, L>(this));
	}
    protected:
	DList *prev_;
	DList *next_;
    };

    template<typename T, DList T::*L>
    class DHead : public DList {
    public:
	class Iterator : public DList::Iterator<T, L> {
	public:
	    Iterator(T &t) : DList::Iterator<T, L>(t) { }
	    Iterator(T *t) : DList::Iterator<T, L>(t) { }
	    static Iterator end() {
		return Iterator();
	    }
	    Iterator & operator++() {
		DList::Iterator<T, L>::operator++();
		return *this;
	    }
	protected:
	    explicit Iterator(DList *pos = nullptr)
		: DList::Iterator<T, L>(pos) { }
	};
	void insert_after(T &t) {
	    DList::insert_after<T, L>(t);
	}
	void insert_after(T *t) {
	    DList::insert_after<T, L>(t);
	}
	void remove(T &t) {
	    (t.*L).remove();
	}
	void remove(T *t) {
	    (t->*L).remove();
	}
	Iterator begin() {
	    return ++Iterator(container_of<T, DList, L>(this));
	}
	Iterator end() {
	    return Iterator::end();
	}
	DList::RevIterator<T, L> rend() {
	    return DList::RevIterator<T, L>(container_of<T, DList, L>(this));;
	}
    };

    class CDList : public DList {
    public:
	template<typename T, CDList T::*L>
	class Iterator : public DList::Iterator<T, (DList T::*)L> {
	public:
	    Iterator(T &h) : DList::Iterator<T, (DList T::*)L>(h),
			     head_(&(h.*L)) { }
	    Iterator(T *h) : DList::Iterator<T, (DList T::*)L>(h),
			     head_(&(h->*L)) { }
	    const Iterator end() { return Iterator(head_); }
	    void insert_after(T &t) {
		DList::Iterator<T, (DList T::*)L>::insert_after(t);
	    }
	    void insert_after(T *t) {
		DList::Iterator<T, (DList T::*)L>::insert_after(t);
	    }
	    void insert_before(T &t) {
		DList::Iterator<T, (DList T::*)L>::insert_before(t);
	    }
	    void insert_before(T *t) {
		DList::Iterator<T, (DList T::*)L>::insert_before(t);
	    }
	protected:
	    explicit Iterator(CDList *head)
		: DList::Iterator<T, (DList T::*)L>(head), head_(head) { }
	    CDList *head_;
	};

	template<typename T, CDList T::*L>
	class RevIterator : public DList::RevIterator<T, (DList T::*)L> {
	public:
	    RevIterator(T &h) : DList::RevIterator<T, (DList T::*)L>(h),
		  head_(&(h.*L)) { }
	    RevIterator(T *h) : DList::RevIterator<T, (DList T::*)L>(h),
		  head_(&(h->*L)) { }
	    const RevIterator end() { return RevIterator(head_); }
	    void insert_after(T &t) {
		DList::RevIterator<T, (DList T::*)L>::insert_after(t);
	    }
	    void insert_after(T *t) {
		DList::RevIterator<T, (DList T::*)L>::insert_after(t);
	    }
	    void insert_before(T &t) {
		DList::RevIterator<T, (DList T::*)L>::insert_before(t);
	    }
	    void insert_before(T *t) {
		DList::RevIterator<T, (DList T::*)L>::insert_before(t);
	    }
	protected:
	    explicit RevIterator(CDList *head)
		: DList::RevIterator<T, (DList T::*)L>(head), head_(head) { }
	    CDList *head_;
	};
	CDList() : DList(this, this) { }
	
	template<typename T, CDList T::*L>
	void insert_after(T &t) {
	    DList::insert_after<T, (DList T::*)L>(t);
	}

	template<typename T, CDList T::*L>
	void insert_after(T *t) {
	    DList::insert_after<T, (DList T::*)L>(t);
	}

	template<typename T, CDList T::*L>
	void insert_before(T &t) {
	    DList::insert_before<T, (DList T::*)L>(t);
	}

	template<typename T, CDList T::*L>
	void insert_before(T *t) {
	    DList::insert_before<T, (DList T::*)L>(t);
	}

	void remove() {
	    DList::remove(this);
	}
    };

    template<typename T, CDList T::*L>
    class CDHead : public CDList {
    public:
	class Iterator : public CDList::Iterator<T, L> {
	public:
	    Iterator(T &h) : CDList::Iterator<T, L>(h) { }
	    Iterator(T *h) : CDList::Iterator<T, L>(h) { }
	    Iterator & operator++() {
		CDList::Iterator<T, L>::operator++();
		return *this;
	    }
	};
	class RevIterator : public CDList::RevIterator<T, L> {
	public:
	    RevIterator(T &h) : CDList::RevIterator<T, L>(h) { }
	    RevIterator(T *h) : CDList::RevIterator<T, L>(h) { }
	    RevIterator & operator++() {
		CDList::RevIterator<T, L>::operator++();
		return *this;
	    }
	};
	void insert_after(T &t) {
	    CDList::insert_after<T, L>(t);
	}
	void insert_after(T *t) {
	    CDList::insert_after<T, L>(t);
	}
	void insert_before(T &t) {
	    CDList::insert_before<T, L>(t);
	}
	void insert_before(T *t) {
	    CDList::insert_before<T, L>(t);
	}
	void remove(T &t) {
	    (t.*L).remove();
	}
	void remove(T *t) {
	    (t->*L).remove();
	}
	Iterator begin() {
	    return ++iterator();
	}

	RevIterator rbegin() {
	    return ++reviterator();
	}

	Iterator end() {
	    return iterator();
	}

	RevIterator rend() {
	    return reviterator();
	}
    protected:
	Iterator iterator() {
	    return Iterator(container_of<T, CDList, L>(this));
	}
	RevIterator reviterator() {
	    return RevIterator(container_of<T, CDList, L>(this));
	}
    };
}

#endif // #ifndef MOOSE_LIB_LIST_H
