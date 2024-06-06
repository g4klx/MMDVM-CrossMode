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
#include <algorithm>

const std::string NULL_CALLSIGN = "";
const uint8_t NULL_DGID = 255U;

CData::CData(const std::string& port, uint32_t speed, bool debug, const std::string& callsign, uint32_t dmrId, uint16_t nxdnId) :
m_transcoder(port, speed, debug),
m_callsign(callsign),
m_dmrId(dmrId),
m_nxdnId(nxdnId),
m_toDStar(false),
m_toDMR(false),
m_toYSF(false),
m_toP25(false),
m_toNXDN(false),
m_toFM(false),
m_toM17(false),
m_dstarFMDest(),
m_dstarM17Dests(),
m_ysfDStarDGIds(),
m_ysfFMDGId(0U),
m_ysfM17DGIds(),
m_m17DStarDests(),
m_m17FMDest(),
m_fromMode(DATA_MODE_NONE),
m_toMode(DATA_MODE_NONE),
m_direction(DIR_NONE),
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
m_count(0U)
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

bool CData::setFromMode(DATA_MODE mode)
{
	m_fromMode = mode;

	return true;
}

bool CData::setToMode(DATA_MODE mode)
{
	m_toMode = mode;

	return setTranscoder();
}

bool CData::setDirection(DIRECTION direction)
{
	m_direction = direction;

	return setTranscoder();
}

bool CData::setTranscoder()
{
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
	case DATA_MODE_YSF:
		transFromMode = MODE_YSFDN;
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
	case DATA_MODE_YSF:
		transToMode = MODE_YSFDN;
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

	switch (m_direction) {
	case DIR_FROM_TO:
		return m_transcoder.setConversion(transFromMode, transToMode);
	case DIR_TO_FROM:
		return m_transcoder.setConversion(transToMode, transFromMode);
	default:
		return true;
	}
}

void CData::setToModes(bool toDStar, bool toDMR, bool toYSF, bool toP25, bool toNXDN, bool toFM, bool toM17)
{
	m_toDStar = toDStar;
	m_toDMR   = toDMR;
	m_toYSF   = toYSF;
	m_toP25   = toP25;
	m_toNXDN  = toNXDN;
	m_toFM    = toFM;
	m_toM17   = toM17;
}

DATA_MODE CData::getToMode() const
{
	return m_toMode;
}

void CData::setDStarFMDest(const std::string& dest)
{
	m_dstarFMDest = dest;
}

void CData::setDStarM17Dests(const std::vector<std::string>& dests)
{
	m_dstarM17Dests = dests;
}

void CData::setYSFDStarDGIds(const std::vector<std::pair<uint8_t, std::string>>& dgIds)
{
	m_ysfDStarDGIds = dgIds;
}

void CData::setYSFFMDGId(uint8_t dgId)
{
	m_ysfFMDGId = dgId;
}

void CData::setYSFM17DGIds(const std::vector<std::pair<uint8_t, std::string>>& dgIds)
{
	m_ysfM17DGIds = dgIds;
}

void CData::setM17DStarDests(const std::vector<std::string>& dests)
{
	m_m17DStarDests = dests;
}

void CData::setM17FMDest(const std::string& dest)
{
	m_m17FMDest = dest;
}

bool CData::open()
{
	return m_transcoder.open();
}

void CData::setDStar(NETWORK network, const uint8_t* source, const uint8_t* destination)
{
	assert(source != nullptr);
	assert(destination != nullptr);

	std::string srcCallsign = bytesToString(source, DSTAR_LONG_CALLSIGN_LENGTH);
	std::string dstCallsign = bytesToString(destination, DSTAR_LONG_CALLSIGN_LENGTH);

	if (network == NET_FROM) {
		m_toMode = DATA_MODE_NONE;

		if (dstCallsign == m_dstarFMDest)
			m_toMode = DATA_MODE_FM;

		if (m_toMode == DATA_MODE_NONE) {
			const auto it = std::find(m_dstarM17Dests.cbegin(), m_dstarM17Dests.cend(), dstCallsign);
			if (it != m_dstarM17Dests.cend()) {
				m_srcCallsign = srcCallsign;
				m_dstCallsign = dstCallsign;
				m_toMode      = DATA_MODE_M17;
			}
		}

		if (m_toDStar && (m_toMode == DATA_MODE_NONE)) {
			m_srcCallsign = srcCallsign;
			m_dstCallsign = dstCallsign;
			m_toMode      = DATA_MODE_DSTAR;
		}
	} else {
		if (m_toMode == DATA_MODE_NONE) {
			if (m_fromMode == DATA_MODE_YSF) {
				uint8_t dgId = find(m_ysfDStarDGIds, dstCallsign);
				if (dgId != NULL_DGID) {
					m_srcCallsign = srcCallsign;
					m_dgId        = dgId;
					m_toMode      = DATA_MODE_DSTAR;
				}
			}

			if (m_fromMode == DATA_MODE_M17) {
				const auto it = std::find(m_m17DStarDests.cbegin(), m_m17DStarDests.cend(), dstCallsign);
				if (it != m_m17DStarDests.cend()) {
					m_srcCallsign = srcCallsign;
					m_dstCallsign = dstCallsign;
					m_toMode      = DATA_MODE_DSTAR;
				}
			}

			if (m_toDStar && (m_toMode == DATA_MODE_NONE)) {
				m_srcCallsign = srcCallsign;
				m_dstCallsign = dstCallsign;
				m_toMode      = DATA_MODE_DSTAR;
			}
		}
	}
}

void CData::setDMR(NETWORK network, uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId32 = source;
	m_dstId32 = destination;
	m_group   = group;
}

void CData::setYSF(NETWORK network, const uint8_t* source, uint8_t dgId)
{
	assert(source != nullptr);

	std::string srcCallsign = bytesToString(source, YSF_CALLSIGN_LENGTH);

	if (network == NET_FROM) {
		m_toMode = DATA_MODE_NONE;

		std::string dest = find(m_ysfDStarDGIds, dgId);
		if (!dest.empty()) {
			m_srcCallsign = srcCallsign;
			m_dstCallsign = dest;
			m_toMode      = DATA_MODE_DSTAR;
		}

		if (m_toMode == DATA_MODE_NONE) {
			if (dgId == m_ysfFMDGId)
				m_toMode = DATA_MODE_FM;
		}

		if (m_toMode == DATA_MODE_NONE) {
			std::string dest = find(m_ysfM17DGIds, dgId);
			if (!dest.empty()) {
				m_srcCallsign = srcCallsign;
				m_dstCallsign = dest;
				m_toMode      = DATA_MODE_M17;
			}
		}

		if (m_toYSF && (m_toMode == DATA_MODE_NONE)) {
			m_srcCallsign = srcCallsign;
			m_dgId        = dgId;
			m_toMode      = DATA_MODE_YSF;
		}
	} else {
		if (m_toYSF && (m_toMode == DATA_MODE_NONE)) {
			m_srcCallsign = srcCallsign;
			m_dgId        = dgId;
			m_toMode      = DATA_MODE_YSF;
		}
	}
}

void CData::setNXDN(NETWORK network, uint16_t source, uint16_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId16 = source;
	m_dstId16 = destination;
	m_group   = group;
}

void CData::setP25(NETWORK network, uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	m_srcId32 = source;
	m_dstId32 = destination;
	m_group   = group;
}

void CData::setM17(NETWORK network, const std::string& source, const std::string& destination)
{
	if (network == NET_FROM) {
		m_toMode = DATA_MODE_NONE;

		const auto it = std::find(m_m17DStarDests.cbegin(), m_m17DStarDests.cend(), destination);
		if (it == m_m17DStarDests.end()) {
			m_srcCallsign = source;
			m_dstCallsign = destination;
			m_toMode      = DATA_MODE_DSTAR;
		}

		if (m_toMode == DATA_MODE_NONE) {
			if (destination == m_m17FMDest)
				m_toMode = DATA_MODE_FM;
		}

		if (m_toM17 && (m_toMode == DATA_MODE_NONE)) {
			m_srcCallsign = source;
			m_dstCallsign = destination;
			m_toMode      = DATA_MODE_M17;
		}
	} else {
		if (m_toMode == DATA_MODE_NONE) {
			if (m_fromMode == DATA_MODE_DSTAR) {
				const auto it = std::find(m_dstarM17Dests.cbegin(), m_dstarM17Dests.cend(), destination);
				if (it != m_dstarM17Dests.cend()) {
					m_srcCallsign = source;
					m_dstCallsign = destination;
					m_toMode      = DATA_MODE_M17;
				}
			}

			if (m_fromMode == DATA_MODE_YSF) {
				uint8_t dgId = find(m_ysfM17DGIds, destination);
				if (dgId != NULL_DGID) {
					m_srcCallsign = source;
					m_dgId        = dgId;
					m_toMode      = DATA_MODE_M17;
				}
			}

			if (m_toM17 && (m_toMode == DATA_MODE_NONE)) {
				m_srcCallsign = source;
				m_dstCallsign = destination;
				m_toMode      = DATA_MODE_M17;
			}
		}
	}
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

void CData::getDStar(NETWORK network, uint8_t* source, uint8_t* destination) const
{
	assert(source != nullptr);
	assert(destination != nullptr);

	stringToBytes(source,      DSTAR_LONG_CALLSIGN_LENGTH, m_srcCallsign);
	stringToBytes(destination, DSTAR_LONG_CALLSIGN_LENGTH, m_dstCallsign);
}

void CData::getYSF(NETWORK network, uint8_t* source, uint8_t* destination, uint8_t& dgId) const
{
	assert(source != nullptr);

	stringToBytes(source,      YSF_CALLSIGN_LENGTH, m_srcCallsign);
	stringToBytes(destination, YSF_CALLSIGN_LENGTH, m_dstCallsign);

	dgId = m_dgId;
}

void CData::getM17(NETWORK network, std::string& source, std::string& destination) const
{
	source      = m_srcCallsign;
	destination = m_dstCallsign;
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
	return m_fromMode != m_toMode;
}

void CData::reset()
{
	m_end       = false;
	m_length    = 0U;
	m_count     = 0U;
	m_rawLength = 0U;
	m_toMode    = DATA_MODE_NONE;
	m_direction = DIR_NONE;
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

uint8_t CData::find(const std::vector<std::pair<uint8_t, std::string>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (it.second == dest)
			return it.first;
	}

	return NULL_DGID;
}

std::string CData::find(const std::vector<std::pair<uint8_t, std::string>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (it.first == dgId)
			return it.second;
	}

	return NULL_CALLSIGN;
}
