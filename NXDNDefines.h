/*
 *   Copyright (C) 2016,2017,2018,2024 by Jonathan Naylor G4KLX
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

#if !defined(NXDNDEFINES_H)
#define  NXDNDEFINES_H

#include <cstdint>

const unsigned int NXDN_RADIO_SYMBOL_LENGTH = 5U;      // At 24 kHz sample rate

const unsigned int NXDN_FRAME_LENGTH_BITS    = 384U;
const unsigned int NXDN_FRAME_LENGTH_BYTES   = NXDN_FRAME_LENGTH_BITS / 8U;
const unsigned int NXDN_FRAME_LENGTH_SYMBOLS = NXDN_FRAME_LENGTH_BITS / 2U;

const unsigned int NXDN_FSW_LENGTH_BITS    = 20U;
const unsigned int NXDN_FSW_LENGTH_SYMBOLS = NXDN_FSW_LENGTH_BITS / 2U;
const unsigned int NXDN_FSW_LENGTH_SAMPLES = NXDN_FSW_LENGTH_SYMBOLS * NXDN_RADIO_SYMBOL_LENGTH;

const uint8_t NXDN_FSW_BYTES[]      = {0xCDU, 0xF5U, 0x90U};
const uint8_t NXDN_FSW_BYTES_MASK[] = {0xFFU, 0xFFU, 0xF0U};
const unsigned int  NXDN_FSW_BYTES_LENGTH = 3U;

const uint8_t HEADER_BYTES[]  = { 0x83U, 0x01U, 0x10U, 0x00U, 0x0FU, 0x01U, 0x00U, 0x20U };
const uint8_t TRAILER_BYTES[] = { 0x83U, 0x01U, 0x10U, 0x00U, 0x0FU, 0x08U, 0x00U, 0x20U };

const unsigned int NXDN_LICH_LENGTH_BITS = 16U;

const unsigned int NXDN_SACCH_LENGTH_BITS  = 60U;
const unsigned int NXDN_FACCH1_LENGTH_BITS = 144U;
const unsigned int NXDN_FACCH2_LENGTH_BITS = 348U;

const unsigned int NXDN_FSW_LICH_SACCH_LENGTH_BITS  = NXDN_FSW_LENGTH_BITS + NXDN_LICH_LENGTH_BITS + NXDN_SACCH_LENGTH_BITS;
const unsigned int NXDN_FSW_LICH_SACCH_LENGTH_BYTES = NXDN_FSW_LICH_SACCH_LENGTH_BITS / 8U;

const uint8_t NXDN_LICH_RFCT_RCCH   = 0U;
const uint8_t NXDN_LICH_RFCT_RTCH   = 1U;
const uint8_t NXDN_LICH_RFCT_RDCH   = 2U;
const uint8_t NXDN_LICH_RFCT_RTCH_C = 3U;

const uint8_t NXDN_LICH_USC_SACCH_NS      = 0U;
const uint8_t NXDN_LICH_USC_UDCH          = 1U;
const uint8_t NXDN_LICH_USC_SACCH_SS      = 2U;
const uint8_t NXDN_LICH_USC_SACCH_SS_IDLE = 3U;

const uint8_t NXDN_LICH_STEAL_NONE     = 3U;
const uint8_t NXDN_LICH_STEAL_FACCH1_2 = 2U;
const uint8_t NXDN_LICH_STEAL_FACCH1_1 = 1U;
const uint8_t NXDN_LICH_STEAL_FACCH    = 0U;

const uint8_t NXDN_LICH_DIRECTION_INBOUND  = 0U;
const uint8_t NXDN_LICH_DIRECTION_OUTBOUND = 1U;

const uint8_t NXDN_SR_SINGLE = 0U;
const uint8_t NXDN_SR_4_4    = 0U;
const uint8_t NXDN_SR_3_4    = 1U;
const uint8_t NXDN_SR_2_4    = 2U;
const uint8_t NXDN_SR_1_4    = 3U;

const uint8_t NXDN_MESSAGE_TYPE_VCALL           = 0x01U;
const uint8_t NXDN_MESSAGE_TYPE_VCALL_IV        = 0x03U;
const uint8_t NXDN_MESSAGE_TYPE_DCALL_HDR       = 0x09U;
const uint8_t NXDN_MESSAGE_TYPE_DCALL_DATA      = 0x0BU;
const uint8_t NXDN_MESSAGE_TYPE_DCALL_ACK       = 0x0CU;
const uint8_t NXDN_MESSAGE_TYPE_TX_REL          = 0x08U;
const uint8_t NXDN_MESSAGE_TYPE_HEAD_DLY        = 0x0FU;
const uint8_t NXDN_MESSAGE_TYPE_SDCALL_REQ_HDR  = 0x38U;
const uint8_t NXDN_MESSAGE_TYPE_SDCALL_REQ_DATA = 0x39U;
const uint8_t NXDN_MESSAGE_TYPE_SDCALL_RESP     = 0x3BU;
const uint8_t NXDN_MESSAGE_TYPE_SDCALL_IV       = 0x3AU;
const uint8_t NXDN_MESSAGE_TYPE_STAT_INQ_REQ    = 0x30U;
const uint8_t NXDN_MESSAGE_TYPE_STAT_INQ_RESP   = 0x31U;
const uint8_t NXDN_MESSAGE_TYPE_STAT_REQ        = 0x32U;
const uint8_t NXDN_MESSAGE_TYPE_STAT_RESP       = 0x33U;
const uint8_t NXDN_MESSAGE_TYPE_REM_CON_REQ     = 0x34U;
const uint8_t NXDN_MESSAGE_TYPE_REM_CON_RESP    = 0x35U;
const uint8_t NXDN_MESSAGE_TYPE_IDLE            = 0x10U;
const uint8_t NXDN_MESSAGE_TYPE_AUTH_INQ_REQ    = 0x28U;
const uint8_t NXDN_MESSAGE_TYPE_AUTH_INQ_RESP   = 0x29U;
const uint8_t NXDN_MESSAGE_TYPE_PROP_FORM       = 0x3FU;

const uint8_t NXDN_VOICE_CALL_OPTION_HALF_DUPLEX = 0x00U;
const uint8_t NXDN_VOICE_CALL_OPTION_DUPLEX      = 0x10U;

const uint8_t NXDN_DATA_CALL_OPTION_HALF_DUPLEX = 0x00U;
const uint8_t NXDN_DATA_CALL_OPTION_DUPLEX      = 0x10U;

const uint8_t NXDN_DATA_CALL_OPTION_4800 = 0x00U;
const uint8_t NXDN_DATA_CALL_OPTION_9600 = 0x02U;

const uint8_t SACCH_IDLE[] = { NXDN_MESSAGE_TYPE_IDLE, 0x00U, 0x00U };

const uint8_t NXDN_SILENCE[] = { 0xB9U, 0xE8U, 0x81U, 0x52U, 0x61U, 0x73U, 0x00U, 0x2AU, 0x6BU };

#endif
