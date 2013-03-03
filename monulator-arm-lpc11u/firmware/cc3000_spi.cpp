/* 
 * TI CC3000 Wi-Fi module SPI interface.
 * 
 * Copyright (C) 2013 Jared Boone, ShareBrained Technology, Inc.
 * 
 * This file is part of the Monulator project.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cc3000_spi.h"

#include "lpc11u.h"

#include "serial.h"

enum spi_operation_t {
	SPI_OPERATION_WRITE = 1,
	SPI_OPERATION_READ = 3,
};

void CC3000SPI::init(const simplelink_patches_request_t patches_request) {
	// CC3000 IRQ will start asserted when enabled.
	// Wait for CC3000 IRQ to de-assert at start of wake-up period.
	// TODO: The timing around this seems ill-considered. This while()
	// should probably be moved elsewhere, perhaps to where the WLAN
	// ENable signal is asserted.
	//while( irq() == true );

	// Wait for CC3000 IRQ to assert once it is ready.
	while( irq() == false );

	// Send goofy first transaction, that must accomodate the CC3000's
	// startup-time SPI timing requirements.
	start_transaction();
	payload_remaining = 10;

	delay(800);		// 100us-ish.

	write_u8(SPI_OPERATION_WRITE);
	write_u8(0);		// HI(length)
	write_u8(5);		// LO(length)
	write_u8(0);		// Busy 0

	delay(800);		// 100us-ish.

	write_u8(0);		// Busy 1
	write_u8(HCI_TYPE_COMMAND);
	write_u8(HCI_COMMAND_OPCODE_SIMPLE_LINK_START & 0xff);
	write_u8(HCI_COMMAND_OPCODE_SIMPLE_LINK_START >> 8);
	write_u8(1);		// parameters_length
	write_u8(patches_request);

	end_transaction();
	
	cc3000_hci.wait_for(HCI_EVENT_OPCODE_SIMPLE_LINK_START);
	end_transaction();
}

void CC3000SPI::reset() {
	cs_deassert();
}

void CC3000SPI::cs_assert() {
	GPIO_PORT.CLR1 = 1 << 23;
	//serial_write_line("CS");
}

bool CC3000SPI::cs_asserted() {
	return (GPIO_PORT.PIN1 & (1 << 23)) == 0;
}

void CC3000SPI::cs_deassert() {
	GPIO_PORT.SET1 = 1 << 23;
	//serial_write_line("/CS");
}

bool CC3000SPI::irq() {
	const bool result = (GPIO_WORD.read(1, 27) == 0);
	
	static bool last_result = false;
	if( last_result != result ) {
		if( !result ) {
			//serial_write('/');
		}
		//serial_write_line("IRQ");
		last_result = result;
	}
	
	return result;
}

void CC3000SPI::start_transaction() {
	if( cs_asserted() ) {
		//serial_write_line("dangling transaction");
		// TODO: Make some sort of assert() function.
		while(true);
	} else {
		// Wait for TFE=1 (FIFO is empty)
		while( (SSP1.SR & 1) == 0 );
		cs_assert();

		//serial_write_line("[");
	
		// Wait until CC3000 is ready to receive data.
		while( irq() == false );
	}
}

size_t CC3000SPI::start_read() {
	//serial_write_string("read ");
	start_transaction();
	payload_remaining = 5;
	//serial_write_string("  spi=");
	write_u8(SPI_OPERATION_READ);
	write_u8(0);
	write_u8(0);
	size_t payload_length = read_u8() << 8;
	payload_length |= read_u8();
	payload_remaining = payload_length;
	//serial_write_line();
	return payload_remaining;
}

void CC3000SPI::start_write(const size_t payload_length) {
	const bool padding_required = (payload_length & 1) == 0;
	const size_t padded_payload_length = payload_length + (padding_required ? 1 : 0);
	//serial_write_string("write ");
	start_transaction();
	payload_remaining = 5;
	//serial_write_string("  spi=");
	write_u8(SPI_OPERATION_WRITE);
	write_u8(padded_payload_length >> 8);
	write_u8(padded_payload_length & 0xff);
	write_u8(0);
	write_u8(0);
	payload_remaining = padded_payload_length;
	//serial_write_line();
}

void CC3000SPI::write(const void* const data, const size_t length) {
	//serial_write_string("  block=");
	const uint8_t* const p = (const uint8_t*)data;
	for(size_t i=0; i<length; i++) {
		if( p ) {
			write_u8(p[i]);
		} else {
			write_u8(0);
		}
	}
	//serial_write_line();
}

void CC3000SPI::write_u8(const uint8_t value) {
	//serial_write_hex(value, 2);
	//serial_write(' ');
	transfer_byte(value);
}

void CC3000SPI::read(void* const data, const size_t length) {
	//serial_write_string("  block=");
	uint8_t* const p = (uint8_t*)data;
	for(size_t i=0; i<length; i++) {
		const uint8_t c = read_u8();
		if( p != nullptr ) {
			p[i] = c;
		}
	}
	//serial_write_line();
}

uint8_t CC3000SPI::read_u8() {
	const uint8_t value = transfer_byte(0);
	//serial_write_hex(value, 2);
	//serial_write(' ');
	return value;
}

void CC3000SPI::end_transaction() {
	if( payload_remaining ) {
		//serial_write_string("  pad=");
		write(nullptr, payload_remaining);
		//serial_write_line();
	}
	
	//serial_write_line("]");
	
	// Wait for TFE=1 (FIFO is empty)
	while( (SSP1.SR & 1) == 0 );
	cs_deassert();
}

uint8_t CC3000SPI::transfer_byte(const uint8_t tx_byte) {
	if( payload_remaining > 0 ) {
		// Wait for TFE=1 (FIFO is empty)
		while( (SSP1.SR & 1) == 0 );
	
		SSP1.DR = tx_byte;
	
		// Wire for RNE=1 (FIFO not empty)
		while( (SSP1.SR & 4) == 0 );
	
		const uint8_t rx_byte = SSP1.DR & 0xFF;
		payload_remaining -= 1;
		return rx_byte;
	} else {
		serial_write_line("payload overrun");
		return 0;
	}
}

CC3000SPI cc3000_spi;
