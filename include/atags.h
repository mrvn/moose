/* atags.h - ARM bootloader tags */
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

#ifndef MOOSE_KERNEL_ATAGS_H
#define MOOSE_KERNEL_ATAGS_H

#include <stdint.h>

namespace ATAG {
    enum Tag {NONE = 0x00000000, CORE = 0x54410001,
	      MEM = 0x54410002, VIDEOTEXT = 0x54410003,
	      RAMDISK = 0x54410004, INITRD2 = 0x54420005,
	      SERIAL = 0x54410006, REVISION = 0x54410007,
	      VIDEOLFB = 0x54410008, CMDLINE = 0x54410009,
	      
    };

    class Core;
    class Mem;
    class Videotext;
    class Ramdisk;
    class Initrd2;
    class Serial;
    class Revision;
    class VideoLFB;
    class Cmdline;
    
    class Header {
    public:
	Header const * next() const;
	Core      const & as_core() const;
	Mem       const & as_mem() const;
	Videotext const & as_videotext() const;
	Ramdisk   const & as_ramdisk() const;
	Initrd2   const & as_initrd2() const;
	Serial    const & as_serial() const;
	Revision  const & as_revision() const;
	VideoLFB  const & as_videolfb() const;
	Cmdline   const & as_cmdline() const;
	uint32_t tag_size;
	uint32_t tag;
    private:
	explicit Header();
    };
    
    class Core : public Header {
    public:
	struct {
	    uint32_t writable:1;
	};
	uint32_t pagesize;
	uint32_t rootdev;
    private:
	explicit Core();
    };

    class Mem : public Header {
    public:
	uint32_t size;
	uint32_t start;
    private:
	explicit Mem();
    };

    class Videotext : public Header {
    public:
	uint8_t  x;
	uint8_t  y;
	uint16_t video_page;
	uint8_t  video_mode;
	uint8_t  video_cols;
	uint16_t video_ega_bx;
        uint8_t  video_lines;
        uint8_t  video_isvga;
        uint16_t video_points;
    private:
	explicit Videotext();
    };

    class Ramdisk : public Header {
    public:
	struct {
	    uint32_t load:1;
	    uint32_t prompt:1;
	};
	uint32_t size;
	uint32_t start;
    private:
	explicit Ramdisk();
    };

    class Initrd2 : public Header {
    public:
	uint32_t size;
	uint32_t start;
    private:
	explicit Initrd2();
    };
    
    class Serial : public Header {
    public:
	uint32_t low;
	uint32_t high;
    private:
	explicit Serial();
    };

    class Revision : public Header {
    public:
	uint32_t rev;
    private:
	explicit Revision();
    };

    class VideoLFB : public Header {
    public:
        uint16_t lfb_width;
        uint16_t lfb_height;
        uint16_t lfb_depth;
        uint16_t lfb_linelength;
        uint32_t lfb_base;
        uint32_t lfb_size;
        uint8_t  red_size;
        uint8_t  red_pos;
        uint8_t  green_size;
        uint8_t  green_pos;
        uint8_t  blue_size;
        uint8_t  blue_pos;
        uint8_t  rsvd_size;
        uint8_t  rsvd_pos;
    private:
	explicit VideoLFB();
    };
    
    class Cmdline : public Header {
    public:
	char cmdline[1];
    private:
	explicit Cmdline();
    };
}

#endif // #ifndef MOOSE_KERNEL_ATAGS_H
