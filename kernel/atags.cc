/* atags.cc - ARM bootloader tags */
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

/* Reference:
 * http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html
 */

#include <stddef.h>

#include <atags.h>

namespace ATAG {
    Header const * Header::next() const {
	if (tag == NONE) {
	    return NULL;
	}
	return (Header const *)(((uint32_t*)this) + tag_size);
    }

    Core const & Header::as_core() const {
	return *(Core const *)this;
    }
    
    Mem const & Header::as_mem() const {
	return *(Mem const *)this;
    }
    
    Videotext const & Header::as_videotext() const {
	return *(Videotext const *)this;
    }
    
    Ramdisk const & Header::as_ramdisk() const {
	return *(Ramdisk const *)this;
    }
    
    Initrd2 const & Header::as_initrd2() const {
	return *(Initrd2 const *)this;
    }
    
    Serial const & Header::as_serial() const {
	return *(Serial const *)this;
    }
    
    Revision const & Header::as_revision() const {
	return *(Revision const *)this;
    }
    
    VideoLFB const & Header::as_videolfb() const {
	return *(VideoLFB const *)this;
    }
    
    Cmdline const & Header::as_cmdline() const {
	return *(Cmdline const *)this;
    }
};
