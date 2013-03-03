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

#ifndef CC3000_WLAN_H_IHRHVIEA
#define CC3000_WLAN_H_IHRHVIEA

#include "cc3000_hci.h"

#include <cstdint>
#include <array>

using octet_t = uint8_t;
using mac_address_t = std::array<octet_t, 6>;
using bssid_t = std::array<octet_t, 6>;

enum wlan_result_t {
	WLAN_ERROR_HCI = -1,
};

struct CC3000WLAN {
	wlan_result_t set_event_mask(
		const hci_event_mask_t event_mask
	);
	
	wlan_result_t smartconfig_set_prefix();

	void smart_config_start(
		const uint32_t encrypted
	);

	wlan_result_t ioctl_set_connection_policy(
		const bool connect_to_open_ap,
		const bool use_fast_connect,
		const bool use_profiles
	);

	wlan_result_t ioctl_del_profile(
		const uint32_t index
	);

	wlan_result_t connect(
		const char* const ssid,
		const bssid_t& bssid,
		const char* const key
	);

	bool handle_unsolicited_event(const hci_event_opcode_t opcode);
	
	bool is_initialized() const {
		return initialized;
	}
	
	bool is_connected() const {
		return connected;
	}
	
	bool is_dhcp() const {
		return dhcp;
	}

	void reset_state() {
		initialized = false;
		connected = false;
		dhcp = false;
	}

private:
	bool initialized = false;
	bool connected = false;
	bool dhcp = false;
};

extern CC3000WLAN cc3000_wlan;

#endif /* end of include guard: CC3000_WLAN_H_IHRHVIEA */
