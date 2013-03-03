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

#ifndef CC3000_SPI_H_WL6JNIXW
#define CC3000_SPI_H_WL6JNIXW

#include "cc3000.h"

#include <cstdint>
#include <cstddef>

struct CC3000SPI {
	void init(const simplelink_patches_request_t patches_request);
	void reset();
	
	size_t start_read();
	void read(void* const data, const size_t length);
	uint8_t read_u8();
	
	void start_write(const size_t length);
	void write(const void* const data, const size_t length);
	void write_u8(const uint8_t value);
	
	void end_transaction();

	bool irq();

private:
	size_t payload_remaining = 0;
	
	void cs_assert();
	void cs_deassert();
	bool cs_asserted();

	void start_transaction();
	uint8_t transfer_byte(const uint8_t tx_byte);
};

extern CC3000SPI cc3000_spi;

#endif /* end of include guard: CC3000_SPI_H_WL6JNIXW */
