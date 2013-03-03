/* 
 * HTTP request/response layer.
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

#include "http.h"

#include <cstdint>
#include <array>

#include "serial.h"

http_connection_t& http_connection_t::ref_(http_parser* parser) {
	return *((http_connection_t*)parser->data);
}

int http_connection_t::on_message_begin_(http_parser* parser) {
	return ref_(parser).on_message_begin();
}

int http_connection_t::on_url_(http_parser* parser, const char* at, size_t length) {
	return ref_(parser).on_url(at, length);
}

int http_connection_t::on_status_complete_(http_parser* parser) {
	return ref_(parser).on_status_complete();
}

int http_connection_t::on_header_field_(http_parser* parser, const char* at, size_t length) {
	return ref_(parser).on_header_field(at, length);
}

int http_connection_t::on_header_value_(http_parser* parser, const char* at, size_t length) {
	return ref_(parser).on_header_value(at, length);
}

int http_connection_t::on_headers_complete_(http_parser* parser) {
	return ref_(parser).on_headers_complete();
}

int http_connection_t::on_body_(http_parser* parser, const char* at, size_t length) {
	return ref_(parser).on_body(at, length);
}

int http_connection_t::on_message_complete_(http_parser* parser) {
	return ref_(parser).on_message_complete();
}

const http_parser_settings http_connection_t::parser_settings {
	http_connection_t::on_message_begin_,
	http_connection_t::on_url_,
	http_connection_t::on_status_complete_,
	http_connection_t::on_header_field_,
	http_connection_t::on_header_value_,
	http_connection_t::on_headers_complete_,
	http_connection_t::on_body_,
	http_connection_t::on_message_complete_,
};

int http_connection_t::on_message_begin() {
	state = STATE_MESSAGE_BEGIN;
	serial_write_line("<message>");
	return 0;
}

int http_connection_t::on_url(const char* const at, const size_t length) {
	serial_write_line("<url/>");
	return 0;
}

int http_connection_t::on_status_complete() {
	state = STATE_STATUS_COMPLETE;
	serial_write_line("<status/>");
	return 0;
}

int http_connection_t::on_header_field(const char* const at, const size_t length) {
	if( state != STATE_HEADER_FIELD ) {
		/*
		if( state == STATE_HEADER_VALUE ) {
			serial_write_buffer(buffer.data(), buffer.length());
			serial_write_line();
		}
		buffer.reset();
		*/
		state = STATE_HEADER_FIELD;
	}
	/*
	return buffer.append((const uint8_t*)at, length) ? 0 : -1;
	*/
	return 0;
}

int http_connection_t::on_header_value(const char* const at, const size_t length) {
	// TODO: check header field value if this is the first call after on_header_field.
	if( state != STATE_HEADER_VALUE ) {
		// TODO:
		// header_field_id = identify_header_field();
		/*
		serial_write_buffer(buffer.data(), buffer.length());
		serial_write_string(": ");
		buffer.reset();
		*/
		state = STATE_HEADER_VALUE;
	}
	/*
	// TODO:
	// if( capture_header_value ) {
		return buffer.append((const uint8_t*)at, length) ? 0 : -1;
	//} else {
	//	return 0;
	//}
	*/
	return 0;
}

int http_connection_t::on_headers_complete() {
	/*
	if( state == STATE_HEADER_VALUE ) {
		serial_write_buffer(buffer.data(), buffer.length());
		serial_write_line();
	}
	*/
	state = STATE_HEADERS_COMPLETE;
	serial_write_line("<headers/>");
	return 0;
}

int http_connection_t::on_body(const char* const at, const size_t length) {
	return sink.sink(at, length);
}

int http_connection_t::on_message_complete() {
	state = STATE_MESSAGE_COMPLETE;
	serial_write_line("</message>");
	return 0;
}

bool http_connection_t::open(const char* const host, const uint16_t port, const char* const path) {
	
	socket = cc3000_socket.socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( socket < SOCKET_OK ) {
		return false;
	}
	
	in_addr_t host_address;
	const auto result_gethostbyname = cc3000_socket.gethostbyname(host, host_address);
	if( result_gethostbyname < SOCKET_OK ) {
		close();
		return false;
	}

	const auto result_connect = cc3000_socket.connect(socket, host_address, port);
	if( result_connect < SOCKET_OK ) {
		close();
		return false;
	}
	
	write_request_header(host, path);
	
	state = STATE_NONE;
	
	http_parser_init(&parser, HTTP_RESPONSE);
	parser.data = (void*)this;

	bool result = false;
	while(true) {
		std::array<uint8_t, 256> recv_data;
		const auto bytes_read_from_cc3000 = cc3000_socket.recv(socket, recv_data.data(), recv_data.size());
		if( bytes_read_from_cc3000 <= 0 ) {
			break;
		}
		http_parser_execute(&parser, &parser_settings, (char*)recv_data.data(), bytes_read_from_cc3000);
		if( state == STATE_MESSAGE_COMPLETE ) {
			result = true;
			break;
		}
	}

	close();
	return result;
}

bool http_connection_t::write_request_header(const char* const host, const char* const path) {
	cc3000_socket.send(socket, "GET ");
	cc3000_socket.send(socket, path);
	cc3000_socket.send(socket, " HTTP/1.0\r\nHost: ");
	cc3000_socket.send(socket, host);
	cc3000_socket.send(socket, "\r\nUser-Agent: Monulator/0.6\r\nAccept: text/html\r\n\r\n");
	return true;
}
/*
bool available = false;
while(!available) {
	uint32_t read_set = 1 << socket_1, write_set = 0, except_set = 1 << socket_1;
	const auto result_select = cc3000_socket.select(socket_1 + 1, read_set, write_set, except_set, 1000000);
	if( result_select < SOCKET_OK ) {
		serial_write_string(" fail: ");
		serial_write_hex(result_select, 8);
		serial_write_line();
		more = false;
		break;
	} else {
		serial_write_string("read: ");
		serial_write_hex(read_set, 8);
		serial_write_string(", write: ");
		serial_write_hex(write_set, 8);
		serial_write_string(", except: ");
		serial_write_hex(except_set, 8);
		serial_write_line();
		if( read_set & (1 << socket_1) ) {
			break;
		}
	}
}
*/	
/*
size_t http_connection_t::read(void* const buffer, const size_t buffer_length) {
	uint8_t* p = (uint8_t*)buffer;
	size_t bytes_remaining = buffer_length;
	while( bytes_remaining > 0 ) {
		const auto bytes_read_from_fifo = rx_fifo.out(p, bytes_remaining);
		p += bytes_read_from_fifo;
		bytes_remaining -= bytes_read_from_fifo;
		
		if( rx_fifo.unused() >= CC3000_READ_SIZE ) {
			std::array<uint8_t, CC3000_READ_SIZE> recv_data;
			const auto bytes_read_from_cc3000 = cc3000_socket.recv(socket, recv_data.data(), recv_data.size());
			if( bytes_read_from_cc3000 < SOCKET_OK ) {
				return buffer_length - bytes_remaining;
			}
			const size_t bytes_written_to_fifo = rx_fifo.in(recv_data.data(), bytes_read_from_cc3000);
			if( bytes_written_to_fifo != bytes_read_from_cc3000 ) {
				// TODO: This is should not be possible. Make sure.
			}
		}
	}
	return buffer_length;
}
*/
void http_connection_t::close() {
	const auto result = cc3000_socket.close_socket(socket);
	socket = INVALID_SOCKET;
}
