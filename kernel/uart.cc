/* uart.cc - serial console driver */
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

/* Reference material:
 * http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 * Chapter 13: UART
 */

#include <stddef.h>
#include <panic.h>
#include <memory.h>
#include <gpio.h>

#include <uart.h>

namespace UART {
    enum {
	BASE = 0x201000,

	DR     = 0x00, // data
	RSRECR = 0x04,
	FR     = 0x18, // flag
	IBRD   = 0x24, // integer BAUD rate divisor
	FBRD   = 0x28, // fractional BAUD rate divisior
	LCRH   = 0x2C, // line control
	CR     = 0x30, // control
	IFLS   = 0x34, // interupt FIFO level select
	IMSC   = 0x38, // interupt mask set/clear
	RIS    = 0x3C, // raw interupt status
	MIS    = 0x40, // masked interupt status
	ICR    = 0x44, // interupt clear
	DMACR  = 0x48, // DMA control
	SIZE   = 0x4C  // ignore test registers	
    };

    // GPIO pin definition
    enum { NUM_PINS = 2 };
    GPIO::Pin pins[NUM_PINS] = {
	(GPIO::Pin){14, GPIO::ALT0, false, GPIO::NONE},
	(GPIO::Pin){15, GPIO::ALT0, false, GPIO::NONE},
    };

    // Error codes
    enum { ERROR_BIT_FRAME, ERROR_BIT_PARITY, ERROR_BIT_BREAK, ERROR_BIT_OVER };
    enum Errors {
	ERROR_NONE    = 0,
	ERROR_FRAMING = 1 << ERROR_BIT_FRAME,
	ERROR_PARITY  = 1 << ERROR_BIT_PARITY,
	ERROR_BREAK   = 1 << ERROR_BIT_BREAK,
	ERROR_OVERRUN = 1 << ERROR_BIT_OVER,
    };

    // Flags
    enum { FLAG_BIT_CTS, FLAG_BIT_BUSY = 3, FLAG_BIT_RECV_EMPTY,
	   FLAG_BIT_TRANS_FULL, FLAG_BIT_RECV_FULL, FLAG_BIT_TRANS_EMPTY };
    enum Flag {
	FLAG_CTS         = 1 << FLAG_BIT_CTS,
	FLAG_BUSY        = 1 << FLAG_BIT_BUSY,
	FLAG_RECV_EMPTY  = 1 << FLAG_BIT_RECV_EMPTY,
	FLAG_TRANS_FULL  = 1 << FLAG_BIT_TRANS_FULL,
	FLAG_RECV_FULL   = 1 << FLAG_BIT_RECV_FULL,
	FLAG_TRANS_EMPTY = 1 << FLAG_BIT_TRANS_EMPTY
    };

    // Line control
    enum { LINECTRL_BIT_BREAK, LINECTRL_BIT_PARITY, LINECTRL_BIT_EVEN,
	   LINECTRL_BIT_TWO, LINECTRL_BIT_FIFO, LINECTRL_BIT_LENGTH,
	   LINECTRL_BIT_STICK = 7 };
    enum LineCtrl {
            LINECTRL_NONE          = 0,
            LINECTRL_SEND_BREAK    = 1 << LINECTRL_BIT_BREAK,
            LINECTRL_ENABLE_PARITY = 1 << LINECTRL_BIT_PARITY,
            LINECTRL_EVEN_PARITY   = 1 << LINECTRL_BIT_EVEN,
            LINECTRL_TWO_STOP_BITS = 1 << LINECTRL_BIT_TWO,
            LINECTRL_ENABLE_FIFO   = 1 << LINECTRL_BIT_FIFO,
            LINECTRL_WORD_5_BITS   = 0 << LINECTRL_BIT_LENGTH,
            LINECTRL_WORD_6_BITS   = 1 << LINECTRL_BIT_LENGTH,
            LINECTRL_WORD_7_BITS   = 2 << LINECTRL_BIT_LENGTH,
            LINECTRL_WORD_8_BITS   = 3 << LINECTRL_BIT_LENGTH,
            LINECTRL_WORD_MASK     = 3 << LINECTRL_BIT_LENGTH,
            LINECTRL_STICK_PARITY  = 1 << LINECTRL_BIT_STICK
        };

    // control
    enum { CTRL_BIT_ENABLE,
	   CTRL_BIT_LOOP = 7, CTRL_BIT_TRANSMIT, CTRL_BIT_RECEIVE };
    enum Ctrl {
	CTRL_NONE     = 0,
	CTRL_ENABLE   = 1 << CTRL_BIT_ENABLE,
	CTRL_LOOPBACK = 1 << CTRL_BIT_LOOP,
	CTRL_TRANSMIT = 1 << CTRL_BIT_TRANSMIT,
	CTRL_RECEIVE  = 1 << CTRL_BIT_RECEIVE
    };

    // interupt fifo select
    enum { FIFO_BIT_TXIFLSEL, FIFO_BIT_RXIFLSEL = 3 };
    enum Fifo {
	FIFO_TRANS_1_8  = 0 << FIFO_BIT_TXIFLSEL,
	FIFO_TRANS_2_8  = 1 << FIFO_BIT_TXIFLSEL,
	FIFO_TRANS_4_8  = 2 << FIFO_BIT_TXIFLSEL,
	FIFO_TRANS_6_8  = 3 << FIFO_BIT_TXIFLSEL,
	FIFO_TRANS_7_8  = 4 << FIFO_BIT_TXIFLSEL,
	FIFO_TRANS_MASK = 7 << FIFO_BIT_TXIFLSEL,
	FIFO_RECV_1_8   = 0 << FIFO_BIT_RXIFLSEL,
	FIFO_RECV_2_8   = 1 << FIFO_BIT_RXIFLSEL,
	FIFO_RECV_4_8   = 2 << FIFO_BIT_RXIFLSEL,
	FIFO_RECV_6_8   = 3 << FIFO_BIT_RXIFLSEL,
	FIFO_RECV_7_8   = 4 << FIFO_BIT_RXIFLSEL,
	FIFO_RECV_MASK  = 7 << FIFO_BIT_RXIFLSEL,
    };

    // interrupts
    enum { IRQ_BIT_CTS_IRQ = 1, IRQ_BIT_RECV_IRQ = 4, IRQ_BIT_TRANS_IRQ,
	   IRQ_BIT_RECV_TIMEOUT, IRQ_BIT_FRAME, IRQ_BIT_PARITY, IRQ_BIT_BREAK,
	   IRQ_BIT_OVERRUN };
    enum Flags {
	IRQ_NONE          = 0,
	IRQ_CTS           = 1 << IRQ_BIT_CTS_IRQ,
	IRQ_RECV          = 1 << IRQ_BIT_RECV_IRQ,
	IRQ_TRANS         = 1 << IRQ_BIT_TRANS_IRQ,
	IRQ_RECV_TIMEOUT  = 1 << IRQ_BIT_RECV_TIMEOUT,
	IRQ_FRAME_ERROR   = 1 << IRQ_BIT_FRAME,
	IRQ_PARITY_ERROR  = 1 << IRQ_BIT_PARITY,
	IRQ_BREAK_ERROR   = 1 << IRQ_BIT_BREAK,
	IRQ_OVERRUN_ERROR = 1 << IRQ_BIT_OVERRUN,
	IRQ_NO_TRANS      = IRQ_CTS | IRQ_RECV | IRQ_RECV_TIMEOUT
	                    | IRQ_FRAME_ERROR | IRQ_PARITY_ERROR
	                    | IRQ_BREAK_ERROR | IRQ_OVERRUN_ERROR,
	IRQ_ALL           = IRQ_NO_TRANS | IRQ_TRANS,
    };

	
    Memory::Peripheral uart;

    enum { UART_CLOCK = 3000000 };
    
    // set the integer and fractional part of the BAUD rate
    void set_baud(uint32_t rate) {
	// Divider = UART_CLOCK / (16 * rate)
        // Fraction part register = (Fractional part * 64) + 0.5
        // UART_CLOCK = 3000000
	uint32_t t = UART_CLOCK * 4 / rate;
	uart.set(IBRD, t / 64);
	uart.set(FBRD, t % 64);
    }

    // get the BAUD rate
    uint32_t get_baud() {
	uint32_t i = uart.get(IBRD);
	uint32_t f = uart.get(FBRD);
	return UART_CLOCK * 4 / (i * 64 + f);
    }
    
    
    void init() {
	// allocate peripheral
	uart = Memory::alloc_peripheral(BASE, SIZE);
	if (!uart.valid()) {
	    panic("UART::init(): peripheral not available\n");
	}
	
	// allocate and configure pins
	if (!GPIO::alloc(pins, NUM_PINS)) {
	    panic("UART::init(): gpio pins not available\n");
	}
	
	// clear interrupts
	uart.set(IMSC, IRQ_NONE);
	uart.set(ICR, IRQ_NONE);

	// set BAUD rate
	set_baud(115200);

	// enable FIFO and 8n1
	uart.set(LCRH, LINECTRL_ENABLE_FIFO | LINECTRL_WORD_8_BITS);

	// enable UART + receive & transfer part of UART.
	uart.set(CR, CTRL_ENABLE | CTRL_TRANSMIT | CTRL_RECEIVE);
    }

    /*
     * Transmit a byte via UART0.
     * byte: byte to send.
     */
    void putc(uint8_t byte) {
	while(uart.get(FR) & FLAG_TRANS_FULL) { }
	uart.set(DR, byte);
	Memory::data_sync_barrier();
    }

    /*
     * Receive a byte via UART0.
     * returns: byte received.
     */
    uint8_t getc(void) {
	while(uart.get(FR) & FLAG_RECV_EMPTY) { }
	return uart.get(DR);
    }

    /*
     * Check if data is available via UART0.
     * returns: data available?
     */
    bool poll(void) {
	return !(uart.get(FR) & FLAG_RECV_EMPTY);
    }

    /*
     * print a string to the UART one character at a time
     * str: 0-terminated string
     */
    void puts(const char *str) {
	while(*str) {
	    putc(*str++);
	}
    }

    /*
     * print an uint32_t as hex to the UART one character at a time
     * data: uint32_t to print
     */
    void put_uint32(uint32_t data) {
	const char hex[] = "0123456789ABCDEF";
        putc('0');
        putc('x');
        for(int i = 28; i >= 0; i -= 4) {
            UART::putc(hex[((data) >> i) % 16]);
        }
    }
};
