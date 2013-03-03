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

#include "cc3000_nvram.h"

#include "cc3000_hci.h"

#include <string.h>

nvram_read_sp_version_t CC3000NVRAM::read_sp_version(
) {
	cc3000_hci.start(HCI_COMMAND_OPCODE_READ_SP_VERSION);
	cc3000_hci.end();
	
	const auto status = cc3000_hci.wait_for(HCI_EVENT_OPCODE_READ_SP_VERSION);
	const uint32_t version_packed = cc3000_hci.read_u32();
	cc3000_hci.end();
	
	return {
		status,
		(version_packed >> 16) & 0xff,
		(version_packed >> 24) & 0xff,
	};
}

hci_event_status_t CC3000NVRAM::read(
	const nvram_file_id_t file_id,
	const size_t requested_length,
	const size_t offset,
	void* const buffer
) {
	cc3000_hci.start(HCI_COMMAND_OPCODE_NVRAM_READ, 12);
	cc3000_hci.write_u32(file_id);
	cc3000_hci.write_u32(requested_length);
	cc3000_hci.write_u32(offset);
	cc3000_hci.end();
	
	const auto status = cc3000_hci.wait_for(HCI_EVENT_OPCODE_NVRAM_READ);
	cc3000_hci.end();

	if( status == HCI_EVENT_STATUS_OK ) {
		hci_response_t response;
		cc3000_hci.wait_for_data(response);
		cc3000_hci.read(nullptr, response.data.args_length);
		cc3000_hci.read(buffer, response.payload_length - response.data.args_length);
		cc3000_hci.end();
	}
	
	return status;
}

hci_event_status_t CC3000NVRAM::write(
	const nvram_file_id_t file_id,
	const size_t length,
	const size_t offset,
	const void* const buffer
) {
	cc3000_hci.start(HCI_DATA_OPCODE_NVRAM_WRITE, 16, length);
	cc3000_hci.write_u32(file_id);
	cc3000_hci.write_u32(12);
	cc3000_hci.write_u32(length);
	cc3000_hci.write_u32(offset);
	cc3000_hci.write(buffer, length);
	cc3000_hci.end();
	
	const auto status = cc3000_hci.wait_for(HCI_EVENT_OPCODE_NVRAM_WRITE);
	cc3000_hci.end();
	
	return status;
}

hci_event_status_t CC3000NVRAM::read_mac_address(
	mac_address_t& mac_address
) {
	return cc3000_nvram.read(
		NVRAM_FILE_ID_MAC_ADDRESS,
		sizeof(mac_address),
		0,
		&mac_address
	);
}

hci_event_status_t CC3000NVRAM::write_mac_address(
	const mac_address_t& mac_address
) {
	return cc3000_nvram.write(
		NVRAM_FILE_ID_MAC_ADDRESS,
		sizeof(mac_address),
		0,
		&mac_address
	);
}

hci_event_status_t CC3000NVRAM::write_patch(
	const nvram_file_id_t file_id,
	const void* const buffer,
	const size_t length
) {
	const size_t transfer_length_maximum = 512;
	
	const uint8_t* buffer_data = (uint8_t*)buffer;
	size_t offset = 0;
	size_t remaining = length;
	hci_event_status_t status = HCI_EVENT_STATUS_OK;
	while( remaining > 0 ) {
		const size_t transfer_length = (remaining < transfer_length_maximum) ? remaining : transfer_length_maximum;
		status = write(file_id, transfer_length, offset, &buffer_data[offset]);
		offset += transfer_length;
		remaining -= transfer_length;
	}
	
	return status;
}

CC3000NVRAM cc3000_nvram;
