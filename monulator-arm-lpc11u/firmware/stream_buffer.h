/* 
 * Simple buffer that tracks current position of writes.
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

#pragma once
#ifndef __STREAM_BUFFER_H__
#define __STREAM_BUFFER_H__ 1

#include <cstdint>
#include <cstddef>
#include <array>

#include <string.h>

template<size_t N>
struct stream_buffer_t {
	constexpr stream_buffer_t() :
		buffer(),
		position(0) {
	}
	
	void reset() {
		position = 0;
	}

	size_t available() const {
		// NOTE: Reserve one item for the trailing zero for string() access.
		return (buffer.size() - 1) - position;
	}
	
	size_t length() const {
		return position;
	}
	
	bool append(const uint8_t* const data, const size_t data_length) {
		if( available() >= data_length ) {
			memcpy(&buffer[position], data, data_length);
			position += data_length;
			return true;
		} else {
			return false;
		}
	}
	
	bool append(const uint8_t value) {
		return append(&value, sizeof(value));
	}
	
	const uint8_t* data() const {
		return buffer.data();
	}
	
	const char* string() {
		buffer[position] = 0;
		return (const char*)buffer.data();
	}
	
private:
	std::array<uint8_t, N> buffer;
	size_t position;
};

#endif/*__STREAM_BUFFER_H__*/
