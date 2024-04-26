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

#include "DStarDefines.h"
#include "YSFDefines.h"
#include "M17Defines.h"

#include <cassert>

CData::CData(const std::string& port, uint32_t speed, const std::string& callsign, uint32_t dmrId, uint16_t nxdnId) :
m_transoder(port, speed),
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

bool CData::open()
{
	return m_transoder.open();
}

bool CData::setModes(DATA_MODE fromMode, DATA_MODE toMode)
{
	m_fromMode = fromMode;
	m_toMode   = toMode;

	uint8_t transFromMode;
	uint8_t transToMode;

	switch (m_fromMode) {
	case DATA_MODE_DSTAR:
		transFromMode = MODE_DSTAR;
		break;
	case DATA_MODE_DMR:
	case DATA_MODE_NXDN:
		transFromMode = MODE_DMR_NXDN;
		break;
	case DATA_MODE_YSFDN:
		transFromMode = MODE_YSFDN;
		break;
	case DATA_MODE_YSFVW:
		transFromMode = MODE_IMBE_FEC;
		break;
	case DATA_MODE_P25:
		transFromMode = MODE_IMBE;
		break;
	case DATA_MODE_FM:
		transFromMode = MODE_PCM;
		break;
	case DATA_MODE_M17:
		transFromMode = MODE_CODEC2_3200;
		break;
	default:
		return false;
	}

	switch (m_toMode) {
	case DATA_MODE_DSTAR:
		transToMode = MODE_DSTAR;
		break;
	case DATA_MODE_DMR:
	case DATA_MODE_NXDN:
		transToMode = MODE_DMR_NXDN;
		break;
	case DATA_MODE_YSFDN:
		transToMode = MODE_YSFDN;
		break;
	case DATA_MODE_YSFVW:
		transToMode = MODE_IMBE_FEC;
		break;
	case DATA_MODE_P25:
		transToMode = MODE_IMBE;
		break;
	case DATA_MODE_FM:
		transToMode = MODE_PCM;
		break;
	case DATA_MODE_M17:
		transToMode = MODE_CODEC2_3200;
		break;
	default:
		return false;
	}

	return m_transoder.setConversion(fromMode, toMode);
}

void CData::setDStar(const uint8_t* source, const uint8_t* destination)
{
	assert(source != nullptr);
	assert(destination != nullptr);

	m_srcCallsign = bytesToString(source, DSTAR_LONG_CALLSIGN_LENGTH);
	m_dstCallsign = bytesToString(destination, DSTAR_LONG_CALLSIGN_LENGTH);
}

void CData::setDMR(uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId32 = source;
	m_dstId32 = destination;
	m_group   = group;
}

void CData::setYSF(const uint8_t* source, uint8_t dgId)
{
	assert(source != nullptr);

	m_srcCallsign = bytesToString(source, YSF_CALLSIGN_LENGTH);
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

bool CData::setData(const uint8_t* data)
{
	assert(data != nullptr);

	return m_transoder.write(data);
}

void CData::setEnd()
{
	m_end = true;
}

void CData::getDStar(uint8_t* source, uint8_t* destination) const
{
	assert(source != nullptr);
	assert(destination != nullptr);

	// This is only true for M17
	stringToBytes(source,      DSTAR_LONG_CALLSIGN_LENGTH, m_srcCallsign);
	stringToBytes(destination, DSTAR_LONG_CALLSIGN_LENGTH, m_dstCallsign);
}

void CData::getM17(uint8_t* source, uint8_t* destination) const
{
	assert(source != nullptr);
	assert(destination != nullptr);

	// This is only true for D-Star
	stringToBytes(source,      M17_CALLSIGN_LENGTH, m_srcCallsign);
	stringToBytes(destination, M17_CALLSIGN_LENGTH, m_dstCallsign);
}

bool CData::hasData() const
{
	return m_length > 0U;
}

bool CData::getData(uint8_t* data)
{
	assert(data != nullptr);

	if (m_length > 0U) {
		::memcpy(data, m_data, m_length);
		m_length = 0U;
		return true;
	}

	return false;
}

bool CData::isEnd() const
{
	return m_end;
}

void CData::reset()
{
	m_end    = false;
	m_length = 0U;
}

void CData::clock(unsigned int ms)
{
	m_length = m_transoder.read(m_data);
}

void CData::close()
{
	m_transoder.close();
}

std::string CData::bytesToString(const uint8_t* str, unsigned int length) const
{
	assert(str != nullptr);

	std::string callsign;

	for (unsigned int i = 0U; i < length; i++) {
		if ((str[i] != ' ') && (str[i] != '/') && (str[i] != '-'))
			callsign += char(str[i]);
		else
			break;
	}

	return callsign;
}

void CData::stringToBytes(uint8_t* str, unsigned int length, const std::string& callsign) const
{
	assert(str != nullptr);

	::memset(str, ' ', length);

	unsigned int len = callsign.length();
	if (len > length)
		len = length;

	for (unsigned int i = 0U; i < len; i++)
		str[i] = callsign[i];
}
