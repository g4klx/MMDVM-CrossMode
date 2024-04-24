/*
 *	 Copyright (C) 2024 by Jonathan Naylor G4KLX
 *
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the Free Software
 *	 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Data.h"

#include <cassert>

CData::CData(const std::string& callsign, uint32_t dmrId, uint16_t nxdnId) :
m_defaultCallsign(callsign),
m_defaultDMRId(dmrId),
m_defaultNXDNId(nxdnId),
m_fromMode(DATA_MODE_NONE),
m_toMode(DATA_MODE_NONE),
m_srcCallsign(),
m_dstCallsign(),
m_dgId(0U),
m_srcId32(0U),
m_dstId32(0U),
m_srcId16(0U),
m_dstId16(0U),
m_group(false),
m_end(false),
m_data(nullptr),
m_length(0U)
{
	assert(!callsign.empty());
	assert(dmrId > 0U);
	assert(nxdnId > 0U);

	// The longest data possible
	m_data = new uint8_t[PCM_DATA_LENGTH];
}

CData::~CData()
{
	delete[] m_data;
}

void CData::setModes(DATA_MODE fromMode, DATA_MODE toMode)
{
	m_fromMode = fromMode;
	m_toMode   = toMode;

	switch (m_fromMode) {
	case DATA_MODE_DSTAR:
		m_length = DSTAR_DATA_LENGTH;
		break;
	case DATA_MODE_DMR:
	case DATA_MODE_NXDN:
		m_length = DMR_NXDN_DATA_LENGTH;
		break;
	case DATA_MODE_YSFDN:
		m_length = YSFDN_DATA_LENGTH;
		break;
	case DATA_MODE_YSFVW:
		m_length = IMBE_FEC_DATA_LENGTH;
		break;
	case DATA_MODE_P25:
		m_length = IMBE_DATA_LENGTH;
		break;
	case DATA_MODE_FM:
		m_length = PCM_DATA_LENGTH;
		break;
	case DATA_MODE_M17:
		m_length = CODEC2_3200_DATA_LENGTH;
		break;
	default:
		break;
	}
}

void CData::setDStar(const std::string& source, const std::string& destination)
{
	assert(!source.empty());
	assert(!destination.empty());

	m_srcCallsign = source;
	m_dstCallsign = destination;
}

void CData::setDMR(uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId32 = source;
	m_dstId32 = destination;
	m_group   = group;
}

void CData::setYSF(const std::string& source, uint8_t dgId)
{
	assert(!source.empty());

	m_srcCallsign = source;
	m_dgId        = dgId;
}

void CData::setNXDN(uint16_t source, uint16_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId16 = source;
	m_dstId16 = destination;
	m_group   = group;
}

void CData::setP25(uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId32 = source;
	m_dstId32 = destination;
	m_group   = group;
}

void CData::setM17(const std::string& source, const std::string& destination)
{
	assert(!source.empty());
	assert(!destination.empty());

	m_srcCallsign = source;
	m_dstCallsign = destination;
}

void CData::setData(const uint8_t* data)
{
	assert(data != nullptr);

	::memcpy(m_data, data, m_length);
}

void CData::setEnd()
{
	m_end = true;
}

bool CData::isEnd() const
{
	return m_end;
}

void CData::reset()
{
	m_end = false;
}
