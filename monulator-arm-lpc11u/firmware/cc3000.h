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

#ifndef CC3000_H_1HDWY19Y
#define CC3000_H_1HDWY19Y

#include "cc3000_hci.h"

#include <cstddef>

enum simplelink_patches_request_t {
	SIMPLELINK_PATCHES_REQUEST_DEFAULT = 0,
	SIMPLELINK_PATCHES_REQUEST_FORCE_HOST = 1,
	SIMPLELINK_PATCHES_REQUEST_FORCE_NONE = 2,
};

struct read_buffer_size_response_t {
	hci_event_status_t status;
	size_t number_of_free_buffers;
	size_t buffer_length;
};

struct CC3000 {
	void enable(const simplelink_patches_request_t patches_request);
	void disable();
	
	read_buffer_size_response_t read_buffer_size();
	
	void check_for_unsolicited_events();

private:
	void reset_state();
};

extern CC3000 cc3000;

#endif /* end of include guard: CC3000_H_1HDWY19Y */
