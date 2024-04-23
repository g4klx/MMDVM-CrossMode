/*
 *   Copyright (C) 2015,2016,2018,2019,2021,024 by Jonathan Naylor G4KLX
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

#if !defined(DStarDefines_H)
#define	DStarDefines_H

#include <cstdint>

const unsigned int DSTAR_HEADER_LENGTH_BYTES = 41U;
const unsigned int DSTAR_FRAME_LENGTH_BYTES  = 12U;

const uint8_t DSTAR_END_PATTERN_BYTES[] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xC8, 0x7A };
const unsigned int  DSTAR_END_PATTERN_LENGTH_BYTES = 12U;

const uint8_t DSTAR_NULL_SLOW_SYNC_BYTES[] = { 0x55, 0x2D, 0x16 };
// Note that these are already scrambled, 0x66 0x66 0x66 otherwise
const uint8_t DSTAR_NULL_SLOW_DATA_BYTES[] = { 0x16, 0x29, 0xF5 };

const unsigned int  DSTAR_VOICE_FRAME_LENGTH_BYTES = 9U;
const unsigned int  DSTAR_DATA_FRAME_LENGTH_BYTES  = 3U;

const unsigned int  DSTAR_LONG_CALLSIGN_LENGTH  = 8U;
const unsigned int  DSTAR_SHORT_CALLSIGN_LENGTH = 4U;

const uint8_t DSTAR_SLOW_DATA_TYPE_MASK       = 0xF0U;
const uint8_t DSTAR_SLOW_DATA_TYPE_GPSDATA    = 0x30U;
const uint8_t DSTAR_SLOW_DATA_TYPE_TEXT       = 0x40U;
const uint8_t DSTAR_SLOW_DATA_TYPE_HEADER     = 0x50U;
const uint8_t DSTAR_SLOW_DATA_TYPE_FASTDATA01 = 0x80U;
const uint8_t DSTAR_SLOW_DATA_TYPE_FASTDATA16 = 0x90U;
const uint8_t DSTAR_SLOW_DATA_TYPE_SQUELCH    = 0xC0U;
const uint8_t DSTAR_SLOW_DATA_LENGTH_MASK     = 0x0FU;

const uint8_t DSTAR_SCRAMBLER_BYTES[] = { 0x70U, 0x4FU, 0x93U, 0x40U, 0x64U, 0x74U, 0x6DU, 0x30U, 0x2BU };

const uint8_t DSTAR_DATA_MASK           = 0x80U;
const uint8_t DSTAR_REPEATER_MASK       = 0x40U;
const uint8_t DSTAR_INTERRUPTED_MASK    = 0x20U;
const uint8_t DSTAR_CONTROL_SIGNAL_MASK = 0x10U;
const uint8_t DSTAR_URGENT_MASK         = 0x08U;
const uint8_t DSTAR_REPEATER_CONTROL    = 0x07U;
const uint8_t DSTAR_AUTO_REPLY          = 0x06U;
const uint8_t DSTAR_RESEND_REQUESTED    = 0x04U;
const uint8_t DSTAR_ACK_FLAG            = 0x03U;
const uint8_t DSTAR_NO_RESPONSE         = 0x02U;
const uint8_t DSTAR_RELAY_UNAVAILABLE   = 0x01U;

const uint8_t DSTAR_SYNC_BYTES[] = {0x55U, 0x2DU, 0x16U};

#endif
