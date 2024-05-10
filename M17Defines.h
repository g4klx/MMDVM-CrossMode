/*
 *   Copyright (C) 2020,2021,2024 by Jonathan Naylor G4KLX
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

#if !defined(M17DEFINES_H)
#define  M17DEFINES_H

#include <cstdint>

const unsigned int M17_LSF_LENGTH_BITS  = 240U;
const unsigned int M17_LSF_LENGTH_BYTES = M17_LSF_LENGTH_BITS / 8U;

const unsigned int M17_LSF_FRAGMENT_LENGTH_BITS  = M17_LSF_LENGTH_BITS / 6U;
const unsigned int M17_LSF_FRAGMENT_LENGTH_BYTES = M17_LSF_FRAGMENT_LENGTH_BITS / 8U;

const unsigned int M17_LICH_FRAGMENT_LENGTH_BITS  = M17_LSF_FRAGMENT_LENGTH_BITS + 8U;
const unsigned int M17_LICH_FRAGMENT_LENGTH_BYTES = M17_LICH_FRAGMENT_LENGTH_BITS / 8U;

const unsigned int M17_LSF_FRAGMENT_FEC_LENGTH_BITS  = M17_LSF_FRAGMENT_LENGTH_BITS * 2U;
const unsigned int M17_LSF_FRAGMENT_FEC_LENGTH_BYTES = M17_LSF_FRAGMENT_FEC_LENGTH_BITS / 8U;

const unsigned int M17_LICH_FRAGMENT_FEC_LENGTH_BITS  = M17_LICH_FRAGMENT_LENGTH_BITS * 2U;
const unsigned int M17_LICH_FRAGMENT_FEC_LENGTH_BYTES = M17_LICH_FRAGMENT_FEC_LENGTH_BITS / 8U;

const unsigned int M17_LICH_LENGTH_BITS  = 224U;
const unsigned int M17_LICH_LENGTH_BYTES = M17_LICH_LENGTH_BITS / 8U;

const unsigned int M17_PAYLOAD_LENGTH_BITS  = 128U;
const unsigned int M17_PAYLOAD_LENGTH_BYTES = M17_PAYLOAD_LENGTH_BITS / 8U;

const uint8_t       M17_NULL_NONCE[] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
const unsigned int  M17_META_LENGTH_BITS  = 112U;
const unsigned int  M17_META_LENGTH_BYTES = M17_META_LENGTH_BITS / 8U;

const unsigned int M17_FN_LENGTH_BITS  = 16U;
const unsigned int M17_FN_LENGTH_BYTES = M17_FN_LENGTH_BITS / 8U;

const unsigned int M17_CRC_LENGTH_BITS  = 16U;
const unsigned int M17_CRC_LENGTH_BYTES = M17_CRC_LENGTH_BITS / 8U;

const uint8_t      M17_3200_AUDIO_SILENCE[]    = {0x01U, 0x00U, 0x09U, 0x43U, 0x9CU, 0xE4U, 0x21U, 0x08U};
const unsigned int M17_3200_AUDIO_LENGTH_BYTES = M17_PAYLOAD_LENGTH_BYTES / 2U;

const uint8_t M17_PACKET_TYPE = 0U;
const uint8_t M17_STREAM_TYPE = 1U;

const uint8_t M17_DATA_TYPE_DATA       = 0x01U;
const uint8_t M17_DATA_TYPE_VOICE      = 0x02U;
const uint8_t M17_DATA_TYPE_VOICE_DATA = 0x03U;

const uint8_t M17_ENCRYPTION_TYPE_NONE     = 0x00U;
const uint8_t M17_ENCRYPTION_TYPE_AES      = 0x01U;
const uint8_t M17_ENCRYPTION_TYPE_SCRAMBLE = 0x02U;

const uint8_t M17_ENCRYPTION_SUB_TYPE_TEXT       = 0x00U;
const uint8_t M17_ENCRYPTION_SUB_TYPE_GPS        = 0x01U;
const uint8_t M17_ENCRYPTION_SUB_TYPE_CALLSIGNS  = 0x02U;

const unsigned int M17_CALLSIGN_LENGTH = 9U;

#endif
