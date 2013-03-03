/* 
 * TI CC3000 Wi-Fi module WLAN API.
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

#include "cc3000_wlan.h"

#include "cc3000_hci.h"

#include <string.h>

static wlan_result_t make_wlan_result(const hci_event_status_t status) {
	wlan_result_t result = WLAN_ERROR_HCI;
	if( status == HCI_EVENT_STATUS_OK ) {
		result = (wlan_result_t)cc3000_hci.read_u32();
	}
	cc3000_hci.end();
	return result;
}

bool CC3000WLAN::handle_unsolicited_event(const hci_event_opcode_t opcode) {
	switch( opcode ) {
	case HCI_EVENT_OPCODE_UNSOLICITED_WLAN_CONNECT:
		initialized = true;
		connected = true;
		dhcp = false;
		return true;

	case HCI_EVENT_OPCODE_UNSOLICITED_WLAN_DHCP:
		initialized = true;
		connected = true;
		dhcp = true;
		return true;

	case HCI_EVENT_OPCODE_UNSOLICITED_WLAN_DISCONNECT:
		initialized = true;
		connected = false;
		dhcp = false;
		return true;
	
	case HCI_EVENT_OPCODE_UNSOLICITED_WLAN_INIT:
		initialized = true;
		connected = false;
		dhcp = false;
		return true;

	default:
		return false;
	}
}

wlan_result_t CC3000WLAN::set_event_mask(
	const hci_event_mask_t event_mask
) {
	cc3000_hci.start(HCI_COMMAND_OPCODE_SET_EVENT_MASK, 4);
	cc3000_hci.write_u32(event_mask);
	cc3000_hci.end();
	
	return make_wlan_result(
		cc3000_hci.wait_for(HCI_EVENT_OPCODE_SET_EVENT_MASK)
	);
}

/*
void cc3000_wlan_ioctl_set_scanparam() {
	const wlan_ioctl_set_scan_params_t data = {
		36,
		1,
		100,
		100,
		5,
		0x7ff,
		-80,
		0,
		205,
		{
			2000, 2000, 2000, 2000,
			2000, 2000, 2000, 2000,
			2000, 2000, 2000, 2000,
			2000, 2000, 2000, 2000,
		},
	};
	
	cc3000_hci.send_command(0x0003, { {
		{ &data, sizeof(data) },
	} });

	CC3000HCIResponse response;
	response.read_until_event(0x0003);
}

void cc3000_wlan_ioctl_get_scan_results(const uint32_t scan_timeout) {
	const wlan_ioctl_get_scan_results_t data = {
		scan_timeout,
	};
	static_assert(sizeof(data) == 4, "wlan_ioctl_get_scan_results_t size incorrect");

	cc3000_hci.send_command(0x0007, { {
		{ &data, sizeof(data) },
	} });
}
*/
wlan_result_t CC3000WLAN::ioctl_set_connection_policy(const bool connect_to_open_ap, const bool use_fast_connect, const bool use_profiles) {
	cc3000_hci.start(HCI_COMMAND_OPCODE_WLAN_IOCTL_SET_CONNECTION_POLICY, 12);
	cc3000_hci.write_u32(connect_to_open_ap ? 1 : 0);
	cc3000_hci.write_u32(use_fast_connect ? 1 : 0);
	cc3000_hci.write_u32(use_profiles ? 1 : 0);
	cc3000_hci.end();
	
	return make_wlan_result(
		cc3000_hci.wait_for(HCI_EVENT_OPCODE_WLAN_IOCTL_SET_CONNECTION_POLICY)
	);
}

wlan_result_t CC3000WLAN::ioctl_del_profile(const uint32_t index) {
	cc3000_hci.start(HCI_COMMAND_OPCODE_WLAN_IOCTL_DEL_PROFILE, 4);
	cc3000_hci.write_u32(index);
	cc3000_hci.end();
	
	return make_wlan_result(
		cc3000_hci.wait_for(HCI_EVENT_OPCODE_WLAN_IOCTL_DEL_PROFILE)
	);
}

wlan_result_t CC3000WLAN::connect(const char* const ssid, const bssid_t& bssid, const char* const key) {
	const size_t ssid_length = strlen(ssid);
	const size_t key_length = strlen(key);
	
	cc3000_hci.start(HCI_COMMAND_OPCODE_WLAN_CONNECT, 28 + ssid_length + key_length);
	cc3000_hci.write_u32(0x1c);
	cc3000_hci.write_u32(ssid_length);
	cc3000_hci.write_u32(3);
	cc3000_hci.write_u32(0x10 + ssid_length);
	cc3000_hci.write_u32(key_length);
	cc3000_hci.write_u16(0);
	cc3000_hci.write(&bssid, sizeof(bssid));
	cc3000_hci.write(ssid, ssid_length);
	cc3000_hci.write(key, key_length);
	cc3000_hci.end();
	
	return make_wlan_result(
		cc3000_hci.wait_for(HCI_EVENT_OPCODE_WLAN_CONNECT)
	);
}

wlan_result_t CC3000WLAN::smartconfig_set_prefix() {
	cc3000_hci.start(HCI_COMMAND_OPCODE_WLAN_SMARTCONFIG_SET_PREFIX, 3);
	cc3000_hci.write_string("TTT");
	cc3000_hci.end();
	
	return make_wlan_result(
		cc3000_hci.wait_for(HCI_EVENT_OPCODE_WLAN_SMARTCONFIG_SET_PREFIX)
	);
}
/*
struct wlan_smart_config_start_t {
	uint32_le_t algo_encrypted_flag;
	
private:
	static void _assert_struct() {
		static_assert(sizeof(wlan_smart_config_start_t) == 4, "wlan_smart_config_start_t size incorrect");
	}
};

void cc3000_wlan_smart_config_start(const uint32_t encrypted) {
	const wlan_smart_config_start_t args {
		encrypted,
	};
	
	cc3000_hci.send_command(0x000a, { {
		{ &args, sizeof(args) },
	} });
}
*/

CC3000WLAN cc3000_wlan;
