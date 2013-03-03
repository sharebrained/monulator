/* 
 * TI CC3000 Wi-Fi module NVRAM API.
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

#ifndef CC3000_NVRAM_H_L3R8KYFP
#define CC3000_NVRAM_H_L3R8KYFP

#include "cc3000_hci.h"
#include "cc3000_wlan.h"

#include <cstdint>
#include <cstddef>

enum nvram_file_id_t : uint32_t {
	NVRAM_FILE_ID_DRIVER_SP = 4,
	NVRAM_FILE_ID_FIRMWARE_SP = 5,
	NVRAM_FILE_ID_MAC_ADDRESS = 6,
	NVRAM_FILE_ID_RM = 11,
	NVRAM_FILE_ID_MAX = 16,
};

struct nvram_read_sp_version_t {
	hci_event_status_t status;
	uint32_t package_id;
	uint32_t package_build_number;
};

struct CC3000NVRAM {
	nvram_read_sp_version_t read_sp_version();

	hci_event_status_t read(
		const nvram_file_id_t file_id,
		const size_t length,
		const size_t offset,
		void* const buffer
	);

	hci_event_status_t write(
		const nvram_file_id_t file_id,
		const size_t length,
		const size_t offset,
		const void* const buffer
	);

	hci_event_status_t read_mac_address(mac_address_t& mac_address);
	hci_event_status_t write_mac_address(const mac_address_t& mac_address);
	
	hci_event_status_t write_patch(
		const nvram_file_id_t file_id,
		const void* const buffer,
		const size_t length
	);
};

extern CC3000NVRAM cc3000_nvram;

#endif /* end of include guard: CC3000_NVRAM_H_L3R8KYFP */
