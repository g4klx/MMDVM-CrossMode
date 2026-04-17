/*
 *	 Copyright (C) 2024,2026 by Jonathan Naylor G4KLX
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

#include "MetaData.h"

#include "DStarDefines.h"
#include "YSFDefines.h"
#include "Utils.h"
#include "Log.h"

#include <cstring>
#include <cassert>
#include <algorithm>

const std::string NULL_CALLSIGN = "";
const uint8_t  NULL_DGID = 0xFFU;
const uint8_t  NULL_SLOT = 0U;
const uint16_t NULL_ID16 = 0xFFFFU;
const uint32_t NULL_ID32 = 0xFFFFFFFFU;

CMetaData::CMetaData(const std::string& callsign, uint32_t dmrId, uint16_t nxdnId, bool debug) :
m_transcoder(debug),
m_defaultCallsign(callsign),
m_defaultDMRId(dmrId),
m_defaultNXDNId(nxdnId),
m_debug(debug),
m_dmrLookup(),
m_nxdnLookup(),
m_toDStar(false),
m_toDMR1(false),
m_toDMR2(false),
m_toYSF(false),
m_toP25(false),
m_toNXDN(false),
m_toFM(false),
m_dstarDMRDests(),
m_dstarYSFDests(),
m_dstarP25Dests(),
m_dstarNXDNDests(),
m_dstarFMDest(NULL_CALLSIGN),
m_dmrDStarTGs(),
m_dmrYSFTGs(),
m_dmrP25TGs(),
m_dmrNXDNTGs(),
m_dmrFMTG(),
m_ysfDStarDGIds(),
m_ysfDMRDGIds(),
m_ysfP25DGIds(),
m_ysfNXDNDGIds(),
m_ysfFMDGId(NULL_DGID),
m_p25DStarTGs(),
m_p25DMRTGs(),
m_p25YSFTGs(),
m_p25NXDNTGs(),
m_p25FMTG(),
m_nxdnDStarTGs(),
m_nxdnDMRTGs(),
m_nxdnYSFTGs(),
m_nxdnP25TGs(),
m_nxdnFMTG(),
m_fromMode(DATA_MODE::NONE),
m_toMode(DATA_MODE::NONE),
m_direction(DIRECTION::NONE),
m_srcCallsign(),
m_dstCallsign(),
m_dgId(NULL_DGID),
m_slot(NULL_SLOT),
m_srcId(NULL_ID32),
m_dstId(NULL_ID32),
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

CMetaData::~CMetaData()
{
	delete[] m_data;
	delete[] m_rawData;
}

void CMetaData::setUARTConnection(const std::string& port, uint32_t speed)
{
	m_transcoder.setUARTConnection(port, speed);
}

void CMetaData::setUDPConnection(const std::string& remoteAddress, uint16_t remotePort, const std::string& localAddress, uint16_t localPort)
{
	m_transcoder.setUDPConnection(remoteAddress, remotePort, localAddress, localPort);
}

bool CMetaData::setFromMode(DATA_MODE mode)
{
	m_fromMode = mode;

	return true;
}

bool CMetaData::setToMode(DATA_MODE mode)
{
	m_toMode = mode;

	return setTranscoder();
}

bool CMetaData::setDirection(DIRECTION direction)
{
	m_direction = direction;

	return setTranscoder();
}

bool CMetaData::setTranscoder()
{
	uint8_t transFromMode;
	uint8_t transToMode;

	switch (m_fromMode) {
	case DATA_MODE::DSTAR:
		transFromMode = MODE_DSTAR;
		break;
	case DATA_MODE::DMR:
	case DATA_MODE::NXDN:
		transFromMode = MODE_DMR_NXDN;
		break;
	case DATA_MODE::YSF:
		transFromMode = MODE_YSFDN;
		break;
	case DATA_MODE::P25:
		transFromMode = MODE_IMBE;
		break;
	case DATA_MODE::FM:
		transFromMode = MODE_PCM;
		break;
	default:
		return false;
	}

	switch (m_toMode) {
	case DATA_MODE::DSTAR:
		transToMode = MODE_DSTAR;
		break;
	case DATA_MODE::DMR:
	case DATA_MODE::NXDN:
		transToMode = MODE_DMR_NXDN;
		break;
	case DATA_MODE::YSF:
		transToMode = MODE_YSFDN;
		break;
	case DATA_MODE::P25:
		transToMode = MODE_IMBE;
		break;
	case DATA_MODE::FM:
		transToMode = MODE_PCM;
		break;
	default:
		return false;
	}

	switch (m_direction) {
	case DIRECTION::FROM_TO:
		return m_transcoder.setConversion(transFromMode, transToMode);
	case DIRECTION::TO_FROM:
		return m_transcoder.setConversion(transToMode, transFromMode);
	default:
		return true;
	}
}

void CMetaData::setThroughModes(bool toDStar, bool toDMR1, bool toDMR2, bool toYSF, bool toP25, bool toNXDN, bool toFM)
{
	m_toDStar = toDStar;
	m_toDMR1  = toDMR1;
	m_toDMR2  = toDMR2;
	m_toYSF   = toYSF;
	m_toP25   = toP25;
	m_toNXDN  = toNXDN;
	m_toFM    = toFM;
}

bool CMetaData::setDMRLookup(const std::string& filename, unsigned int reloadTime)
{
	return m_dmrLookup.load(filename, reloadTime);
}

bool CMetaData::setNXDNLookup(const std::string& filename, unsigned int reloadTime)
{
	return m_nxdnLookup.load(filename, reloadTime);
}

DATA_MODE CMetaData::getToMode() const
{
	return m_toMode;
}

void CMetaData::setDStarDMRDests(const std::vector<std::tuple<std::string, uint8_t, uint32_t>>& dests)
{
	m_dstarDMRDests = dests;
}

void CMetaData::setDStarYSFDests(const std::vector<std::pair<std::string, uint8_t>>& dests)
{
	m_dstarYSFDests = dests;
}

void CMetaData::setDStarP25Dests(const std::vector<std::pair<std::string, uint32_t>>& dests)
{
	m_dstarP25Dests = dests;
}

void CMetaData::setDStarNXDNDests(const std::vector<std::pair<std::string, uint16_t>>& dests)
{
	m_dstarNXDNDests = dests;
}

void CMetaData::setDStarFMDest(const std::string& dest)
{
	m_dstarFMDest = dest;
}

void CMetaData::setDMRDStarTGs(const std::vector<std::tuple<uint8_t, uint32_t, std::string>>& tgs)
{
	m_dmrDStarTGs = tgs;
}

void CMetaData::setDMRYSFTGs(const std::vector<std::tuple<uint8_t, uint32_t, uint8_t>>& tgs)
{
	m_dmrYSFTGs = tgs;
}

void CMetaData::setDMRP25TGs(const std::vector<std::tuple<uint8_t, uint32_t, uint32_t>>& tgs)
{
	m_dmrP25TGs = tgs;
}

void CMetaData::setDMRNXDNTGs(const std::vector<std::tuple<uint8_t, uint32_t, uint16_t>>& tgs)
{
	m_dmrNXDNTGs = tgs;
}

void CMetaData::setDMRFMTG(const std::pair<uint8_t, uint32_t>& tg)
{
	m_dmrFMTG = tg;
}

void CMetaData::setYSFDStarDGIds(const std::vector<std::pair<uint8_t, std::string>>& dgIds)
{
	m_ysfDStarDGIds = dgIds;
}

void CMetaData::setYSFDMRDGIds(const std::vector<std::tuple<uint8_t, uint8_t, uint32_t>>& dgIds)
{
	m_ysfDMRDGIds = dgIds;
}

void CMetaData::setYSFP25DGIds(const std::vector<std::pair<uint8_t, uint32_t>>& dgIds)
{
	m_ysfP25DGIds = dgIds;
}

void CMetaData::setYSFNXDNDGIds(const std::vector<std::pair<uint8_t, uint16_t>>& dgIds)
{
	m_ysfNXDNDGIds = dgIds;
}

void CMetaData::setYSFFMDGId(uint8_t dgId)
{
	m_ysfFMDGId = dgId;
}

void CMetaData::setP25DStarTGs(const std::vector<std::pair<uint32_t, std::string>>& tgs)
{
	m_p25DStarTGs = tgs;
}

void CMetaData::setP25DMRTGs(const std::vector<std::tuple<uint32_t, uint8_t, uint32_t>>& tgs)
{
	m_p25DMRTGs = tgs;
}

void CMetaData::setP25YSFTGs(const std::vector<std::pair<uint32_t, uint8_t>>& tgs)
{
	m_p25YSFTGs = tgs;
}

void CMetaData::setP25NXDNTGs(const std::vector<std::pair<uint32_t, uint16_t>>& tgs)
{
	m_p25NXDNTGs = tgs;
}

void CMetaData::setP25FMTG(uint32_t tg)
{
	m_p25FMTG = tg;
}

void CMetaData::setNXDNDStarTGs(const std::vector<std::pair<uint16_t, std::string>>& tgs)
{
	m_nxdnDStarTGs = tgs;
}

void CMetaData::setNXDNDMRTGs(const std::vector<std::tuple<uint16_t, uint8_t, uint32_t>>& tgs)
{
	m_nxdnDMRTGs = tgs;
}

void CMetaData::setNXDNYSFTGs(const std::vector<std::pair<uint16_t, uint8_t>>& tgs)
{
	m_nxdnYSFTGs = tgs;
}

void CMetaData::setNXDNP25TGs(const std::vector<std::pair<uint16_t, uint32_t>>& tgs)
{
	m_nxdnP25TGs = tgs;
}

void CMetaData::setNXDNFMTG(uint16_t tg)
{
	m_nxdnFMTG = tg;
}

bool CMetaData::open()
{
	writeJSONStatus(DATA_MODE::NONE, DATA_MODE::NONE);

	return m_transcoder.open();
}

void CMetaData::setDStar(NETWORK network, const uint8_t* source, const uint8_t* destination)
{
	assert(source != nullptr);
	assert(destination != nullptr);

	std::string srcCallsign = bytesToString(source, DSTAR_LONG_CALLSIGN_LENGTH);
	std::string dstCallsign = bytesToString(destination, DSTAR_LONG_CALLSIGN_LENGTH);

	if (network == NETWORK::FROM) {
		m_toMode = DATA_MODE::NONE;

		std::pair<uint8_t, uint32_t> dst = find(m_dstarDMRDests, dstCallsign);
		if (dst.second != NULL_ID32) {
			uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
			if (srcId == NULL_ID32)
				return;

			LogDebug("D-Star => DMR, %s>%s -> %u>%u:TG%u", srcCallsign.c_str(), dstCallsign.c_str(), srcId, dst.first, dst.second);

			m_slot   = dst.first;
			m_dstId  = dst.second;
			m_srcId  = srcId;
			m_group  = true;
			m_toMode = DATA_MODE::DMR;

			writeJSONStatus(DATA_MODE::DSTAR, DATA_MODE::DMR, 0U, m_dstId, false, true, dstCallsign, "", 0U, m_slot);
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_dstarYSFDests, dstCallsign);
			if (dgId != NULL_DGID) {
				LogDebug("D-Star => YSF, %s>%s -> %s>%u", srcCallsign.c_str(), dstCallsign.c_str(), srcCallsign.c_str(), dgId);

				m_dgId        = dgId;
				m_srcCallsign = srcCallsign;
				m_toMode      = DATA_MODE::YSF;

				writeJSONStatus(DATA_MODE::DSTAR, DATA_MODE::YSF, 0U, m_dgId, false, false, dstCallsign);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint32_t dstId = find(m_dstarP25Dests, dstCallsign);
			if (dstId != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId == NULL_ID32)
					return;

				LogDebug("D-Star => P25, %s>%s -> %u>TG%u", srcCallsign.c_str(), dstCallsign.c_str(), srcId, dstId);

				m_dstId  = dstId;
				m_srcId  = srcId;
				m_group  = true;
				m_toMode = DATA_MODE::P25;

				writeJSONStatus(DATA_MODE::DSTAR, DATA_MODE::P25, 0U, m_dstId, false, true, dstCallsign);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint16_t dstId = find(m_dstarNXDNDests, dstCallsign);
			if (dstId != NULL_ID16) {
				uint16_t srcId = m_nxdnLookup.lookup(srcCallsign);
				if (srcId == NULL_ID16)
					return;

				LogDebug("D-Star => NXDN, %s>%s -> %u>TG%u", srcCallsign.c_str(), dstCallsign.c_str(), srcId, dstId);

				m_dstId  = dstId;
				m_srcId  = srcId;
				m_group  = true;
				m_toMode = DATA_MODE::NXDN;

				writeJSONStatus(DATA_MODE::DSTAR, DATA_MODE::NXDN, 0U, m_dstId, false, true, dstCallsign);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if (dstCallsign == m_dstarFMDest) {
				LogDebug("D-Star => FM, %s>%s ->", srcCallsign.c_str(), dstCallsign.c_str());

				m_toMode = DATA_MODE::FM;

				writeJSONStatus(DATA_MODE::DSTAR, DATA_MODE::FM, 0U, 0U, false, false, dstCallsign);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if (m_toDStar) {
				LogDebug("D-Star => D-Star, %s>%s -> %s>%s", srcCallsign.c_str(), dstCallsign.c_str(), srcCallsign.c_str(), dstCallsign.c_str());

				m_srcCallsign = srcCallsign;
				m_dstCallsign = dstCallsign;
				m_toMode      = DATA_MODE::DSTAR;

				writeJSONStatus(DATA_MODE::DSTAR, DATA_MODE::DSTAR, 0U, 0U, false, false, dstCallsign, dstCallsign);
			}
		}
	} else {
		if (m_fromMode == DATA_MODE::DMR) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_dmrDStarTGs, dstCallsign);
			if (slotTG.second != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId != NULL_ID32) {
					LogDebug("D-Star <= DMR, %s>%s -> %u>%u:TG%u", srcCallsign.c_str(), dstCallsign.c_str(), srcId, slotTG.first, slotTG.second);

					m_slot   = slotTG.first;
					m_srcId  = srcId;
					m_dstId  = slotTG.second;
					m_group  = true;
					m_toMode = DATA_MODE::DSTAR;
				}
			}
		}

		if (m_fromMode == DATA_MODE::YSF) {
			uint8_t dgId = find(m_ysfDStarDGIds, dstCallsign);
			if (dgId != NULL_DGID) {
				LogDebug("D-Star <= YSF, %s>%s -> %s>%u", srcCallsign.c_str(), dstCallsign.c_str(), srcCallsign.c_str(), dgId);

				m_srcCallsign = srcCallsign;
				m_dgId        = dgId;
				m_toMode      = DATA_MODE::DSTAR;
			}
		}

		if (m_fromMode == DATA_MODE::P25) {
			uint32_t tg = find(m_p25DStarTGs, dstCallsign);
			if (tg != NULL_ID32) {
				uint32_t id = m_dmrLookup.lookup(srcCallsign);
				if (id != NULL_ID32) {
					LogDebug("D-Star <= P25, %s>%s -> %u>TG%u", srcCallsign.c_str(), dstCallsign.c_str(), id, tg);

					m_srcId  = id;
					m_dstId  = tg;
					m_group  = true;
					m_toMode = DATA_MODE::DSTAR;
				}
			}
		}

		if (m_fromMode == DATA_MODE::NXDN) {
			uint16_t tg = find(m_nxdnDStarTGs, dstCallsign);
			if (tg != NULL_ID16) {
				uint16_t id = m_nxdnLookup.lookup(srcCallsign);
				if (id != NULL_ID16) {
					LogDebug("D-Star <= NXDN, %s>%s -> %u>TG%u", srcCallsign.c_str(), dstCallsign.c_str(), id, tg);

					m_srcId  = id;
					m_dstId  = tg;
					m_group  = true;
					m_toMode = DATA_MODE::DSTAR;
				}
			}
		}

		else if (m_fromMode == DATA_MODE::FM) {
			if (dstCallsign == m_dstarFMDest) {
				LogDebug("D-Star <= FM, -> %s>%s", srcCallsign.c_str(), dstCallsign.c_str());

				m_toMode = DATA_MODE::DSTAR;
			}
		}

		else if (m_fromMode == DATA_MODE::DSTAR) {
			if (m_toDStar) {
				LogDebug("D-Star <= D-Star, %s>%s -> %s>%s", srcCallsign.c_str(), dstCallsign.c_str(), srcCallsign.c_str(), dstCallsign.c_str());

				m_srcCallsign = srcCallsign;
				m_dstCallsign = dstCallsign;
				m_toMode      = DATA_MODE::DSTAR;
			}
		}

		else
			m_toMode = DATA_MODE::NONE;
	}
}

void CMetaData::setDMR(NETWORK network, uint8_t slot, uint32_t source, uint32_t destination, bool group)
{
	assert((slot == 1U) || (slot == 2U));
	assert(source > 0U);
	assert(destination > 0U);

	if (network == NETWORK::FROM) {
		m_toMode = DATA_MODE::NONE;

		std::string dst = find(m_dmrDStarTGs, slot, destination);
		if (dst != NULL_CALLSIGN) {
			std::string src = m_dmrLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("DMR => D-Star, %u>%u:TG%u -> %s>%s", source, slot, destination, src.c_str(), dst.c_str());

				m_srcCallsign = src;
				m_dstCallsign = dst;
				m_toMode      = DATA_MODE::DSTAR;

				writeJSONStatus(DATA_MODE::DMR, DATA_MODE::DSTAR, destination, 0U, true, false, "", m_dstCallsign, slot);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_dmrYSFTGs, slot, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("DMR => YSF, %u>%u:TG%u -> %s>%u", source, slot, destination, src.c_str(), dgId);

					m_srcCallsign = src;
					m_dgId        = dgId;
					m_toMode      = DATA_MODE::YSF;

					writeJSONStatus(DATA_MODE::DMR, DATA_MODE::YSF, destination, dgId, true, false, "", "", slot);
				}
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint32_t tg = find(m_dmrP25TGs, slot, destination);
			if (tg != NULL_ID32) {
				LogDebug("DMR => P25, %u>%u:TG%u -> %u>TG%u", source, slot, destination, source, tg);

				m_srcId  = source;
				m_dstId  = tg;
				m_group  = true;
				m_toMode = DATA_MODE::P25;

				writeJSONStatus(DATA_MODE::DMR, DATA_MODE::P25, destination, tg, true, true, "", "", slot);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint16_t tg = find(m_dmrNXDNTGs, slot, destination);
			if (tg != NULL_ID16) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint16_t id = m_nxdnLookup.lookup(src);
					if (id != NULL_ID16) {
						LogDebug("DMR => NXDN, %u>%u:TG%u -> %u>TG%u", source, slot, destination, id, tg);

						m_srcId  = id;
						m_dstId  = tg;
						m_group  = true;
						m_toMode = DATA_MODE::NXDN;

						writeJSONStatus(DATA_MODE::DMR, DATA_MODE::NXDN, destination, tg, true, true, "", "", slot);
					}
				}
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> tg = std::make_pair(slot, destination);
			if (tg == m_dmrFMTG) {
				LogDebug("DMR => FM, %u>%u:TG%u ->", source, slot, destination);

				m_toMode = DATA_MODE::FM;

				writeJSONStatus(DATA_MODE::DMR, DATA_MODE::FM, destination, 0U, true, false, "", "", slot);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if ((slot == 1U) && m_toDMR1) {
				LogDebug("DMR => DMR, %u>%u:TG%u -> %u>%u:TG%u", source, slot, destination, source, slot, destination);

				m_slot   = slot;
				m_srcId  = source;
				m_dstId  = destination;
				m_group  = group;
				m_toMode = DATA_MODE::DMR;

				writeJSONStatus(DATA_MODE::DMR, DATA_MODE::DMR, destination, destination, true, true, "", "", slot, slot);
			}
			if ((slot == 2U) && m_toDMR2) {
				LogDebug("DMR => DMR, %u>%u:TG%u -> %u>%u:TG%u", source, slot, destination, source, slot, destination);

				m_slot   = slot;
				m_srcId  = source;
				m_dstId  = destination;
				m_group  = group;
				m_toMode = DATA_MODE::DMR;

				writeJSONStatus(DATA_MODE::DMR, DATA_MODE::DMR, destination, destination, true, true, "", "", slot, slot);
			}
		}
	} else {
		if (m_fromMode == DATA_MODE::DSTAR) {
			std::string dest = find(m_dstarDMRDests, slot, destination);
			if (dest != NULL_CALLSIGN) {
				std::string src = m_dmrLookup.lookup(source);
				if (src == NULL_CALLSIGN)
					return;

				LogDebug("DMR <= D-Star, %u>%u:TG%u -> %s>%s", source, slot, destination, src.c_str(), dest.c_str());

				m_srcCallsign = src;
				m_dstCallsign = dest;
				m_toMode      = DATA_MODE::DMR;
			}
		}

		else if (m_fromMode == DATA_MODE::YSF) {
			uint8_t dgId = find(m_ysfDMRDGIds, slot, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src == NULL_CALLSIGN)
					return;

				LogDebug("DMR <= YSF, %u>%u:TG%u -> %s>%u", source, slot, destination, src.c_str(), dgId);

				m_srcCallsign = src;
				m_dgId        = dgId;
				m_toMode      = DATA_MODE::DMR;
			}
		}

		else if (m_fromMode == DATA_MODE::P25) {
			uint32_t tg = find(m_p25DMRTGs, slot, destination);
			if (tg != NULL_ID32) {
				LogDebug("DMR <= P25, %u>%u:TG%u -> %u>TG%u", source, slot, destination, source, tg);

				m_srcId  = source;
				m_dstId  = tg;
				m_group  = true;
				m_toMode = DATA_MODE::DMR;
			}
		}

		else if (m_fromMode == DATA_MODE::NXDN) {
			uint16_t tg = find(m_nxdnDMRTGs, slot, destination);
			if (tg != NULL_ID16) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint16_t id = m_nxdnLookup.lookup(src);
					if (id != NULL_ID16) {
						LogDebug("DMR <= NXDN, %u>%u:TG%u -> %u>TG%u", source, slot, destination, id, tg);

						m_srcId  = id;
						m_dstId  = tg;
						m_group  = true;
						m_toMode = DATA_MODE::DMR;
					}
				}
			}
		}

		if (m_fromMode == DATA_MODE::FM) {
			std::pair<uint8_t, uint32_t> tg = std::make_pair(slot, destination);
			if (tg == m_dmrFMTG) {
				LogDebug("DMR <= FM, -> %u>%u:TG%u", source, slot, destination);

				m_toMode = DATA_MODE::DMR;
			}
		}

		else if (m_fromMode == DATA_MODE::DMR) {
			if ((slot == 1U) && m_toDMR1) {
				LogDebug("DMR <= DMR, %u>%u:TG%u -> %u>%u:TG%u", source, slot, destination, source, slot, destination);

				m_slot   = slot;
				m_srcId  = source;
				m_dstId  = destination;
				m_group  = group;
				m_toMode = DATA_MODE::DMR;
			}
			if ((slot == 2U) && m_toDMR2) {
				LogDebug("DMR <= DMR, %u>%u:TG%u -> %u>%u:TG%u", source, slot, destination, source, slot, destination);

				m_slot   = slot;
				m_srcId  = source;
				m_dstId  = destination;
				m_group  = group;
				m_toMode = DATA_MODE::DMR;
			}
		}

		else
			m_toMode = DATA_MODE::NONE;
	}
}

void CMetaData::setYSF(NETWORK network, const uint8_t* source, uint8_t dgId)
{
	assert(source != nullptr);

	std::string srcCallsign = bytesToString(source, YSF_CALLSIGN_LENGTH);

	if (network == NETWORK::FROM) {
		m_toMode = DATA_MODE::NONE;

		std::string dest = find(m_ysfDStarDGIds, dgId);
		LogDebug("YSF => D-Star, %s>%u -> %s>%s", srcCallsign.c_str(), dgId, srcCallsign.c_str(), dest.c_str());

		if (dest != NULL_CALLSIGN) {
			m_srcCallsign = srcCallsign;
			m_dstCallsign = dest;
			m_toMode      = DATA_MODE::DSTAR;

			writeJSONStatus(DATA_MODE::YSF, DATA_MODE::DSTAR, dgId, 0U, false, false, "", m_dstCallsign);
		}

		if (m_toMode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> dst = find(m_ysfDMRDGIds, dgId);
			if (dst.second != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId == NULL_ID32)
					return;

				LogDebug("YSF => DMR, %s>%u -> %u>%u:TG%u", srcCallsign.c_str(), dgId, srcId, dst.first, dst.second);

				m_slot   = dst.first;
				m_srcId  = srcId;
				m_dstId  = dst.second;
				m_group  = true;
				m_toMode = DATA_MODE::DMR;

				writeJSONStatus(DATA_MODE::YSF, DATA_MODE::DMR, dgId, m_dstId, false, true, "", "", 0U, m_slot);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint32_t dstId = find(m_ysfP25DGIds, dgId);
			if (dstId != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId == NULL_ID32)
					return;

				LogDebug("YSF => P25, %s>%u -> %u>TG%u", srcCallsign.c_str(), dgId, srcId, dstId);

				m_srcId  = srcId;
				m_dstId  = dstId;
				m_group  = true;
				m_toMode = DATA_MODE::P25;

				writeJSONStatus(DATA_MODE::YSF, DATA_MODE::P25, dgId, m_dstId, false, true);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint16_t dstId = find(m_ysfNXDNDGIds, dgId);
			if (dstId != NULL_ID16) {
				uint16_t srcId = m_nxdnLookup.lookup(srcCallsign);
				if (srcId == NULL_ID16)
					return;

				LogDebug("YSF => NXDN, %s>%u -> %u>TG%u", srcCallsign.c_str(), dgId, srcId, dstId);

				m_srcId  = srcId;
				m_dstId  = dstId;
				m_group  = true;
				m_toMode = DATA_MODE::NXDN;

				writeJSONStatus(DATA_MODE::YSF, DATA_MODE::NXDN, dgId, m_dstId, false, true);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if (dgId == m_ysfFMDGId) {
				LogDebug("YSF => FM, %s>%u ->", srcCallsign.c_str(), dgId);

				m_toMode = DATA_MODE::FM;

				writeJSONStatus(DATA_MODE::YSF, DATA_MODE::FM, dgId);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if (m_toYSF) {
				LogDebug("YSF => YSF, %s>%u -> %s>%u", srcCallsign.c_str(), dgId, srcCallsign.c_str(), dgId);

				m_srcCallsign = srcCallsign;
				m_dgId        = dgId;
				m_toMode      = DATA_MODE::YSF;

				writeJSONStatus(DATA_MODE::YSF, DATA_MODE::YSF, dgId, dgId);
			}
		}
	} else {
		if (m_fromMode == DATA_MODE::DSTAR) {
			std::string dest = find(m_dstarYSFDests, dgId);
			if (dgId != NULL_DGID) {
				LogDebug("YSF <= D-Star, %s>%u -> %s>%s", srcCallsign.c_str(), dgId, srcCallsign.c_str(), dest.c_str());

				m_srcCallsign = srcCallsign;
				m_dstCallsign = dest;
				m_toMode      = DATA_MODE::YSF;
			}
		}

		if (m_fromMode == DATA_MODE::DMR) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_dmrYSFTGs, dgId);
			if (slotTG.second != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId != NULL_ID32) {
					LogDebug("YSF <= DMR, %s>%u -> %u>%u:TG%u", srcCallsign.c_str(), dgId, srcId, slotTG.first, slotTG.second);

					m_slot   = slotTG.first;
					m_srcId  = srcId;
					m_dstId  = slotTG.second;
					m_group  = true;
					m_toMode = DATA_MODE::YSF;
				}
			}
		}

		if (m_fromMode == DATA_MODE::P25) {
			uint32_t tg = find(m_p25YSFTGs, dgId);
			if (tg != NULL_ID32) {
				uint32_t id = m_dmrLookup.lookup(srcCallsign);
				if (id != NULL_ID32) {
					LogDebug("YSF <= P25, %s>%u -> %u>TG%u", srcCallsign.c_str(), dgId, id, tg);

					m_srcId  = id;
					m_dstId  = tg;
					m_group  = true;
					m_toMode = DATA_MODE::YSF;
				}
			}
		}

		if (m_fromMode == DATA_MODE::NXDN) {
			uint16_t tg = find(m_nxdnYSFTGs, dgId);
			if (tg != NULL_ID16) {
				uint16_t id = m_nxdnLookup.lookup(srcCallsign);
				if (id != NULL_ID16) {
					LogDebug("YSF <= NXDN, %s>%u -> %u>TG%u", srcCallsign.c_str(), dgId, id, tg);

					m_srcId  = id;
					m_dstId  = tg;
					m_group  = true;
					m_toMode = DATA_MODE::YSF;
				}
			}
		}

		if (m_fromMode == DATA_MODE::FM) {
			if (dgId == m_ysfFMDGId) {
				LogDebug("YSF <= FM, %s>%u ->", srcCallsign.c_str(), dgId);

				m_toMode = DATA_MODE::YSF;
			}
		}

		else if (m_fromMode == DATA_MODE::YSF) {
			if (m_toYSF) {
				LogDebug("YSF <= YSF, %s>%u -> %s>%u", srcCallsign.c_str(), dgId, srcCallsign.c_str(), dgId);

				m_srcCallsign = srcCallsign;
				m_dgId        = dgId;
				m_toMode      = DATA_MODE::YSF;
			}
		}

		else
			m_toMode = DATA_MODE::NONE;
	}
}

void CMetaData::setP25(NETWORK network, uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	if (network == NETWORK::FROM) {
		m_toMode = DATA_MODE::NONE;

		std::string dst = find(m_p25DStarTGs, destination);
		if (dst != NULL_CALLSIGN) {
			std::string src = m_dmrLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("P25 => D-Star, %u>TG%u -> %s>%s", source, destination, src.c_str(), dst.c_str());

				m_srcCallsign = src;
				m_dstCallsign = dst;
				m_toMode      = DATA_MODE::DSTAR;

				writeJSONStatus(DATA_MODE::P25, DATA_MODE::DSTAR, destination, 0U, true, false, "", m_dstCallsign);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_p25DMRTGs, destination);
			if (slotTG.second != NULL_ID32) {
				LogDebug("P25 => DMR, %u>TG%u -> %u>%u:TG%u", source, destination, source, slotTG.first, slotTG.second);

				m_slot   = slotTG.first;
				m_srcId  = source;
				m_dstId  = slotTG.second;
				m_group  = true;
				m_toMode = DATA_MODE::DMR;

				writeJSONStatus(DATA_MODE::P25, DATA_MODE::DMR, destination, m_dstId, true, true, "", "", 0U, m_slot);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_p25YSFTGs, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("P25 => YSF, %u>TG%u -> %s>%u", source, destination, src.c_str(), dgId);

					m_srcCallsign = src;
					m_dgId        = dgId;
					m_toMode      = DATA_MODE::YSF;

					writeJSONStatus(DATA_MODE::P25, DATA_MODE::YSF, destination, m_dgId, true, false);
				}
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint16_t tg = find(m_p25NXDNTGs, destination);
			if (tg != NULL_ID16) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint16_t id = m_nxdnLookup.lookup(src);
					if (id != NULL_ID16) {
						LogDebug("P25 => NXDN, %u>TG%u -> %u>TG%u", source, destination, id, tg);

						m_srcId  = id;
						m_dstId  = tg;
						m_group  = true;
						m_toMode = DATA_MODE::NXDN;

						writeJSONStatus(DATA_MODE::P25, DATA_MODE::NXDN, destination, m_dstId, true, true);
					}
				}
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if (destination == m_p25FMTG) {
				LogDebug("P25 => FM, %u>TG%u ->", source, destination);

				m_toMode = DATA_MODE::FM;

				writeJSONStatus(DATA_MODE::P25, DATA_MODE::FM, destination, 0U, true, true);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if (m_toP25) {
				LogDebug("P25 => P25, %u>TG%u -> %u>TG%u", source, destination, source, destination);

				m_srcId  = source;
				m_dstId  = destination;
				m_group  = group;
				m_toMode = DATA_MODE::P25;

				writeJSONStatus(DATA_MODE::P25, DATA_MODE::P25, destination, destination, true, true);
			}
		}
	} else {
		if (m_fromMode == DATA_MODE::DSTAR) {
			std::string dest = find(m_dstarP25Dests, destination);
			if (dest != NULL_CALLSIGN) {
				std::string src = m_dmrLookup.lookup(source);
				if (src == NULL_CALLSIGN)
					return;

				LogDebug("P25 <= D-Star, %u>TG%u -> %s>%s", source, destination, src.c_str(), dest.c_str());

				m_srcCallsign = src;
				m_dstCallsign = dest;
				m_toMode      = DATA_MODE::P25;
			}
		}

		else if (m_fromMode == DATA_MODE::DMR) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_dmrP25TGs, destination);
			if (slotTG.second != NULL_ID32) {
				LogDebug("P25 <= DMR, %u>TG%u -> %u>%u:TG%u", source, destination, source, slotTG.first, slotTG.second);

				m_slot   = slotTG.first;
				m_srcId  = source;
				m_dstId  = slotTG.second;
				m_group  = true;
				m_toMode = DATA_MODE::P25;
			}
		}

		else if (m_fromMode == DATA_MODE::YSF) {
			uint8_t dgId = find(m_ysfP25DGIds, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src == NULL_CALLSIGN)
					return;

				LogDebug("P25 <= YSF, %u>TG%u -> %s>%u", source, destination, src.c_str(), dgId);

				m_srcCallsign = src;
				m_dgId        = dgId;
				m_toMode      = DATA_MODE::P25;
			}
		}

		else if (m_fromMode == DATA_MODE::NXDN) {
			uint16_t tg = find(m_nxdnP25TGs, destination);
			if (tg != NULL_ID16) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint16_t id = m_nxdnLookup.lookup(src);
					if (id != NULL_ID16) {
						LogDebug("P25 <= NXDN, %u>TG%u -> %u>TG%u", source, destination, id, tg);

						m_srcId  = id;
						m_dstId  = tg;
						m_group  = true;
						m_toMode = DATA_MODE::P25;
					}
				}
			}
		}

		if (m_fromMode == DATA_MODE::FM) {
			if (destination == m_p25FMTG) {
				LogDebug("P25 <= FM, -> %u>TG%u", source, destination);

				m_toMode = DATA_MODE::P25;
			}
		}

		else if (m_fromMode == DATA_MODE::P25) {
			if (m_toP25) {
				LogDebug("P25 <= P25, %u>TG%u -> %u>TG%u", source, destination, source, destination);

				m_srcId  = source;
				m_dstId  = destination;
				m_group  = group;
				m_toMode = DATA_MODE::P25;
			}
		}

		else
			m_toMode = DATA_MODE::NONE;
	}
}

void CMetaData::setNXDN(NETWORK network, uint16_t source, uint16_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	if (network == NETWORK::FROM) {
		m_toMode = DATA_MODE::NONE;

		std::string dst = find(m_nxdnDStarTGs, destination);
		if (dst != NULL_CALLSIGN) {
			std::string src = m_dmrLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("NXDN => D-Star, %u>TG%u -> %s>%s", source, destination, src.c_str(), dst.c_str());

				m_srcCallsign = src;
				m_dstCallsign = dst;
				m_toMode      = DATA_MODE::DSTAR;

				writeJSONStatus(DATA_MODE::NXDN, DATA_MODE::DSTAR, destination, 0U, true, false, "", m_dstCallsign);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_nxdnDMRTGs, destination);
			if (slotTG.second != NULL_ID32) {
				LogDebug("NXDN => DMR, %u>TG%u -> %u>%u:TG%u", source, destination, source, slotTG.first, slotTG.second);

				m_slot   = slotTG.first;
				m_srcId  = source;
				m_dstId  = slotTG.second;
				m_group  = true;
				m_toMode = DATA_MODE::DMR;

				writeJSONStatus(DATA_MODE::NXDN, DATA_MODE::DMR, destination, m_dstId, true, true, "", "", 0U, m_slot);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_nxdnYSFTGs, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("NXDN => YSF, %u>TG%u -> %s>%u", source, destination, src.c_str(), dgId);

					m_srcCallsign = src;
					m_dgId        = dgId;
					m_toMode      = DATA_MODE::YSF;

					writeJSONStatus(DATA_MODE::NXDN, DATA_MODE::YSF, destination, m_dgId, true, false);
				}
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			uint32_t tg = find(m_nxdnP25TGs, destination);
			if (tg != NULL_ID32) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint32_t id = m_dmrLookup.lookup(src);
					if (id != NULL_ID32) {
						LogDebug("NXDN => P25, %u>TG%u -> %u>TG%u", source, destination, id, tg);

						m_srcId  = id;
						m_dstId  = tg;
						m_group  = true;
						m_toMode = DATA_MODE::P25;

						writeJSONStatus(DATA_MODE::NXDN, DATA_MODE::P25, destination, m_dstId, true, true);
					}
				}
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if (destination == m_nxdnFMTG) {
				LogDebug("NXDN => FM, %u>TG%u ->", source, destination);

				m_toMode = DATA_MODE::FM;

				writeJSONStatus(DATA_MODE::NXDN, DATA_MODE::FM, destination, 0U, true, true);
			}
		}

		if (m_toMode == DATA_MODE::NONE) {
			if (m_toNXDN) {
				LogDebug("NXDN => NXDN, %u>TG%u -> %u>TG%u", source, destination, source, destination);

				m_srcId  = source;
				m_dstId  = destination;
				m_group  = group;
				m_toMode = DATA_MODE::NXDN;

				writeJSONStatus(DATA_MODE::NXDN, DATA_MODE::NXDN, destination, destination, true, true);
			}
		}
	} else {
		if (m_fromMode == DATA_MODE::DSTAR) {
			std::string dest = find(m_dstarNXDNDests, destination);
			if (dest != NULL_CALLSIGN) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src == NULL_CALLSIGN)
					return;

				LogDebug("NXDN <= D-Star, %u>TG%u -> %s>%s", source, destination, src.c_str(), dest.c_str());

				m_srcCallsign = src;
				m_dstCallsign = dest;
				m_toMode      = DATA_MODE::NXDN;
			}
		}

		else if (m_fromMode == DATA_MODE::DMR) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_dmrNXDNTGs, destination);
			if (slotTG.second != NULL_ID32) {
				LogDebug("NXDN <= DMR, %u>TG%u -> %u>%u:TG%u", source, destination, source, slotTG.first, slotTG.second);

				m_slot   = slotTG.first;
				m_srcId  = source;
				m_dstId  = slotTG.second;
				m_group  = true;
				m_toMode = DATA_MODE::NXDN;
			}
		}

		else if (m_fromMode == DATA_MODE::YSF) {
			uint8_t dgId = find(m_ysfNXDNDGIds, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src == NULL_CALLSIGN)
					return;

				LogDebug("NXDN <= YSF, %u>TG%u -> %s>%u", source, destination, src.c_str(), dgId);

				m_srcCallsign = src;
				m_dgId        = dgId;
				m_toMode      = DATA_MODE::NXDN;
			}
		}

		else if (m_fromMode == DATA_MODE::P25) {
			uint32_t tg = find(m_p25NXDNTGs, destination);
			if (tg != NULL_ID32) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint32_t id = m_dmrLookup.lookup(src);
					if (id != NULL_ID32) {
						LogDebug("NXDN <= P25, %u>TG%u -> %u>TG%u", source, destination, id, tg);

						m_srcId  = id;
						m_dstId  = tg;
						m_group  = true;
						m_toMode = DATA_MODE::NXDN;
					}
				}
			}
		}

		if (m_fromMode == DATA_MODE::FM) {
			if (destination == m_nxdnFMTG) {
				LogDebug("NXDN <= FM, -> %u>TG%u", source, destination);

				m_toMode = DATA_MODE::NXDN;
			}
		}

		else if (m_fromMode == DATA_MODE::NXDN) {
			if (m_toNXDN) {
				LogDebug("NXDN <= NXDN, %u>TG%u -> %u>TG%u", source, destination, source, destination);

				m_srcId  = source;
				m_dstId  = destination;
				m_group  = group;
				m_toMode = DATA_MODE::NXDN;
			}
		}

		else
			m_toMode = DATA_MODE::NONE;
	}
}

// XXX FIXME this'll need rework for the new FM network protocol
void CMetaData::setFM(NETWORK network)
{
	if (network == NETWORK::FROM) {
		if (m_toMode == DATA_MODE::NONE) {
			if (m_toFM) {
				LogDebug("FM => FM, %s ->", m_defaultCallsign.c_str());

				m_toMode = DATA_MODE::FM;

				writeJSONStatus(DATA_MODE::FM, DATA_MODE::FM);
			}
		}
	} else {
		if (m_fromMode == DATA_MODE::DSTAR) {
			if (m_dstarFMDest != NULL_CALLSIGN) {
				LogDebug("D-Star <= FM, %s>%s ->", m_defaultCallsign.c_str(), m_dstarFMDest.c_str());

				m_srcCallsign = m_defaultCallsign;
				m_dstCallsign = m_dstarFMDest;
				m_toMode      = DATA_MODE::FM;
			}
		}

		if (m_fromMode == DATA_MODE::DMR) {
			if (m_dmrFMTG.second != NULL_ID32) {
				LogDebug("DMR <= FM, %u>%u:%u ->", m_defaultDMRId, m_dmrFMTG.first, m_dmrFMTG.second);

				m_srcId  = m_defaultDMRId;
				m_dstId  = m_dmrFMTG.second;
				m_slot   = m_dmrFMTG.first;
				m_group  = true;
				m_toMode = DATA_MODE::FM;
			}
		}

		else if (m_fromMode == DATA_MODE::YSF) {
			if (m_ysfFMDGId != NULL_DGID) {
				LogDebug("YSF <= FM, %s>%u ->", m_defaultCallsign.c_str(), m_ysfFMDGId);

				m_srcCallsign = m_defaultCallsign;
				m_dgId        = m_ysfFMDGId;
				m_toMode      = DATA_MODE::FM;
			}
		}

		else if (m_fromMode == DATA_MODE::P25) {
			if (m_p25FMTG != NULL_ID32) {
				LogDebug("P25 <= FM, %s>%u ->", m_defaultCallsign.c_str(), m_p25FMTG);

				m_srcId  = m_defaultDMRId;
				m_dstId  = m_p25FMTG;
				m_toMode = DATA_MODE::FM;
			}
		}

		else if (m_fromMode == DATA_MODE::NXDN) {
			if (m_nxdnFMTG != NULL_ID16) {
				LogDebug("NXDN <= FM, %s>%u ->", m_defaultCallsign.c_str(), m_nxdnFMTG);

				m_srcId  = m_defaultNXDNId;
				m_dstId  = m_nxdnFMTG;
				m_toMode = DATA_MODE::FM;
			}
		}

		else if (m_fromMode == DATA_MODE::FM) {
			if (m_toFM) {
				LogDebug("FM <= FM, %s ->", m_defaultCallsign.c_str());

				m_toMode = DATA_MODE::FM;
			}
		}
	}
}

void CMetaData::setRaw(const uint8_t* data, uint16_t length)
{
	assert(data != nullptr);
	assert(length > 0U);

	::memcpy(m_rawData, data, length);
	m_rawLength = length;
}

bool CMetaData::setData(const uint8_t* data)
{
	assert(data != nullptr);

	bool ret = m_transcoder.write(data);
	if (!ret)
		return false;

	m_count++;

	return true;
}

void CMetaData::setEnd()
{
	m_end = true;
}

void CMetaData::getDStar(NETWORK network, uint8_t* source, uint8_t* destination) const
{
	assert(source != nullptr);
	assert(destination != nullptr);

	stringToBytes(source,      DSTAR_LONG_CALLSIGN_LENGTH, m_srcCallsign);
	stringToBytes(destination, DSTAR_LONG_CALLSIGN_LENGTH, m_dstCallsign);
}

void CMetaData::getDMR(NETWORK network, uint8_t& slot, uint32_t& source, uint32_t& destination, bool& group) const
{
	slot        = m_slot;
	source      = m_srcId;
	destination = m_dstId;
	group       = m_group;
}

void CMetaData::getYSF(NETWORK network, uint8_t* source, uint8_t* destination, uint8_t& dgId) const
{
	assert(source != nullptr);

	stringToBytes(source,      YSF_CALLSIGN_LENGTH, m_srcCallsign);
	stringToBytes(destination, YSF_CALLSIGN_LENGTH, m_dstCallsign);

	dgId = m_dgId;
}

void CMetaData::getP25(NETWORK network, uint32_t& source, uint32_t& destination, bool& group) const
{
	source      = m_srcId;
	destination = m_dstId;
	group       = m_group;
}

void CMetaData::getNXDN(NETWORK network, uint16_t& source, uint16_t& destination, bool& group) const
{
	source      = m_srcId;
	destination = m_dstId;
	group       = m_group;
}

void CMetaData::setFM(const uint8_t* source)
{
	assert(source != nullptr);

	m_srcCallsign = bytesToString(source, ::strlen((char*)source));
}

void CMetaData::getFM(uint8_t* source) const
{
	assert(source != nullptr);

	uint16_t length = uint16_t(m_srcCallsign.size());

	stringToBytes(source, length, m_srcCallsign);
	source[length] = 0x00U;
}

bool CMetaData::hasRaw() const
{
	return m_rawLength > 0U;
}

bool CMetaData::hasData() const
{
	return m_length > 0U;
}

uint16_t CMetaData::getRaw(uint8_t* data)
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

bool CMetaData::getData(uint8_t* data)
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

bool CMetaData::isEnd() const
{
	if (m_count > 0U)
		return false;

	return m_end;
}

bool CMetaData::isTranscode() const
{
	return m_fromMode != m_toMode;
}

void CMetaData::reset()
{
	m_end       = false;
	m_length    = 0U;
	m_count     = 0U;
	m_rawLength = 0U;
	m_toMode    = DATA_MODE::NONE;
	m_direction = DIRECTION::NONE;

	writeJSONStatus(DATA_MODE::NONE, DATA_MODE::NONE);
}

void CMetaData::clock(unsigned int ms)
{
	m_length = m_transcoder.read(m_data);

	m_dmrLookup.clock(ms);
	m_nxdnLookup.clock(ms);
}

void CMetaData::close()
{
	m_transcoder.close();
}

// uint8_t <=> std::string
uint8_t CMetaData::find(const std::vector<std::pair<std::string, uint8_t>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (it.first == dest)
			return it.second;
	}

	return NULL_DGID;
}

std::string CMetaData::find(const std::vector<std::pair<std::string, uint8_t>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (it.second == dgId)
			return it.first;
	}

	return NULL_CALLSIGN;
}

uint8_t CMetaData::find(const std::vector<std::pair<uint8_t, std::string>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (it.second == dest)
			return it.first;
	}

	return NULL_DGID;
}

std::string CMetaData::find(const std::vector<std::pair<uint8_t, std::string>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (it.first == dgId)
			return it.second;
	}

	return NULL_CALLSIGN;
}

// uint16_t <=> std::string
uint16_t CMetaData::find(const std::vector<std::pair<std::string, uint16_t>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (it.first == dest)
			return it.second;
	}

	return NULL_ID16;
}

std::string CMetaData::find(const std::vector<std::pair<std::string, uint16_t>>& mapping, uint16_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.second == tgId)
			return it.first;
	}

	return NULL_CALLSIGN;
}

uint16_t CMetaData::find(const std::vector<std::pair<uint16_t, std::string>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (it.second == dest)
			return it.first;
	}

	return NULL_ID16;
}

std::string CMetaData::find(const std::vector<std::pair<uint16_t, std::string>>& mapping, uint16_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.first == tgId)
			return it.second;
	}

	return NULL_CALLSIGN;
}

// uint32_t <=> std::string
uint32_t CMetaData::find(const std::vector<std::pair<std::string, uint32_t>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (it.first == dest)
			return it.second;
	}

	return NULL_ID32;
}

std::string CMetaData::find(const std::vector<std::pair<std::string, uint32_t>>& mapping, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.second == tgId)
			return it.first;
	}

	return NULL_CALLSIGN;
}

uint32_t CMetaData::find(const std::vector<std::pair<uint32_t, std::string>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (it.second == dest)
			return it.first;
	}

	return NULL_ID32;
}

std::string CMetaData::find(const std::vector<std::pair<uint32_t, std::string>>& mapping, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.first == tgId)
			return it.second;
	}

	return NULL_CALLSIGN;
}

// <uint8_t, uint32_t> <=> std::string
std::pair<uint8_t, uint32_t> CMetaData::find(const std::vector<std::tuple<std::string, uint8_t, uint32_t>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (std::get<0>(it) == dest)
			return std::make_pair(std::get<1>(it), std::get<2>(it));
	}

	return std::make_pair(NULL_SLOT, NULL_ID32);
}

std::string CMetaData::find(const std::vector<std::tuple<std::string, uint8_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if ((std::get<1>(it) == slot) && (std::get<2>(it) == tgId))
			return std::get<0>(it);
	}

	return NULL_CALLSIGN;
}

std::string CMetaData::find(const std::vector<std::tuple<uint8_t, uint32_t, std::string>>& mapping, uint8_t slot, uint32_t tgid) const
{
	for (const auto& it : mapping) {
		if ((std::get<0>(it) == slot) && (std::get<1>(it) == tgid))
			return std::get<2>(it);
	}

	return NULL_CALLSIGN;
}

std::pair<uint8_t, uint32_t> CMetaData::find(const std::vector<std::tuple<uint8_t, uint32_t, std::string>>& mapping, const std::string& dest) const
{
	for (const auto& it : mapping) {
		if (std::get<2>(it) == dest)
			return std::make_pair(std::get<0>(it), std::get<1>(it));
	}

	return std::make_pair(NULL_SLOT, NULL_ID32);
}

// <uint8_t, uint32_t> <=> uint16_t
std::pair<uint8_t, uint32_t> CMetaData::find(const std::vector<std::tuple<uint16_t, uint8_t, uint32_t>>& mapping, uint16_t tgId) const
{
	for (const auto& it : mapping) {
		if (std::get<0>(it) == tgId)
			return std::make_pair(std::get<1>(it), std::get<2>(it));
	}

	return std::make_pair(NULL_SLOT, NULL_ID32);
}

uint16_t CMetaData::find(const std::vector<std::tuple<uint16_t, uint8_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if ((std::get<1>(it) == slot) && (std::get<2>(it) == tgId))
			return std::get<0>(it);
	}

	return NULL_ID16;
}

uint16_t CMetaData::find(const std::vector<std::tuple<uint8_t, uint32_t, uint16_t>>& mapping, uint8_t slot, uint32_t tgid) const
{
	for (const auto& it : mapping) {
		if ((std::get<0>(it) == slot) && (std::get<1>(it) == tgid))
			return std::get<2>(it);
	}

	return NULL_ID16;
}

std::pair<uint8_t, uint32_t> CMetaData::find(const std::vector<std::tuple<uint8_t, uint32_t, uint16_t>>& mapping, uint16_t tgId) const
{
	for (const auto& it : mapping) {
		if (std::get<2>(it) == tgId)
			return std::make_pair(std::get<0>(it), std::get<1>(it));
	}

	return std::make_pair(NULL_SLOT, NULL_ID32);
}

// <uint8_t, uint32_t> <=> uint32_t
std::pair<uint8_t, uint32_t> CMetaData::find(const std::vector<std::tuple<uint32_t, uint8_t, uint32_t>>& mapping, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if (std::get<0>(it) == tgId)
			return std::make_pair(std::get<1>(it), std::get<2>(it));
	}

	return std::make_pair(NULL_SLOT, NULL_ID32);
}

uint32_t CMetaData::find(const std::vector<std::tuple<uint32_t, uint8_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if ((std::get<1>(it) == slot) && (std::get<2>(it) == tgId))
			return std::get<0>(it);
	}

	return NULL_ID32;
}

uint32_t CMetaData::find(const std::vector<std::tuple<uint8_t, uint32_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgid) const
{
	for (const auto& it : mapping) {
		if ((std::get<0>(it) == slot) && (std::get<1>(it) == tgid))
			return std::get<2>(it);
	}

	return NULL_ID32;
}

std::pair<uint8_t, uint32_t> CMetaData::find(const std::vector<std::tuple<uint8_t, uint32_t, uint32_t>>& mapping, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if (std::get<2>(it) == tgId)
			return std::make_pair(std::get<0>(it), std::get<1>(it));
	}

	return std::make_pair(NULL_SLOT, NULL_ID32);
}

// uint8_t <=> uint16_t
uint8_t CMetaData::find(const std::vector<std::pair<uint8_t, uint16_t>>& mapping, uint16_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.second == tgId)
			return it.first;
	}

	return NULL_DGID;
}

uint16_t CMetaData::find(const std::vector<std::pair<uint8_t, uint16_t>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (it.first == dgId)
			return it.second;
	}

	return NULL_ID16;
}

uint8_t CMetaData::find(const std::vector<std::pair<uint16_t, uint8_t>>& mapping, uint16_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.first == tgId)
			return it.second;
	}

	return NULL_DGID;
}

uint16_t CMetaData::find(const std::vector<std::pair<uint16_t, uint8_t>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (it.second == dgId)
			return it.first;
	}

	return NULL_ID16;
}

// uint32_t <=> uint16_t
uint32_t CMetaData::find(const std::vector<std::pair<uint32_t, uint16_t>>& mapping, uint16_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.second == tgId)
			return it.first;
	}

	return NULL_ID32;
}

uint16_t CMetaData::find(const std::vector<std::pair<uint32_t, uint16_t>>& mapping, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.first == tgId)
			return it.second;
	}

	return NULL_ID16;
}

uint32_t CMetaData::find(const std::vector<std::pair<uint16_t, uint32_t>>& mapping, uint16_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.first == tgId)
			return it.second;
	}

	return NULL_ID32;
}

uint16_t CMetaData::find(const std::vector<std::pair<uint16_t, uint32_t>>& mapping, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.second == tgId)
			return it.first;
	}

	return NULL_ID16;
}

// uint32_t <=> uint8_t
uint32_t CMetaData::find(const std::vector<std::pair<uint32_t, uint8_t>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (it.second == dgId)
			return it.first;
	}

	return NULL_ID32;
}

uint8_t CMetaData::find(const std::vector<std::pair<uint32_t, uint8_t>>& mapping, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.first == tgId)
			return it.second;
	}

	return NULL_DGID;
}

uint32_t CMetaData::find(const std::vector<std::pair<uint8_t, uint32_t>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (it.first == dgId)
			return it.second;
	}

	return NULL_ID32;
}

uint8_t CMetaData::find(const std::vector<std::pair<uint8_t, uint32_t>>& mapping, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if (it.second == tgId)
			return it.first;
	}

	return NULL_DGID;
}

// <uint8_t, uint32_t> <=> uint32_t
uint8_t CMetaData::find(const std::vector<std::tuple<uint8_t, uint8_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const
{
	for (const auto& it : mapping) {
		if ((std::get<1>(it) == slot) && (std::get<2>(it) == tgId))
			return std::get<0>(it);
	}

	return NULL_DGID;
}

std::pair<uint8_t, uint32_t> CMetaData::find(const std::vector<std::tuple<uint8_t, uint8_t, uint32_t>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (std::get<0>(it) == dgId)
			return std::make_pair(std::get<1>(it), std::get<2>(it));
	}

	return std::make_pair(NULL_SLOT, NULL_ID32);
}

// <uint8_t, uint32_t> <=> uint8_t
uint8_t CMetaData::find(const std::vector<std::tuple<uint8_t, uint32_t, uint8_t>>& mapping, uint8_t slot, uint32_t tgid) const
{
	for (const auto& it : mapping) {
		if ((std::get<0>(it) == slot) && (std::get<1>(it) == tgid))
			return std::get<2>(it);
	}

	return NULL_DGID;
}

std::pair<uint8_t, uint32_t> CMetaData::find(const std::vector<std::tuple<uint8_t, uint32_t, uint8_t>>& mapping, uint8_t dgId) const
{
	for (const auto& it : mapping) {
		if (std::get<2>(it) == dgId)
			return std::make_pair(std::get<0>(it), std::get<1>(it));
	}

	return std::make_pair(NULL_SLOT, NULL_ID32);
}

std::string CMetaData::bytesToString(const uint8_t* str, size_t length) const
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

void CMetaData::stringToBytes(uint8_t* str, size_t length, const std::string& callsign) const
{
	assert(str != nullptr);

	::memset(str, ' ', length);

	size_t len = callsign.length();
	if (len > length)
		len = length;

	for (size_t i = 0U; i < len; i++)
		str[i] = callsign[i];
}

void CMetaData::writeJSONStatus(DATA_MODE fromMode, DATA_MODE toMode, uint32_t fromId, uint32_t toId, bool fromGroup, bool toGroup, const std::string& fromCS, const std::string& toCS, uint8_t fromSlot, uint8_t toSlot)
{
	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();

	switch (fromMode) {
	case DATA_MODE::DSTAR:
		json["from_mode"]             = "D-Star";
		json["from_dstar_callsign"]   = fromCS;
		break;
	case DATA_MODE::DMR:
		json["from_mode"]             = "DMR";
		json["from_dmr_slot"]         = fromSlot;
		json["from_dmr_id"]           = fromId;
		json["from_destination_type"] = fromGroup ? "group" : "individual";
		break;
	case DATA_MODE::YSF:
		json["from_mode"]             = "YSF";
		json["from_dg-id"]            = fromId;
		break;
	case DATA_MODE::P25:
		json["from_mode"]             = "P25";
		json["from_p25_id"]           = fromId;
		json["from_destination_type"] = fromGroup ? "group" : "individual";
		break;
	case DATA_MODE::NXDN:
		json["from_mode"]             = "NXDN";
		json["from_nxdn_id"]          = fromId;
		json["from_destination_type"] = fromGroup ? "group" : "individual";
		break;
	case DATA_MODE::FM:
		json["from_mode"]             = "FM";
		break;
	default:
		json["from_mode"]             = "none";
		break;
	}

	switch (toMode) {
	case DATA_MODE::DSTAR:
		json["to_mode"]             = "D-Star";
		json["to_dstar_callsign"]   = toCS;
		break;
	case DATA_MODE::DMR:
		json["to_mode"]             = "DMR";
		json["to_dmr_slot"]         = toSlot;
		json["to_dmr_id"]           = toId;
		json["to_destination_type"] = toGroup ? "group" : "individual";
		break;
	case DATA_MODE::YSF:
		json["to_mode"]             = "YSF";
		json["to_dg-id"]            = toId;
		break;
	case DATA_MODE::P25:
		json["to_mode"]             = "P25";
		json["to_p25_id"]           = toId;
		json["to_destination_type"] = toGroup ? "group" : "individual";
		break;
	case DATA_MODE::NXDN:
		json["to_mode"]             = "NXDN";
		json["to_nxdn_id"]          = toId;
		json["to_destination_type"] = toGroup ? "group" : "individual";
		break;
	case DATA_MODE::FM:
		json["to_mode"]             = "FM";
		break;
	default:
		json["to_mode"]             = "none";
		break;
	}

	WriteJSON("Status", json);
}
