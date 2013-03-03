/* 
 * C++ transliteration of Linux kfifo FIFO.
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

#ifndef KFIFO_H_VI8SRWOK
#define KFIFO_H_VI8SRWOK

#include <cstddef>
#include <algorithm>

#include <string.h>

/*
 * Implementation derived HEAVILY from concepts in the Linux kernel's
 * kfifo implementation. Very clever stuff, and so elegant...
 */

template<typename T, size_t K>
struct kfifo_t {
	constexpr kfifo_t() :
		in_(0), out_(0)
	{
	}
	
	void reset() {
		in_ = out_ = 0;
	}
	
	void reset_out() {
		out_ = in_;
	}
	
	size_t len() const {
		return in_ - out_;
	}
	
	size_t unused() const {
		return size() - (in_ - out_);
	}
	
	bool is_empty() const {
		return in_ == out_;
	}
	
	bool is_full() const {
		return len() > mask();
	}
	
	bool put(const T& val) {
		const auto is_not_full = !is_full();
		if( is_not_full ) {
			data[in & mask()] = val;
			in_++;
		}
		return is_not_full;
	}
	
	bool get(T& val) {
		const auto is_not_empty = !is_empty();
		if( is_not_empty ) {
			val = data[out & mask()];
			out_++;
		}
		return is_not_empty;
	}
	
	bool peek(T& val) const {
		const auto is_not_empty = !is_empty();
		if( is_not_empty ) {
			val = data[out & mask()];
		}
		return is_not_empty;
	}
	
	size_t in(const T* const buf, size_t len) {
		const auto l = unused();
		if( len > l ) {
			len = l;
		}
		
		copy_in(buf, len, in_);
		in_ += len;
		return len;
	}
	
	size_t out_peek(T* const buf, size_t len) {
		const auto l = in_ - out_;
		if( len > l ) {
			len = l;
		}
		
		copy_out(buf, len, out_);
		return len;
	}
	
	size_t out(T* const buf, size_t len) {
		len = out_peek(buf, len);
		out_ += len;
		return len;
	}
	
private:
	T data[1 << K];
	size_t in_;
	size_t out_;
	
	constexpr size_t size() {
		return (1 << K);
	}
	
	constexpr size_t esize() {
		return sizeof(T);
	}
	
	constexpr size_t mask() {
		return size() - 1;
	}
	
	void copy_in(const T* const src, const size_t len, size_t off) {
		off &= mask();
		const auto l = std::min(len, size() - off);
		
		memcpy(&data[off], &src[0], l * esize());
		memcpy(&data[0], &src[l], (len - l) * esize());
	}
	
	void copy_out(T* const dst, const size_t len, size_t off) {
		off &= mask();
		const auto l = std::min(len, size() - off);
		
		memcpy(&dst[0], &data[off], l * esize());
		memcpy(&dst[l], &data[0], (len - l) * esize());
	}
};

#endif /* end of include guard: KFIFO_H_VI8SRWOK */
