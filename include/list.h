/* Copyright (C) 2015 Goswin von Brederlow <goswin-v-b@web.de>

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

/* Doubly linked list class
 */

#ifndef LIST_H
#define LIST_H 1

template<class Container, class Identifier>
class List {
public:
    constexpr List(Container *self) : next_(self), prev_(self) { }

    ~List() {
	next_->List::prev_ = prev_;
	prev_->List::next_ = next_;
	next_ = nullptr;
	prev_ = nullptr;
    }

    const Container & next() const {
	return *next_;
    }

    Container & next() {
	return *next_;
    }

    const Container & prev() const {
	return *prev_;
    }

    Container & prev() {
	return *prev_;
    }
private:
    List(const List &&) = delete;
    List && operator =(const List &&) = delete;

    Container *next_;
    Container *prev_;
};

template<class Container, class ... Identifiers> class Lists;

template<class Container, class Identifier, class ... Identifiers>
class Lists<Container, Identifier, Identifiers ...>
    : public List<Container, Identifier>,
      public Lists<Container, Identifiers ...> {
public:
    Lists(Container *self) : List<Container, Identifier>(self),
			     Lists<Container, Identifiers ...>(self) {
    }

    template<class Id>
    const Container & next() const {
	return List<Container, Id>::next();
    }

    template<class Id>
    Container & next() {
	return List<Container, Id>::next();
    }

    template<class Id>
    const Container & prev() const {
	return List<Container, Id>::prev();
    }

    template<class Id>
    Container & prev() {
	return List<Container, Id>::prev();
    }
};

template<class Container>
class Lists<Container> {
public:
    Lists(Container *) { }
};

#endif // ##ifndef LIST_H
