/*
 *   Copyright (C) 2015,2016,2019,2021,2022,2024,2026 by Jonathan Naylor G4KLX
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

#include "DMRLC.h"

#include "Hamming.h"
#include "RS129.h"
#include "CRC.h"
#include "Log.h"

#include "Utils.h"

#include <cstring>
#include <cstdio>
#include <cassert>

CDMRLC::CDMRLC() :
m_PF(false),
m_R(false),
m_FLCO(FLCO::GROUP),
m_FID(0U),
m_options(0U),
m_srcId(0U),
m_dstId(0U),
m_bptc(),
m_raw(nullptr)
{
	m_raw = new bool[128U];
}

CDMRLC::~CDMRLC()
{
	delete[] m_raw;
}

void CDMRLC::setParameters(FLCO flco, uint32_t srcId, uint32_t dstId)
{
	m_FLCO  = flco;
	m_srcId = srcId;
	m_dstId = dstId;

	encodeEmbeddedData();
}

void CDMRLC::getData(uint8_t* bytes) const
{
	assert(bytes != nullptr);

	bytes[0U] = uint8_t(m_FLCO);

	if (m_PF)
		bytes[0U] |= 0x80U;

	if (m_R)
		bytes[0U] |= 0x40U;

	bytes[1U] = m_FID;

	bytes[2U] = m_options;

	bytes[3U] = m_dstId >> 16;
	bytes[4U] = m_dstId >> 8;
	bytes[5U] = m_dstId >> 0;

	bytes[6U] = m_srcId >> 16;
	bytes[7U] = m_srcId >> 8;
	bytes[8U] = m_srcId >> 0;
}

void CDMRLC::getData(bool* bits) const
{
	assert(bits != nullptr);

	uint8_t bytes[9U];
	getData(bytes);

	CUtils::byteToBitsBE(bytes[0U], bits + 0U);
	CUtils::byteToBitsBE(bytes[1U], bits + 8U);
	CUtils::byteToBitsBE(bytes[2U], bits + 16U);
	CUtils::byteToBitsBE(bytes[3U], bits + 24U);
	CUtils::byteToBitsBE(bytes[4U], bits + 32U);
	CUtils::byteToBitsBE(bytes[5U], bits + 40U);
	CUtils::byteToBitsBE(bytes[6U], bits + 48U);
	CUtils::byteToBitsBE(bytes[7U], bits + 56U);
	CUtils::byteToBitsBE(bytes[8U], bits + 64U);
}

void CDMRLC::encode(uint8_t* data, uint8_t type)
{
	assert(data != nullptr);

	uint8_t lcData[12U];
	getData(lcData);

	uint8_t parity[4U];
	CRS129::encode(lcData, 9U, parity);

	switch (type) {
	case DT_VOICE_LC_HEADER:
		lcData[9U]  = parity[2U] ^ VOICE_LC_HEADER_CRC_MASK[0U];
		lcData[10U] = parity[1U] ^ VOICE_LC_HEADER_CRC_MASK[1U];
		lcData[11U] = parity[0U] ^ VOICE_LC_HEADER_CRC_MASK[2U];
		break;

	case DT_TERMINATOR_WITH_LC:
		lcData[9U]  = parity[2U] ^ TERMINATOR_WITH_LC_CRC_MASK[0U];
		lcData[10U] = parity[1U] ^ TERMINATOR_WITH_LC_CRC_MASK[1U];
		lcData[11U] = parity[0U] ^ TERMINATOR_WITH_LC_CRC_MASK[2U];
		break;

	default:
		::LogError("Unsupported LC type - %d", int(type));
		return;
	}

	m_bptc.encode(lcData, data);
}

void CDMRLC::getData(uint8_t* data, uint8_t n) const
{
	assert(data != nullptr);
	assert(n < 4U);

	bool bits[40U];
	::memset(bits, 0x00U, 40U * sizeof(bool));
	::memcpy(bits + 4U, m_raw + n * 32U, 32U * sizeof(bool));

	unsigned char bytes[5U];
	CUtils::bitsToByteBE(bits + 0U, bytes[0U]);
	CUtils::bitsToByteBE(bits + 8U, bytes[1U]);
	CUtils::bitsToByteBE(bits + 16U, bytes[2U]);
	CUtils::bitsToByteBE(bits + 24U, bytes[3U]);
	CUtils::bitsToByteBE(bits + 32U, bytes[4U]);

	data[14U] = (data[14U] & 0xF0U) | (bytes[0U] & 0x0FU);
	data[15U] = bytes[1U];
	data[16U] = bytes[2U];
	data[17U] = bytes[3U];
	data[18U] = (data[18U] & 0x0FU) | (bytes[4U] & 0xF0U);
}

void CDMRLC::encodeEmbeddedData()
{
	bool lcData[72U];
	getData(lcData);

	unsigned int crc;
	CCRC::encodeFiveBit(lcData, crc);

	bool data[128U];
	::memset(data, 0x00U, 128U * sizeof(bool));

	data[106U] = (crc & 0x01U) == 0x01U;
	data[90U]  = (crc & 0x02U) == 0x02U;
	data[74U]  = (crc & 0x04U) == 0x04U;
	data[58U]  = (crc & 0x08U) == 0x08U;
	data[42U]  = (crc & 0x10U) == 0x10U;

	unsigned int b = 0U;
	for (unsigned int a = 0U; a < 11U; a++, b++)
		data[a] = lcData[b];
	for (unsigned int a = 16U; a < 27U; a++, b++)
		data[a] = lcData[b];
	for (unsigned int a = 32U; a < 42U; a++, b++)
		data[a] = lcData[b];
	for (unsigned int a = 48U; a < 58U; a++, b++)
		data[a] = lcData[b];
	for (unsigned int a = 64U; a < 74U; a++, b++)
		data[a] = lcData[b];
	for (unsigned int a = 80U; a < 90U; a++, b++)
		data[a] = lcData[b];
	for (unsigned int a = 96U; a < 106U; a++, b++)
		data[a] = lcData[b];

	// Hamming (16,11,4) check each row except the last one
	for (unsigned int a = 0U; a < 112U; a += 16U)
		CHamming::encode16114(data + a);

	// Add the parity bits for each column
	for (unsigned int a = 0U; a < 16U; a++)
		data[a + 112U] = data[a + 0U] ^ data[a + 16U] ^ data[a + 32U] ^ data[a + 48U] ^ data[a + 64U] ^ data[a + 80U] ^ data[a + 96U];

	// The data is packed downwards in columns
	b = 0U;
	for (unsigned int a = 0U; a < 128U; a++) {
		m_raw[a] = data[b];
		b += 16U;
		if (b > 127U)
			b -= 127U;
	}
}
