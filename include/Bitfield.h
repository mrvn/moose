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

/* templates to access and manage bit-fields
 *
 * example usage:
 * // header file
 * class MyBitfield : public Bitfield<ParentClass> {
 * public:
 *     using SingleBit = Bit<11>;
 *     using Multibit  = Bits<31, 12>;
 *     using SplitBit  = Field<Bit<9>, Bits<5, 4> >;
 *     static constexpr const SingleBit SINGLE{true};
 *     static constexpr M FIXED() {
 *         return M(SINGLE);
 *     };
 *     static constexpr M DEFAULT() {
 *         return M(SplitBit(0b010));
 *     };
 * }
 *
 * // must be in exactly one cc file in case the address is taken
 * constexpr const MyBitfield::SingleBit MyBitfield::SINGLE;
 *
 * // declaring a value
 * MyBitField field(MyBitfield::MultiBit(0b10));
 *
 * DEFAULT() is used to initialize a new bit-field but can be overridden by
 * specifying different values in the constructor. 
 *
 * FIXED() is used to force values after all the values from the constructor
 * are applied and can't be overridden. Use this to mask values that must be
 * written as zero or one.
 */

#ifndef BITFIELD_H
#define BITFIELD_H 1

#include <stddef.h>
#include <stdint.h>

struct Raw { };
static constexpr const Raw RAW{};

template<typename Base, size_t high, size_t low> class Bits {
public:
    static constexpr size_t SHIFT = high - low + 1;
    static constexpr uint32_t MASK = ((1U << SHIFT) - 1) << low;
    explicit constexpr Bits(uint32_t x) : raw_((x << low) & MASK) { }
    explicit constexpr Bits(Raw, uint32_t x) : raw_(x & MASK) { }
    constexpr uint32_t raw() const { return raw_; }
    constexpr uint32_t decode() const { return raw_ >> low; }
protected:
    uint32_t raw_;
};

template<typename Base, size_t num> class Bit : public Bits<Base, num, num> {
public:
    explicit constexpr Bit(Raw r, uint32_t x) : Bits<Base, num, num>(r, x) { }
    explicit constexpr Bit(bool x) : Bits<Base, num, num>(x ? 1 : 0) { }
    explicit operator bool() const { return Bits<Base, num, num>::raw_ != 0; }
    constexpr Bit operator !() const {
	return Bit(Bits<Base, num, num>::raw_ == 0);
    }
};

template<typename Base, typename ...> class Field;

template<typename Base, typename T, typename ... Ts>
class Field<Base, T, Ts ...> {
public:
    using F = Field<Base, T>;
    using Sub = Field<Base, Ts ...>;
    static constexpr size_t SHIFT = F::SHIFT + Sub::SHIFT;
    static constexpr uint32_t MASK = F::MASK | Sub::MASK;

    explicit constexpr Field(Raw, uint32_t x) : raw_(x & MASK) { }
    explicit constexpr Field(uint32_t x)
	: raw_(F(x >> Sub::SHIFT).raw() | Sub(x).raw()) { }
    constexpr uint32_t raw() const { return raw_; }
    constexpr uint32_t decode() const {
	return F(raw_).decode() << Sub::SHIFT | Sub(raw_).decode();
    }
private:
    uint32_t raw_;
};

template<typename Base, size_t num>
class Field<Base, Bit<Base, num> > {
public:
    using T = Bit<Base, num>;
    static constexpr size_t SHIFT = T::SHIFT;
    static constexpr uint32_t MASK = T::MASK;

    explicit constexpr Field(Raw, uint32_t x) : raw_(x & MASK) { }
    explicit constexpr Field(uint32_t x) : raw_(T(x).raw()) { }
    constexpr uint32_t raw() const { return raw_; }
    constexpr uint32_t decode() const { return T(raw_).decode(); }
private:
    uint32_t raw_;
};

template<typename Base, size_t high, size_t low>
class Field<Base, Bits<Base, high, low> > {
public:
    using T = Bits<Base, high, low>;
    static constexpr size_t SHIFT = T::SHIFT;
    static constexpr uint32_t MASK = T::MASK;

    explicit constexpr Field(Raw, uint32_t x) : raw_(x & MASK) { }
    explicit constexpr Field(uint32_t x) : raw_(T(x).raw()) { }
    constexpr uint32_t raw() const { return raw_; }
    constexpr uint32_t decode() const { return T(raw_).decode(); }
private:
    uint32_t raw_;
};

template<typename Base>
struct Field<Base> {
    static constexpr size_t SHIFT = 0;
    static constexpr uint32_t MASK = 0;
    explicit constexpr Field(Raw, uint32_t) { }
    explicit constexpr Field(uint32_t) { }
    constexpr uint32_t raw() const { return 0; }
    constexpr uint32_t decode() const { return 0; }
};

template<typename Base>
class Merge {
public:
    constexpr Merge(uint32_t set__, uint32_t mask__)
	: set_(set__), mask_(mask__) { }

    template<size_t num>
    constexpr Merge<Base>(const Bit<Base, num> t)
    : set_(t.raw()), mask_(t.MASK) { }

    template<size_t high, size_t low>
    constexpr Merge<Base>(const Bits<Base, high, low> t)
    : set_(t.raw()), mask_(t.MASK) { }

    template<typename ... Ts>
    constexpr Merge<Base>(const Field<Base, Ts ...> t)
    : set_(t.raw()), mask_(t.MASK) { }

    template<typename ... Ts>
    constexpr Merge<Base>(const Merge<Base> first, const Ts ... ts)
    : Merge<Base>(first + Merge<Base>(ts ...)) { }

    constexpr Merge() : set_(0), mask_(0) { }

    constexpr Merge operator +(const Merge other) const {
	return Merge(set_ | (other.set_ & ~mask_), mask_ | other.mask_);
    }

    constexpr uint32_t set() const { return set_; }
    constexpr uint32_t mask() const { return mask_; }
private:
    uint32_t set_;
    uint32_t mask_;
};

// 32bit Bitfield
template<typename Base>
class Bitfield {
public:
    template<size_t num> using Bit = Bit<Base, num>;
    template<size_t high, size_t low> using Bits = Bits<Base, high, low>;
    template<typename ... Ts> using Field = Field<Base, Ts ...>;
    using M = Merge<Base>;
    
    template<typename ... Ts>
    constexpr Bitfield(const Ts ... ts)
        : raw_(M(Base::FIXED(), ts ..., Base::DEFAULT()).set()) { }

    template<typename T>
    constexpr uint32_t is(const T t) const {
	return (raw() & t.MASK) == t.raw();
    }

    template<typename T>
    constexpr uint32_t get() const {
	return T(RAW, raw_).decode();
    }
    
    constexpr uint32_t raw() const {
        return raw_;
    }
    static constexpr M FIXED() {
	return M();
    };
    static constexpr M DEFAULT() {
	return M();
    };
protected:
    constexpr Bitfield(Raw, uint32_t x) : raw_(x) { }
private:
    uint32_t raw_;
};

#endif // #ifndef BITFIELD_H


