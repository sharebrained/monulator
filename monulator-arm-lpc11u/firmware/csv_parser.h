/* 
 * CSV stream parser.
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

#ifndef CSV_PARSER_H_9I2AIMAG
#define CSV_PARSER_H_9I2AIMAG

#include "noncopyable.h"
#include "sink.h"
#include "stream_buffer.h"

#include <cstddef>

struct csv_sink_t : private noncopyable {
	int csv_cell(const size_t column, const size_t row, const char* const s) {
		return do_csv_cell(column, row, s);
	}
	
private:
	virtual int do_csv_cell(const size_t column, const size_t row, const char* const s) = 0;
};

struct csv_parser_t : public sink_t {
	constexpr csv_parser_t(const char delimiter, csv_sink_t& sink) :
		sink(sink),
		value(),
		delimiter(delimiter)
	{
	}
	
private:
	csv_sink_t& sink;
	stream_buffer_t<80> value;
	size_t row = 0;
	size_t column = 0;
	const char delimiter = ',';
	
	enum state_t {
		csv_delimiter,
		csv_value,
		csv_quoted_value,
		csv_newline,
		csv_error,
	};
	state_t expect = csv_value;

	virtual int do_sink(const void* const data, const size_t data_length);
	
	void feed(const char c);
};

#endif /* end of include guard: CSV_PARSER_H_9I2AIMAG */
