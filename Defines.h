/*
 *   Copyright (C) 2024 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#if !defined(Defines_H)
#define	Defines_H

#include <cstdint>

enum DATA_MODE {
	DATA_MODE_NONE,
	DATA_MODE_DSTAR,
	DATA_MODE_DMR,
	DATA_MODE_YSFDN,
	DATA_MODE_YSFVW,
	DATA_MODE_P25,
	DATA_MODE_NXDN,
	DATA_MODE_FM,
	DATA_MODE_M17
};

const uint8_t TAG_HEADER = 0U;
const uint8_t TAG_DATA   = 1U;
const uint8_t TAG_EOT    = 2U;

#endif
