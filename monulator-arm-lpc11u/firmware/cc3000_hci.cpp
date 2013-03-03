/* 
 * TI CC3000 Wi-Fi module HCI layer.
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

#include "cc3000_hci.h"

#include "cc3000_spi.h"
#include "cc3000_wlan.h"

#include <string.h>

// TODO: Qualify for big-endian architectures
static constexpr uint32_t htole32(const uint32_t value) {
	return value;
}

// TODO: Qualify for big-endian architectures
static constexpr uint16_t htole16(const uint16_t value) {
	return value;
}

// TODO: Qualify for big-endian architectures
static constexpr uint32_t le32toh(const uint32_t value) {
	return value;
}

// TODO: Qualify for big-endian architectures
static constexpr uint16_t le16toh(const uint16_t value) {
	return value;
}

void CC3000HCI::start(
	const hci_command_opcode_t opcode,
	const size_t hci_payload_length
) {
	const size_t hci_length = 4 + hci_payload_length;
	
	cc3000_spi.start_write(hci_length);
	write_u8(HCI_TYPE_COMMAND);
	write_u16(opcode);
	write_u8(hci_payload_length);
}

void CC3000HCI::start(
	const hci_data_opcode_t opcode,
	const size_t args_length,
	const size_t data_length
) {
	const size_t hci_header_length = 5;
	const size_t hci_payload_length = args_length + data_length;
	const size_t hci_length = hci_header_length + hci_payload_length;
	
	cc3000_spi.start_write(hci_length);
	write_u8(HCI_TYPE_DATA);
	write_u8(opcode);
	write_u8(args_length);
	write_u16(hci_payload_length);
}

void CC3000HCI::write_string(const char* const value) {
	write(value, strlen(value));
}

void CC3000HCI::write_u8(const uint8_t value) {
	write(&value, 1);
}

void CC3000HCI::write_u16(const uint16_t value) {
	const uint16_t value_le = htole16(value);
	write(&value_le, 2);
}

void CC3000HCI::write_u32(const uint32_t value) {
	const uint32_t value_le = htole32(value);
	write(&value_le, 4);
}

void CC3000HCI::write(const void* const data, const size_t length) {
	cc3000_spi.write(data, length);
}

void CC3000HCI::end() {
	cc3000_spi.end_transaction();
}

void CC3000HCI::read(void* const data, const size_t length) {
	cc3000_spi.read(data, length);
}

uint8_t CC3000HCI::read_u8() {
	uint8_t value;
	read(&value, 1);
	return value;
}

uint16_t CC3000HCI::read_u16() {
	uint16_t value;
	read(&value, 2);
	return le16toh(value);
}

uint32_t CC3000HCI::read_u32() {
	uint32_t value;
	read(&value, 4);
	return le32toh(value);
}

void CC3000HCI::send_patch(
	const hci_patch_opcode_t opcode,
	const void* const data,
	const size_t length
) {
	const size_t transfer_maximum_size = 512;
	
	const uint8_t* data_ptr = (uint8_t*)data;
	size_t remaining = length;
	while( true ) {
		const size_t transfer_length = std::min(remaining, transfer_maximum_size);
		const size_t hci_payload_length = 2 + transfer_length;
		const size_t hci_length = 4 + hci_payload_length;

		cc3000_spi.start_write(hci_length);
		write_u8(HCI_TYPE_PATCH);
		write_u8(opcode);
		write_u16(hci_payload_length);
		write_u16(transfer_length);
		write(data_ptr, transfer_length);
		end();

		remaining -= transfer_length;
		data_ptr += transfer_length;
		
		if( remaining == 0 ) {
			break;
		}
	}
}

hci_event_status_t CC3000HCI::wait_for(const hci_event_opcode_t match_opcode) {
	while(true) {
		hci_response_t response;
		if( get_next_event(response) ) {
			if( response.type == HCI_TYPE_EVENT ) {
				if( response.event.opcode == match_opcode ) {
					return response.event.status;
				}
			}
			end();
		}
	}
}

void CC3000HCI::wait_for_data(hci_response_t& response) {
	while(true) {
		if( get_next_event(response) ) {
			if( response.type == HCI_TYPE_DATA ) {
				return;
			}
		}
	}
}

void CC3000HCI::handle_unsolicited_events() {
	hci_response_t response;
	if( get_next_event(response) ) {
		end();
	}
}

bool CC3000HCI::read_header(
	const size_t payload_length,
	hci_response_t& header
) {
	bool result = false;
	
	//serial_write_string("hci(");
	if( payload_length >= 5 ) {
		header.type = (hci_type_t)read_u8();
		//serial_write_string("type=");
		//serial_write_hex(header.type, 2);
		switch( header.type ) {
		case HCI_TYPE_EVENT:
			{
				header.event.opcode = (hci_event_opcode_t)read_u16();
				// It appears the status byte is counted in the payload, though source
				// and documentation indicates it's considered part of the HCI header.
				// I think it's a cowardly attempt to align the args so arg struct
				// packing is less complicated.
				// TODO: Move reading the status somewhere else -- layer above.
				header.payload_length = read_u8() - 1;
				header.event.status = (hci_event_status_t)read_u8();
				//serial_write_string(",evt=");
				//serial_write_hex(header.event.opcode, 4);
				//serial_write_string(",len=");
				//serial_write_hex(header.payload_length, 2);
				//serial_write_string(",status=");
				//serial_write_hex(header.event.status, 2);
				result = true;
			}
			break;
		
		case HCI_TYPE_DATA:
			header.data.opcode = (hci_data_opcode_t)read_u8();
			header.data.args_length = read_u8();
			header.payload_length = read_u16();
			//serial_write_string(",data=");
			//serial_write_hex(header.data.opcode, 2);
			//serial_write_string(",args_len=");
			//serial_write_hex(header.data.args_length, 2);
			//serial_write_string(",payload_len=");
			//serial_write_hex(header.payload_length, 4);
			result = true;
			break;
	
		default:
			// Unhandled type.
			//serial_write_string(",unknown type");
			break;
		}
	} else {
		//serial_write_string("!length");
	}
	//serial_write_line(")");
	
	return result;
}

bool CC3000HCI::handle_unsolicited_event(const hci_response_t& response) {
	switch( response.event.opcode ) {
	case HCI_EVENT_OPCODE_UNSOLICITED_FREE_BUFFER:
		
		end();
		break;
	
	case HCI_EVENT_OPCODE_PATCHES_REQUEST:
		{
			const hci_patch_opcode_t opcode = (hci_patch_opcode_t)read_u8();
			end();
			handle_patches_request(opcode);
		}
		break;
		
	case HCI_EVENT_OPCODE_UNSOLICITED_WLAN_CONNECT:
	case HCI_EVENT_OPCODE_UNSOLICITED_WLAN_DISCONNECT:
		end();
		cc3000_wlan.handle_unsolicited_event(response.event.opcode);
		break;
	
	case HCI_EVENT_OPCODE_UNSOLICITED_WLAN_DHCP:
		{
			const uint32_t ip_address = read_u32();
			const uint32_t subnet = read_u32();
			const uint32_t default_gateway = read_u32();
			const uint32_t dhcp_server_address = read_u32();
			const uint32_t dns_server_address = read_u32();
		}
		end();
		cc3000_wlan.handle_unsolicited_event(response.event.opcode);
		break;
		
	default:
		return false;
	}
	
	return true;
}

bool CC3000HCI::get_next_event(hci_response_t& hci_header) {
	// Retreive events (if any) until a non-unsolicited event is encountered.
	while( cc3000_spi.irq() ) {
		const size_t spi_payload_length = cc3000_spi.start_read();
		if( read_header(spi_payload_length, hci_header) ) {
			if( handle_unsolicited_event(hci_header) == false ) {
				return true;
			}
		} else {
			end();
		}
	}
	
	return false;
}

void CC3000HCI::wait_for_next_event(hci_response_t& hci_header) {
	while( true ) {
		if( get_next_event(hci_header) ) {
			return;
		}
	}
}

bool CC3000HCI::handle_patches_request(const hci_patch_opcode_t opcode) {
	switch(opcode) {
	case HCI_PATCH_OPCODE_DRIVER_REQUEST:
		//serial_write_line("patch driver");
		send_patch(opcode, nullptr, 0);
		break;

	case HCI_PATCH_OPCODE_FIRMWARE_REQUEST:
		//serial_write_line("patch firmware");
		send_patch(opcode, nullptr, 0);
		break;

	case HCI_PATCH_OPCODE_BOOTLOADER_REQUEST:
		//serial_write_line("patch bootloader");
		send_patch(opcode, nullptr, 0);
		break;
	
	default:
		//serial_write_string("patch unknown: ");
		//serial_write_hex(opcode);
		//serial_write_line();
		break;
	}
	
	return true;
}

CC3000HCI cc3000_hci;
