/*
 *   Copyright (C) 2015,2016,2017,2024 by Jonathan Naylor G4KLX
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

#if !defined(YSFDefines_H)
#define	YSFDefines_H

#include <cstdint>

const unsigned int YSF_FRAME_LENGTH_BYTES = 120U;

const uint8_t YSF_SYNC_BYTES[] = {0xD4U, 0x71U, 0xC9U, 0x63U, 0x4DU};
const unsigned int YSF_SYNC_LENGTH_BYTES  = 5U;

const unsigned int YSF_FICH_LENGTH_BYTES  = 25U;

const uint8_t YSF_SYNC_OK           = 0x01U;

const unsigned int  YSF_CALLSIGN_LENGTH   = 10U;

const unsigned int  YSF_FRAME_TIME        = 100U;

const uint8_t YSF_FI_HEADER         = 0x00U;
const uint8_t YSF_FI_COMMUNICATIONS = 0x01U;
const uint8_t YSF_FI_TERMINATOR     = 0x02U;
const uint8_t YSF_FI_TEST           = 0x03U;

const uint8_t YSF_CS_RESERVE1 = 0x00U;
const uint8_t YSF_CS_RESERVE2 = 0x01U;
const uint8_t YSF_CS_ASSIGN   = 0x02U;
const uint8_t YSF_CS_RESERVE3 = 0x03U;

const uint8_t YSF_CM_GROUP_CQ   = 0x00U;
const uint8_t YSF_CM_RADIO_ID   = 0x01U;
const uint8_t YSF_CM_RESERVE    = 0x02U;
const uint8_t YSF_CM_INDIVIDUAL = 0x03U;

const uint8_t YSF_DT_VD_MODE1      = 0x00U;
const uint8_t YSF_DT_DATA_FR_MODE  = 0x01U;
const uint8_t YSF_DT_VD_MODE2      = 0x02U;
const uint8_t YSF_DT_VOICE_FR_MODE = 0x03U;

const uint8_t YSF_MR_DIRECT   = 0x00U;
const uint8_t YSF_MR_NOT_BUSY = 0x01U;
const uint8_t YSF_MR_BUSY     = 0x02U;

const uint8_t YSFDN_SILENCE[] = {0xFFU, 0xF0U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const uint8_t YSF_NULL_CALLSIGN1[] = {0x20U, 0x20U, 0x20U, 0x20U, 0x20U, 0x20U, 0x20U, 0x20U, 0x20U, 0x20U};
const uint8_t YSF_NULL_CALLSIGN2[] = {0x2AU, 0x2AU, 0x2AU, 0x2AU, 0x2AU, 0x2AU, 0x2AU, 0x2AU, 0x2AU, 0x2AU};
const uint8_t YSF_NULL_DT[]        = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

#endif
