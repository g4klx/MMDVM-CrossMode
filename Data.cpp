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
#include "Log.h"

#include <cstring>
#include <cassert>

CData::CData(const std::string& port, uint32_t speed, bool debug, const std::string& callsign, uint32_t dmrId, uint16_t nxdnId) :
m_transcoder(port, speed, debug),
m_defaultCallsign(callsign),
m_defaultDMRId(dmrId),
m_defaultNXDNId(nxdnId),
m_ysfM17Mapping(),
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
m_length(0U),
m_rawData(nullptr),
m_rawLength(0U),
m_count(0U),
m_transcode(false)
{
	assert(!callsign.empty());
	assert(dmrId > 0U);
	assert(nxdnId > 0U);

	// The longest data possible
	m_data    = new uint8_t[PCM_DATA_LENGTH];
	m_rawData = new uint8_t[1000U];
}

CData::~CData()
{
	delete[] m_data;
	delete[] m_rawData;
}

void CData::setYSFM17Mapping(const std::map<uint8_t, std::string>& mapping)
{
	m_ysfM17Mapping = mapping;
}

void CData::setM17YSFMapping(const std::map<std::string, uint8_t>& mapping)
{
	m_m17YSFMapping = mapping;
}

bool CData::open()
{
	return m_transcoder.open();
}

bool CData::setModes(DATA_MODE fromMode, DATA_MODE toMode)
{
	if ((m_fromMode == fromMode) && (m_toMode == toMode))
		return true;

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

	return m_transcoder.setConversion(transFromMode, transToMode);
}

void CData::setDStar(const uint8_t* source, const uint8_t* destination)
{
	assert(source != nullptr);
	assert(destination != nullptr);

	m_srcCallsign = bytesToString(source, DSTAR_LONG_CALLSIGN_LENGTH);
	m_dstCallsign = bytesToString(destination, DSTAR_LONG_CALLSIGN_LENGTH);

	LogMessage("From D-Star: src=%s dest=%s", m_srcCallsign.c_str(), m_dstCallsign.c_str());

	m_transcode = true;
}

void CData::setDMR(uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId32 = source;
	m_dstId32 = destination;
	m_group   = group;

	m_transcode = true;
}

void CData::setYSF(const uint8_t* source, uint8_t dgId)
{
	assert(source != nullptr);

	m_srcCallsign = bytesToString(source, YSF_CALLSIGN_LENGTH);
	m_dgId = dgId;

	auto it = m_ysfM17Mapping.find(dgId);
	if (it == m_ysfM17Mapping.end()) {
		m_dstCallsign.clear();
		m_transcode = false;
	} else {
		m_dstCallsign = it->second;
		LogMessage("From YSF: src=%s dgId=%u", m_srcCallsign.c_str(), dgId);
		m_transcode = true;
	}
}

void CData::setNXDN(uint16_t source, uint16_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId16 = source;
	m_dstId16 = destination;
	m_group   = group;

	m_transcode = true;
}

void CData::setP25(uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId32 = source;
	m_dstId32 = destination;
	m_group = group;

	m_transcode = true;
}

void CData::setM17(const std::string& source, const std::string& destination)
{
	m_srcCallsign = source;
	m_dstCallsign = destination;

	auto it = m_m17YSFMapping.find(destination);
	if (it == m_m17YSFMapping.end()) {
		m_dgId = 0U;
		m_transcode = false;
	} else {
		m_dgId = it->second;
		LogMessage("From M17: src=%s dest=%s", m_srcCallsign.c_str(), m_dstCallsign.c_str());
		m_transcode = true;
	}
}

void CData::setFM(const uint8_t* source)
{
	assert(source != nullptr);

	m_srcCallsign = bytesToString(source, ::strlen((char*)source));
}

void CData::setRaw(const uint8_t* data, uint16_t length)
{
	assert(data != nullptr);
	assert(length > 0U);

	::memcpy(m_rawData, data, length);
	m_rawLength = length;
}

bool CData::setData(const uint8_t* data)
{
	assert(data != nullptr);

	bool ret = m_transcoder.write(data);
	if (!ret)
		return false;

	m_count++;

	return true;
}

void CData::setEnd()
{
	m_end = true;
}

void CData::getDStar(uint8_t* source, uint8_t* destination) const
{
	assert(source != nullptr);
	assert(destination != nullptr);

	stringToBytes(source,      DSTAR_LONG_CALLSIGN_LENGTH, m_srcCallsign);
	stringToBytes(destination, DSTAR_LONG_CALLSIGN_LENGTH, m_dstCallsign);

	LogMessage("To D-Star: src=%s dest=%s", m_srcCallsign.c_str(), m_dstCallsign.c_str());
}

void CData::getYSF(uint8_t* source, uint8_t* destination, uint8_t& dgId) const
{
	assert(source != nullptr);

	stringToBytes(source,      YSF_CALLSIGN_LENGTH, m_srcCallsign);
	stringToBytes(destination, YSF_CALLSIGN_LENGTH, m_dstCallsign);

	dgId = m_dgId;
}

void CData::getM17(std::string& source, std::string& destination) const
{
	source      = m_srcCallsign;
	destination = m_dstCallsign;

	LogMessage("To M17: src=%s dest=%s", source.c_str(), destination.c_str());
}

void CData::getFM(uint8_t* source) const
{
	assert(source != nullptr);

	uint16_t length = m_srcCallsign.size();

	stringToBytes(source, length, m_srcCallsign);
	source[length] = 0x00U;
}

bool CData::hasRaw() const
{
	return m_rawLength > 0U;
}

bool CData::hasData() const
{
	return m_length > 0U;
}

uint16_t CData::getRaw(uint8_t* data)
{
	assert(data != nullptr);

	if (m_rawLength > 0U) {
		::memcpy(data, m_rawData, m_rawLength);
		uint16_t length = m_rawLength;
		m_rawLength     = 0U;
		return length;
	}

	return 0U;
}

bool CData::getData(uint8_t* data)
{
	assert(data != nullptr);

	if (m_length > 0U) {
		::memcpy(data, m_data, m_length);
		m_length = 0U;
		if (m_count > 0U)
			m_count--;
		return true;
	}

	return false;
}

bool CData::isEnd() const
{
	if (m_count > 0U)
		return false;

	return m_end;
}

bool CData::isTranscode() const
{
	return m_transcode;
}

void CData::reset()
{
	m_transcode = false;
	m_end       = false;
	m_length    = 0U;
	m_count     = 0U;
	m_rawLength = 0U;
}

void CData::clock(unsigned int ms)
{
	m_length = m_transcoder.read(m_data);
}

void CData::close()
{
	m_transcoder.close();
}

std::string CData::bytesToString(const uint8_t* str, size_t length) const
{
	assert(str != nullptr);

	std::string callsign;

	for (size_t i = 0U; i < length; i++) {
		if ((str[i] != ' ') && (str[i] != '/') && (str[i] != '-'))
			callsign += char(str[i]);
		else
			break;
	}

	return callsign;
}

void CData::stringToBytes(uint8_t* str, size_t length, const std::string& callsign) const
{
	assert(str != nullptr);

	::memset(str, ' ', length);

	size_t len = callsign.length();
	if (len > length)
		len = length;

	for (size_t i = 0U; i < len; i++)
		str[i] = callsign[i];
}
