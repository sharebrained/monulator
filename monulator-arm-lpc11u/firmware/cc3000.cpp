/* 
 * TI CC3000 Wi-Fi module top-level abstraction.
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

#include "cc3000.h"

#include "cc3000_spi.h"
#include "cc3000_hci.h"
#include "cc3000_wlan.h"

#include "lpc11u.h"

void CC3000::reset_state() {
	cc3000_wlan.reset_state();
}

void CC3000::enable(const simplelink_patches_request_t patches_request) {
	reset_state();

	// Apply power.
	GPIO_PORT.SET1 = 1 << 22;

	cc3000_spi.init(patches_request);
}

void CC3000::disable() {
	cc3000_spi.reset();

	GPIO_PORT.CLR1 = 1 << 22;
	reset_state();
}

read_buffer_size_response_t CC3000::read_buffer_size() {
	cc3000_hci.start(HCI_COMMAND_OPCODE_READ_BUFFER_SIZE);
	cc3000_hci.end();
	
	const auto status = cc3000_hci.wait_for(HCI_EVENT_OPCODE_READ_BUFFER_SIZE);
	const uint8_t number_of_free_buffers = cc3000_hci.read_u8();
	const uint16_t buffer_length = cc3000_hci.read_u16();
	
	return {
		status,
		number_of_free_buffers,
		buffer_length,
	};
}

void CC3000::check_for_unsolicited_events() {
	cc3000_hci.handle_unsolicited_events();
}

CC3000 cc3000;
