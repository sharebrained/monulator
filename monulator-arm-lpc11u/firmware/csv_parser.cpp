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

#include "csv_parser.h"

int csv_parser_t::do_sink(const void* const data, const size_t data_length) {
	for(size_t i=0; i<data_length; i++) {
		feed(((const char*)data)[i]);
	}
	return 0;
}

void csv_parser_t::feed(const char c) {
	if( c == '"' ) {
		if( expect == csv_value ) {
			expect = csv_quoted_value;
		} else if( expect == csv_quoted_value ) {
			sink.csv_cell(column, row, value.string());
			value.reset();
			column += 1;
			expect = csv_delimiter;
		} else {
			expect = csv_error;
			return;
		}
		
	} else if( c == delimiter ) {
		if( expect == csv_delimiter ) {
			expect = csv_value;
		} else if( expect == csv_quoted_value ) {
			if( value.append(c) ) {
				expect = expect;
			} else {
				expect = csv_error;
				return;
			}
		} else if( expect == csv_value ) {
			sink.csv_cell(column, row, value.string());
			value.reset();
			column += 1;
			expect = csv_value;
		} else {
			expect = csv_error;
			return;
		}
		
	} else if( (c == '\x0A') || (c == '\x0D') ) {
		if( expect == csv_value ) {
			sink.csv_cell(column, row, value.string());
			value.reset();
			column = 0;
			row += 1;
			expect = csv_newline;
		} else if( expect == csv_newline ) {
			expect = csv_newline;
		} else {
			expect = csv_error;
			return;
		}
		
	} else {
		if( (expect == csv_value) || (expect == csv_quoted_value) ) {
			if( value.append(c) ) {
				expect = expect;
			} else {
				expect = csv_error;
				return;
			}
		} else {
			expect = csv_error;
			return;
		}
	}
}