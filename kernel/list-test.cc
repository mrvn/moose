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

// test list class

#include "list.h"

class All;
class Same;

class Foo : public Lists<Foo, All, Same> {
public:
    Foo() : Lists<Foo, All, Same>(this) { }
    ~Foo() { }

    void test() {
	Foo &n = next<All>();
	if (&n == this) { }
	const Foo &c = next<All>();
	if (&c == this) { }
	const Foo &c2 = c.prev<All>();
	if (&c2 == this) { }
    }
private:
    Foo(const Foo &&) = delete;
    Foo && operator =(const Foo &&) = delete;
};
