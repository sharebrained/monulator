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

#ifndef HTTP_H_XEKHJCG4
#define HTTP_H_XEKHJCG4

#include "cc3000_socket.h"
#include "stream_buffer.h"
#include "http-parser/http_parser.h"
#include "sink.h"

#include <cstdint>
#include <cstddef>

struct http_connection_t {
	enum state_t {
		STATE_NONE,
		STATE_MESSAGE_BEGIN,
		STATE_STATUS_COMPLETE,
		STATE_HEADER_FIELD,
		STATE_HEADER_VALUE,
		STATE_HEADERS_COMPLETE,
		STATE_MESSAGE_COMPLETE
	};
	
	constexpr http_connection_t(sink_t& sink) :
		buffer(),
		socket(INVALID_SOCKET),
		parser(),
		state(STATE_NONE),
		sink(sink)
	{
	}
	
	bool open(const char* const host, const uint16_t port, const char* const path);
	void close();

private:
	stream_buffer_t<256> buffer;
	socket_t socket;
	http_parser parser;
	state_t state;
	sink_t& sink;

	bool write_request_header(const char* const host, const char* const path);

	int on_message_begin();
	int on_url(const char* const at, const size_t length);
	int on_status_complete();
	int on_header_field(const char* const at, const size_t length);
	int on_header_value(const char* const at, const size_t length);
	int on_headers_complete();
	int on_body(const char* const at, const size_t length);
	int on_message_complete();
	
	static http_connection_t& ref_(http_parser* parser);
	static int on_message_begin_(http_parser* parser);
	static int on_url_(http_parser* parser, const char* at, size_t length);
	static int on_status_complete_(http_parser* parser);
	static int on_header_field_(http_parser* parser, const char* at, size_t length);
	static int on_header_value_(http_parser* parser, const char* at, size_t length);
	static int on_headers_complete_(http_parser* parser);
	static int on_body_(http_parser*, const char* at, size_t length);
	static int on_message_complete_(http_parser* parser);

	static const http_parser_settings parser_settings;
};

#endif /* end of include guard: HTTP_H_XEKHJCG4 */
