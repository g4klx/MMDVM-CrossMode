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
m_mode(DATA_MODE_NONE),
m_srcCallsign(),
m_dstCallsign(),
m_dgId(0U),
m_srcId32(0U),
m_dstId32(0U),
m_srcId16(0U),
m_dstId16(0U),
m_group(false),
m_data(nullptr)
{
	assert(!callsign.empty());
	assert(dmrId > 0U);
	assert(nxdnId > 0U);
}

CData::~CData()
{
}

void CData::setDStar(const std::string& source, const std::string& destination, const uint8_t* data)
{
	assert(!source.empty());
	assert(!destination.empty());
	assert(data != nullptr);

	m_mode = DATA_MODE_DSTAR;

	m_srcCallsign = source;
	m_dstCallsign = destination;

	m_data = new uint8_t[DSTAR_DATA_LENGTH];
	::memcpy(m_data, data, DSTAR_DATA_LENGTH);
}

void CData::setDMR(uint32_t source, uint32_t destination, bool group, const uint8_t* data)
{
	assert(source > 0U);
	assert(destination > 0U);
	assert(data != nullptr);

	m_mode = DATA_MODE_DMR;

	m_srcId32 = source;
	m_dstId32 = destination;
	m_srcId16 = 0U;
	m_dstId16 = 0U;
	m_group   = group;

	m_data = new uint8_t[DMR_NXDN_DATA_LENGTH];
	::memcpy(m_data, data, DMR_NXDN_DATA_LENGTH);
}

void CData::setYSFDN(const std::string& source, uint8_t dgId, const uint8_t* data)
{
	assert(!source.empty());
	assert(data != nullptr);

	m_mode = DATA_MODE_YSFDN;

	m_srcCallsign = source;
	m_dgId        = dgId;

	m_data = new uint8_t[YSFDN_DATA_LENGTH];
	::memcpy(m_data, data, YSFDN_DATA_LENGTH);
}

void CData::setYSFVW(const std::string& source, uint8_t dgId, const uint8_t* data)
{
	assert(!source.empty());
	assert(data != nullptr);

	m_mode = DATA_MODE_YSFVW;

	m_srcCallsign = source;
	m_dgId        = dgId;

	m_data = new uint8_t[IMBE_FEC_DATA_LENGTH];
	::memcpy(m_data, data, IMBE_FEC_DATA_LENGTH);
}

void CData::setNXDN(uint16_t source, uint16_t destination, bool group, const uint8_t* data)
{
	assert(source > 0U);
	assert(destination > 0U);
	assert(data != nullptr);

	m_mode = DATA_MODE_NXDN;

	m_srcId16 = source;
	m_dstId16 = destination;
	m_srcId32 = 0U;
	m_dstId32 = 0U;
	m_group = group;

	m_data = new uint8_t[DMR_NXDN_DATA_LENGTH];
	::memcpy(m_data, data, DMR_NXDN_DATA_LENGTH);
}

void CData::setP25(uint32_t source, uint32_t destination, bool group, const uint8_t* data)
{
	assert(source > 0U);
	assert(destination > 0U);
	assert(data != nullptr);

	m_mode = DATA_MODE_P25;

	m_srcId32 = source;
	m_dstId32 = destination;
	m_srcId16 = 0U;
	m_dstId16 = 0U;
	m_group = group;

	m_data = new uint8_t[IMBE_FEC_DATA_LENGTH];
	::memcpy(m_data, data, IMBE_FEC_DATA_LENGTH);
}

void CData::setM17(const std::string& source, const std::string& destination, const uint8_t* data)
{
	assert(!source.empty());
	assert(!destination.empty());
	assert(data != nullptr);

	m_mode = DATA_MODE_M17;

	m_srcCallsign = source;
	m_dstCallsign = destination;

	m_data = new uint8_t[CODEC2_3200_DATA_LENGTH];
	::memcpy(m_data, data, CODEC2_3200_DATA_LENGTH);
}

void CData::setFM(const uint8_t* data)
{
	assert(data != nullptr);

	m_mode = DATA_MODE_FM;

	m_data = new uint8_t[PCM_DATA_LENGTH];
	::memcpy(m_data, data, PCM_DATA_LENGTH);
}

void CData::reset()
{
	m_mode = DATA_MODE_NONE;

	delete[] m_data;
	m_data = nullptr;
}
