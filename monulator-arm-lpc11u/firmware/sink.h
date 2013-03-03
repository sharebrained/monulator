/* 
 * Data sink interface for chaining processing stages.
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

#ifndef SINK_H_5BYF859T
#define SINK_H_5BYF859T

#include "noncopyable.h"

#include <cstddef>

struct sink_t : private noncopyable {
	int sink(const void* const data, const size_t data_length) {
		return do_sink(data, data_length);
	}
	
private:
	virtual int do_sink(const void* const data, const size_t data_length) = 0;
};

/*
struct http_response_sink_t {
	//virtual ~http_response_sink_t() { }
	virtual void http_response_sink(const uint8_t* const data, const size_t length) = 0;
};

template<typename Derived>
struct http_response_sink_base_t : public http_response_sink_t {
	virtual void http_response_sink(const uint8_t* const data, const size_t length) override {
		static_cast<Derived*>(this)->do_http_response_sink(data, length);
	}
};

struct http_response_sink_log_t : public http_response_sink_base_t<http_response_sink_log_t> {
	void do_http_response_sink(const uint8_t* const data, const size_t length) {
		serial_write_line("sink");
	}
};
*/

#endif /* end of include guard: SINK_H_5BYF859T */
