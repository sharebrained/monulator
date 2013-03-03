/* 
 * Parser for SWPC aurora data.
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

#ifndef AURORA_SWPC_H_LFIAXICA
#define AURORA_SWPC_H_LFIAXICA

#include "csv_parser.h"

struct aurora_swpc_t : public csv_sink_t {

	uint32_t level() const {
		return aurora_level;
	}
	
private:
	float aurora_n = 0.0f;
	uint32_t aurora_level = 0;
	
	virtual int do_csv_cell(const size_t column, const size_t row, const char* const s);
};

#endif /* end of include guard: AURORA_SWPC_H_LFIAXICA */
