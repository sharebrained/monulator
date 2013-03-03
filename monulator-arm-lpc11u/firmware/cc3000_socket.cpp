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

#include "cc3000_socket.h"

#include "cc3000_hci.h"

#include <string.h>

static constexpr uint32_t htonl(const uint32_t host_value) {
	return ((host_value & 0xff000000) >> 24) |
	       ((host_value & 0x00ff0000) >>  8) |
	       ((host_value & 0x0000ff00) <<  8) |
	       ((host_value & 0x000000ff) << 24);
}

static constexpr uint16_t htons(const uint16_t host_value) {
	return ((host_value & 0xff00) >> 8) |
	       ((host_value & 0x00ff) << 8);
}

static socket_result_t make_socket_result(const hci_event_status_t status) {
	socket_result_t result = SOCKET_ERROR_HCI;
	if( status == HCI_EVENT_STATUS_OK ) {
		result = (socket_result_t)cc3000_hci.read_u32();
	}
	cc3000_hci.end();
	return result;
}
/*
#include <tuple>

template<size_t> struct int_{};

template<class Tuple, size_t Pos>
void write_helper(const Tuple& t, int_<Pos>) {
	cc3000_hci.write(std::get<std::tuple_size<Tuple>::value - Pos>(t));
	write_helper(t, int_<Pos - 1>());
}

template<class Tuple>
void write_helper(const Tuple& t, int_<1>) {
	cc3000_hci.write(std::get<std::tuple_size<Tuple>::value - 1>(t));
}

template<class... Args>
void write(const std::tuple<Args...>& t) {
	write_helper(t, int_<sizeof...(Args)>());
}

using args_t = std::tuple<
	uint32_t,
	uint32_t,
	uint32_t
>;

const args_t args {
	domain,
	type,
	protocol
};

write(args);
*/
socket_t CC3000Socket::socket(
	const address_family_t domain,
	const socket_type_t type,
	const ip_protocol_t protocol
) {
	cc3000_hci.start(HCI_COMMAND_OPCODE_SOCKET, 12);
	cc3000_hci.write_u32(domain);
	cc3000_hci.write_u32(type);
	cc3000_hci.write_u32(protocol);
	cc3000_hci.end();
	
	const auto status = cc3000_hci.wait_for(HCI_EVENT_OPCODE_SOCKET);
	socket_t result = SOCKET_ERROR_HCI;
	if( status == HCI_EVENT_STATUS_OK ) {
		result = cc3000_hci.read_u32();
	}
	cc3000_hci.end();
	return result;
}

socket_result_t CC3000Socket::close_socket(const socket_t socket) {
	cc3000_hci.start(HCI_COMMAND_OPCODE_CLOSE_SOCKET, 4);
	cc3000_hci.write_u32(socket);
	cc3000_hci.end();
	
	return make_socket_result(
		cc3000_hci.wait_for(HCI_EVENT_OPCODE_CLOSE_SOCKET)
	);
}

socket_result_t CC3000Socket::gethostbyname(
	const char* const name,
	in_addr_t& address
) {
	const size_t name_length = strlen(name);
	const size_t hci_payload_length = 8 + name_length;
	cc3000_hci.start(HCI_COMMAND_OPCODE_GETHOSTBYNAME, hci_payload_length);
	cc3000_hci.write_u32(8);
	cc3000_hci.write_u32(name_length);
	cc3000_hci.write(name, name_length);
	cc3000_hci.end();
	
	const auto status = cc3000_hci.wait_for(HCI_EVENT_OPCODE_GETHOSTBYNAME);
	socket_result_t result = SOCKET_ERROR_HCI;
	if( status == HCI_EVENT_STATUS_OK ) {
		result = (socket_result_t)cc3000_hci.read_u32();
		address.ipv4 = cc3000_hci.read_u32();
	}
	cc3000_hci.end();
	return result;
}

socket_result_t CC3000Socket::connect(
	const socket_t socket,
	const in_addr_t address,
	const in_port_t port
) {
	const size_t address_length = 8;
	
	cc3000_hci.start(HCI_COMMAND_OPCODE_CONNECT, 12 + address_length);
	cc3000_hci.write_u32(socket);
	cc3000_hci.write_u32(8);
	cc3000_hci.write_u32(address_length);
	cc3000_hci.write_u16(AF_INET);
	cc3000_hci.write_u16(htons(port));
	cc3000_hci.write_u32(htonl(address.ipv4));
	cc3000_hci.end();
	
	return make_socket_result(
		cc3000_hci.wait_for(HCI_EVENT_OPCODE_CONNECT)
	);
}

socket_result_t CC3000Socket::select(
	const size_t nfds,
	uint32_t& read_set,
	uint32_t& write_set,
	uint32_t& except_set,
	const uint32_t timeout_usec
) {
	const uint32_t block = (timeout_usec > 0) ? 1 : 0;
	
	cc3000_hci.start(HCI_COMMAND_OPCODE_SELECT, 44);
	cc3000_hci.write_u32(nfds);
	cc3000_hci.write_u32(0x14);
	cc3000_hci.write_u32(0x14);
	cc3000_hci.write_u32(0x14);
	cc3000_hci.write_u32(0x14);
	cc3000_hci.write_u32(block);
	cc3000_hci.write_u32(read_set);
	cc3000_hci.write_u32(write_set);
	cc3000_hci.write_u32(except_set);
	cc3000_hci.write_u32(timeout_usec / 1000000);
	cc3000_hci.write_u32(timeout_usec % 1000000);
	cc3000_hci.end();
	
	const auto status = cc3000_hci.wait_for(HCI_EVENT_OPCODE_SELECT);
	socket_result_t result = SOCKET_ERROR_HCI;
	if( status >= 0 ) {
		const uint32_t status = cc3000_hci.read_u32();
		read_set = cc3000_hci.read_u32();
		write_set = cc3000_hci.read_u32();
		except_set = cc3000_hci.read_u32();
		result = (socket_result_t)status;
	}
	cc3000_hci.end();
	return result;
}

socket_result_t CC3000Socket::send(
	const socket_t socket,
	const void* const data,
	const size_t data_length
) {
	const size_t socket_length = 4;
	const size_t args_length = 16;
	const uint32_t flags = 0;
	
	cc3000_hci.start(HCI_DATA_OPCODE_SEND, args_length, data_length);
	cc3000_hci.write_u32(socket);
	cc3000_hci.write_u32(args_length - socket_length);
	cc3000_hci.write_u32(data_length);
	cc3000_hci.write_u32(flags);
	cc3000_hci.write(data, data_length);
	cc3000_hci.end();
	
	return SOCKET_OK;
}

socket_result_t CC3000Socket::send(
	const socket_t socket,
	const char* const string
) {
	return send(socket, string, strlen(string));
}

socket_result_t CC3000Socket::recv(
	const socket_t socket,
	void* const data,
	const size_t length
) {
	const size_t flags = 0;
	const size_t args_length = 12;
	const size_t hci_payload_length = args_length;
	
	cc3000_hci.start(HCI_COMMAND_OPCODE_RECV, hci_payload_length);
	cc3000_hci.write_u32(socket);
	cc3000_hci.write_u32(length);
	cc3000_hci.write_u32(flags);
	cc3000_hci.end();
	
	const auto status = cc3000_hci.wait_for(HCI_EVENT_OPCODE_RECV);
	socket_result_t result = SOCKET_ERROR_HCI;
	if( status == HCI_EVENT_STATUS_OK ) {
		const uint32_t event_sd = cc3000_hci.read_u32();
		const uint32_t number_of_bytes = cc3000_hci.read_u32();
		const uint32_t flags = cc3000_hci.read_u32();
		cc3000_hci.end();

		if( number_of_bytes > 0 ) {
			hci_response_t response;
			cc3000_hci.wait_for_data(response);
			// TODO: Verify that args_length == 0x18.
			// TODO: Handle the args, once I can find a reasonable definition of what they are...
			cc3000_hci.read(nullptr, response.data.args_length);
			cc3000_hci.read(data, response.payload_length - response.data.args_length);
			cc3000_hci.end();
		}
		result = (socket_result_t)number_of_bytes;
	} else {
		cc3000_hci.end();
	}
	
	return result;
}

CC3000Socket cc3000_socket;
