/* 
 * TI CC3000 Wi-Fi module Socket API.
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

#ifndef CC3000_SOCKET_H_XOYUD586
#define CC3000_SOCKET_H_XOYUD586

#include "cc3000_hci.h"

#include <cstdint>
#include <cstddef>

using socket_t = int32_t;

enum address_family_t : uint16_t {
	AF_INET = 2,
};

enum socket_type_t {
	SOCK_STREAM = 1,
	SOCK_DGRAM = 2,
};

enum ip_protocol_t {
	IPPROTO_DEFAULT = 0,
	IPPROTO_TCP = 6,
	IPPROTO_UDP = 17,
};

using in_port_t = uint16_t;

struct in_addr_t {
 	uint32_t ipv4;
};

const socket_t INVALID_SOCKET = -1;
enum socket_result_t : int32_t {
	SOCKET_OK = 0,
	SOCKET_ERROR_HCI = (int32_t)0xDEADBEEF,
};

struct CC3000Socket {
	socket_result_t gethostbyname(
		const char* const name,
		in_addr_t& address
	);

	socket_t socket(
		const address_family_t domain,
		const socket_type_t type,
		const ip_protocol_t protocol
	);
	/*
	socket_result_t bind(
		const socket_t socket
	);
	*/
	socket_result_t connect(
		const socket_t socket,
		const in_addr_t address,
		const in_port_t port
	);

	socket_result_t select(
		const size_t nfds,
		uint32_t& read_set,
		uint32_t& write_set,
		uint32_t& except_set,
		const uint32_t timeout_usec
	);
	
	socket_result_t send(
		const socket_t socket,
		const void* const data,
		const size_t data_length
	);
	
	socket_result_t send(
		const socket_t socket,
		const char* const string
	);
	
	socket_result_t recv(
		const socket_t socket,
		void* const data,
		const size_t data_length
	);
	
	socket_result_t close_socket(
		const socket_t socket
	);
};

extern CC3000Socket cc3000_socket;

#endif /* end of include guard: CC3000_SOCKET_H_XOYUD586 */
