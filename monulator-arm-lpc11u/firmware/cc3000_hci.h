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

#ifndef CC3000_HCI_H_XBGVM3NF
#define CC3000_HCI_H_XBGVM3NF

#include <cstdint>
#include <cstddef>

enum hci_event_mask_t {
	HCI_EVENT_MASK_WLAN_CONNECT = 0x0001,
	HCI_EVENT_MASK_WLAN_DISCONNECT = 0x0002,
	HCI_EVENT_MASK_WLAN_INIT = 0x0004,
	HCI_EVENT_MASK_WLAN_TX_COMPLETE = 0x0008,
	HCI_EVENT_MASK_WLAN_DHCP = 0x0010,
	HCI_EVENT_MASK_WLAN_ASYNC_PING_REPORT = 0x0040,
	HCI_EVENT_MASK_WLAN_ASYNC_SIMPLE_CONFIG_DONE = 0x0080,
	HCI_EVENT_MASK_DATA_FREE_BUFFER = 0x0100,
	HCI_EVENT_MASK_WLAN_KEEPALIVE = 0x0200,
};

enum hci_type_t {
	HCI_TYPE_INVALID = 0,
	HCI_TYPE_COMMAND = 1,
	HCI_TYPE_DATA = 2,
	HCI_TYPE_PATCH = 3,
	HCI_TYPE_EVENT = 4,
};

enum hci_command_opcode_t {
	HCI_COMMAND_OPCODE_INVALID = 0,
	HCI_COMMAND_OPCODE_WLAN_CONNECT = 0x0001,
	HCI_COMMAND_OPCODE_WLAN_IOCTL_SET_CONNECTION_POLICY = 0x0004,
	HCI_COMMAND_OPCODE_WLAN_IOCTL_DEL_PROFILE = 0x0006,
	HCI_COMMAND_OPCODE_SET_EVENT_MASK = 0x0008,
	HCI_COMMAND_OPCODE_WLAN_SMARTCONFIG_SET_PREFIX = 0x000C,
	HCI_COMMAND_OPCODE_NVRAM_READ = 0x0201,
	HCI_COMMAND_OPCODE_READ_SP_VERSION = 0x0207,
	HCI_COMMAND_OPCODE_SOCKET = 0x1001,
	HCI_COMMAND_OPCODE_RECV = 0x1004,
	HCI_COMMAND_OPCODE_CONNECT = 0x1007,
	HCI_COMMAND_OPCODE_SELECT = 0x1008,
	HCI_COMMAND_OPCODE_CLOSE_SOCKET = 0x100b,
	HCI_COMMAND_OPCODE_GETHOSTBYNAME = 0x1010,
	HCI_COMMAND_OPCODE_SIMPLE_LINK_START = 0x4000,
	HCI_COMMAND_OPCODE_READ_BUFFER_SIZE = 0x400b,
};

enum hci_data_opcode_t {
	HCI_DATA_OPCODE_SEND = 0x81,
	HCI_DATA_OPCODE_NVRAM_WRITE = 0x0090,
};

enum hci_patch_opcode_t {
	HCI_PATCH_OPCODE_DRIVER_REQUEST = 1,
	HCI_PATCH_OPCODE_FIRMWARE_REQUEST = 2,
	HCI_PATCH_OPCODE_BOOTLOADER_REQUEST = 3,
};

enum hci_event_opcode_t {
	HCI_EVENT_OPCODE_INVALID = 0,
	HCI_EVENT_OPCODE_WLAN_CONNECT = HCI_COMMAND_OPCODE_WLAN_CONNECT,
	HCI_EVENT_OPCODE_WLAN_IOCTL_SET_CONNECTION_POLICY = HCI_COMMAND_OPCODE_WLAN_IOCTL_SET_CONNECTION_POLICY,
	HCI_EVENT_OPCODE_WLAN_IOCTL_DEL_PROFILE = HCI_COMMAND_OPCODE_WLAN_IOCTL_DEL_PROFILE,
	HCI_EVENT_OPCODE_SET_EVENT_MASK = HCI_COMMAND_OPCODE_SET_EVENT_MASK,
	HCI_EVENT_OPCODE_WLAN_SMARTCONFIG_SET_PREFIX = HCI_COMMAND_OPCODE_WLAN_SMARTCONFIG_SET_PREFIX,
	HCI_EVENT_OPCODE_NVRAM_READ = HCI_COMMAND_OPCODE_NVRAM_READ,
	HCI_EVENT_OPCODE_NVRAM_WRITE = 0x0202,
	HCI_EVENT_OPCODE_READ_SP_VERSION = HCI_COMMAND_OPCODE_READ_SP_VERSION,
	HCI_EVENT_OPCODE_PATCHES_REQUEST = 0x1000,
	HCI_EVENT_OPCODE_SOCKET = HCI_COMMAND_OPCODE_SOCKET,
	HCI_EVENT_OPCODE_RECV = HCI_COMMAND_OPCODE_RECV,
	HCI_EVENT_OPCODE_CONNECT = HCI_COMMAND_OPCODE_CONNECT,
	HCI_EVENT_OPCODE_SELECT = HCI_COMMAND_OPCODE_SELECT,
	HCI_EVENT_OPCODE_CLOSE_SOCKET = HCI_COMMAND_OPCODE_CLOSE_SOCKET,
	HCI_EVENT_OPCODE_GETHOSTBYNAME = HCI_COMMAND_OPCODE_GETHOSTBYNAME,
	HCI_EVENT_OPCODE_SIMPLE_LINK_START = HCI_COMMAND_OPCODE_SIMPLE_LINK_START,
	HCI_EVENT_OPCODE_READ_BUFFER_SIZE = HCI_COMMAND_OPCODE_READ_BUFFER_SIZE,
	HCI_EVENT_OPCODE_UNSOLICITED_FREE_BUFFER = 0x4100,
	HCI_EVENT_OPCODE_UNSOLICITED_WLAN_BASE = 0x8000,
	HCI_EVENT_OPCODE_UNSOLICITED_WLAN_CONNECT = HCI_EVENT_OPCODE_UNSOLICITED_WLAN_BASE + 0x0001,
	HCI_EVENT_OPCODE_UNSOLICITED_WLAN_DISCONNECT = HCI_EVENT_OPCODE_UNSOLICITED_WLAN_BASE + 0x0002,
	HCI_EVENT_OPCODE_UNSOLICITED_WLAN_INIT = HCI_EVENT_OPCODE_UNSOLICITED_WLAN_BASE + 0x0004,
	HCI_EVENT_OPCODE_UNSOLICITED_WLAN_DHCP = HCI_EVENT_OPCODE_UNSOLICITED_WLAN_BASE + 0x0010,
};

enum hci_event_status_t {
	HCI_EVENT_STATUS_OK = 0,
};

struct hci_event_t {
	hci_event_opcode_t opcode;
	hci_event_status_t status;
};

struct hci_data_t {
	hci_data_opcode_t opcode;
	size_t args_length;
};

struct hci_response_t {
	hci_type_t type;
	union {
		hci_event_t event;
		hci_data_t data;
	};
	size_t payload_length;
};

struct CC3000HCI {
	void start(const hci_command_opcode_t opcode, const size_t payload_length=0);
	void start(const hci_data_opcode_t opcode, const size_t args_length, const size_t data_length);

	void write_string(const char* const value);
	void write_u8(const uint8_t value);
	void write_u16(const uint16_t value);
	void write_u32(const uint32_t value);
	void write(const void* const data, const size_t length);
	
	uint8_t read_u8();
	uint16_t read_u16();
	uint32_t read_u32();
	void read(void* const data, const size_t length);

	void end();
	
	void send_patch(
		const hci_patch_opcode_t opcode,
		const void* const data,
		const size_t length
	);
	
	hci_event_status_t wait_for(const hci_event_opcode_t opcode);
	void wait_for_data(hci_response_t& response);

	void handle_unsolicited_events();

private:
	bool read_header(const size_t payload_length, hci_response_t& header);
	bool read_payload(const hci_response_t& hci_header);
	bool read(hci_response_t& hci_header);
	
	bool get_next_event(hci_response_t& hci_header);
	void wait_for_next_event(hci_response_t& hci_header);

	bool handle_unsolicited_event(const hci_response_t& response);
	bool handle_patches_request(const hci_patch_opcode_t opcode);
};

extern CC3000HCI cc3000_hci;

#endif /* end of include guard: CC3000_HCI_H_XBGVM3NF */
