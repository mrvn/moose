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
private:
    List(const List &&) = delete;
    List && operator =(const List &&) = delete;

    Container *next_;
    Container *prev_;
};

#endif // ##ifndef LIST_H
