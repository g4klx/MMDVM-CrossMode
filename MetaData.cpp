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
m_direction(DIRECTION::NONE),
m_rf(),
m_net(),
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

bool CMetaData::setDirection(DIRECTION direction)
{
	m_direction = direction;

	return setTranscoder();
}

bool CMetaData::setTranscoder()
{
	uint8_t transRFMode;
	uint8_t transNetMode;

	switch (m_rf.m_mode) {
	case DATA_MODE::DSTAR:
		transRFMode = MODE_DSTAR;
		break;
	case DATA_MODE::DMR:
	case DATA_MODE::NXDN:
		transRFMode = MODE_DMR_NXDN;
		break;
	case DATA_MODE::YSF:
		transRFMode = MODE_YSFDN;
		break;
	case DATA_MODE::P25:
		transRFMode = MODE_IMBE;
		break;
	case DATA_MODE::FM:
		transRFMode = MODE_PCM;
		break;
	default:
		return false;
	}

	switch (m_net.m_mode) {
	case DATA_MODE::DSTAR:
		transNetMode = MODE_DSTAR;
		break;
	case DATA_MODE::DMR:
	case DATA_MODE::NXDN:
		transNetMode = MODE_DMR_NXDN;
		break;
	case DATA_MODE::YSF:
		transNetMode = MODE_YSFDN;
		break;
	case DATA_MODE::P25:
		transNetMode = MODE_IMBE;
		break;
	case DATA_MODE::FM:
		transNetMode = MODE_PCM;
		break;
	default:
		return false;
	}

	switch (m_direction) {
	case DIRECTION::RF_TO_NET:
		return m_transcoder.setConversion(transRFMode, transNetMode);
	case DIRECTION::NET_TO_RF:
		return m_transcoder.setConversion(transNetMode, transRFMode);
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

DATA_MODE CMetaData::getRFMode() const
{
	return m_rf.m_mode;
}

DATA_MODE CMetaData::getNetMode() const
{
	return m_net.m_mode;
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
	return m_transcoder.open();
}

void CMetaData::setDStar(NETWORK network, const uint8_t* source, const uint8_t* destination)
{
	assert(source != nullptr);
	assert(destination != nullptr);

	std::string srcCallsign = bytesToString(source, DSTAR_LONG_CALLSIGN_LENGTH);
	std::string dstCallsign = bytesToString(destination, DSTAR_LONG_CALLSIGN_LENGTH);

	if (network == NETWORK::RF) {
		m_net.reset();

		m_rf.m_mode            = DATA_MODE::DSTAR;
		m_rf.DStar.srcCallsign = srcCallsign;
		m_rf.DStar.dstCallsign = dstCallsign;

		m_direction = DIRECTION::RF_TO_NET;

		std::pair<uint8_t, uint32_t> dst = find(m_dstarDMRDests, dstCallsign);
		if (dst.second != NULL_ID32) {
			uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
			if (srcId != NULL_ID32) {
				LogDebug("D-Star => DMR, %s>%s -> %u>%u:TG%u", srcCallsign.c_str(), dstCallsign.c_str(), srcId, dst.first, dst.second);

				m_net.m_mode    = DATA_MODE::DMR;
				m_net.DMR.group = true;
				m_net.DMR.slot  = dst.first;
				m_net.DMR.dstId = dst.second;
				m_net.DMR.srcId = srcId;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_dstarYSFDests, dstCallsign);
			if (dgId != NULL_DGID) {
				LogDebug("D-Star => YSF, %s>%s -> %s>%u", srcCallsign.c_str(), dstCallsign.c_str(), srcCallsign.c_str(), dgId);

				m_net.m_mode       = DATA_MODE::YSF;
				m_net.YSF.dgId     = dgId;
				m_net.YSF.callsign = srcCallsign;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint32_t dstId = find(m_dstarP25Dests, dstCallsign);
			if (dstId != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId != NULL_ID32) {
					LogDebug("D-Star => P25, %s>%s -> %u>TG%u", srcCallsign.c_str(), dstCallsign.c_str(), srcId, dstId);

					m_net.m_mode    = DATA_MODE::P25;
					m_net.P25.dstId = dstId;
					m_net.P25.srcId = srcId;
					m_net.P25.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint16_t dstId = find(m_dstarNXDNDests, dstCallsign);
			if (dstId != NULL_ID16) {
				uint16_t srcId = m_nxdnLookup.lookup(srcCallsign);
				if (srcId != NULL_ID16) {
					LogDebug("D-Star => NXDN, %s>%s -> %u>TG%u", srcCallsign.c_str(), dstCallsign.c_str(), srcId, dstId);

					m_net.m_mode     = DATA_MODE::NXDN;
					m_net.NXDN.dstId = dstId;
					m_net.NXDN.srcId = srcId;
					m_net.NXDN.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if (dstCallsign == m_dstarFMDest) {
				LogDebug("D-Star => FM, %s>%s -> %s", srcCallsign.c_str(), dstCallsign.c_str(), srcCallsign.c_str());

				m_net.m_mode      = DATA_MODE::FM;
				m_net.FM.callsign = srcCallsign;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if (m_toDStar) {
				LogDebug("D-Star => D-Star, %s>%s -> %s>%s", srcCallsign.c_str(), dstCallsign.c_str(), srcCallsign.c_str(), dstCallsign.c_str());

				m_net.m_mode            = DATA_MODE::DSTAR;
				m_net.DStar.srcCallsign = srcCallsign;
				m_net.DStar.dstCallsign = dstCallsign;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			LogDebug("D-Star => , %s>%s ->", srcCallsign.c_str(), dstCallsign.c_str());

			writeJSONStatus();

			m_rf.reset();
		}
	} else {
		m_rf.reset();

		m_net.m_mode            = DATA_MODE::DSTAR;
		m_net.DStar.srcCallsign = srcCallsign;
		m_net.DStar.dstCallsign = dstCallsign;

		m_direction = DIRECTION::NET_TO_RF;

		std::pair<uint8_t, uint32_t> slotTG = find(m_dmrDStarTGs, dstCallsign);
		if (slotTG.second != NULL_ID32) {
			uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
			if (srcId != NULL_ID32) {
				LogDebug("DMR <= D-Star, %u>%u:TG%u <- %s>%s", srcId, slotTG.first, slotTG.second, srcCallsign.c_str(), dstCallsign.c_str());

				m_rf.m_mode    = DATA_MODE::DMR;
				m_rf.DMR.slot  = slotTG.first;
				m_rf.DMR.srcId = srcId;
				m_rf.DMR.dstId = slotTG.second;
				m_rf.DMR.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_ysfDStarDGIds, dstCallsign);
			if (dgId != NULL_DGID) {
				LogDebug("YSF <= D-Star, %s>%u <- %s>%s", srcCallsign.c_str(), dgId, srcCallsign.c_str(), dstCallsign.c_str());

				m_rf.m_mode       = DATA_MODE::YSF;
				m_rf.YSF.callsign = srcCallsign;
				m_rf.YSF.dgId     = dgId;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint32_t tg = find(m_p25DStarTGs, dstCallsign);
			if (tg != NULL_ID32) {
				uint32_t id = m_dmrLookup.lookup(srcCallsign);
				if (id != NULL_ID32) {
					LogDebug("P25 <= D-Star, %u>TG%u <- %s>%s", id, tg, srcCallsign.c_str(), dstCallsign.c_str());

					m_rf.m_mode    = DATA_MODE::P25;
					m_rf.P25.srcId = id;
					m_rf.P25.dstId = tg;
					m_rf.P25.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint16_t tg = find(m_nxdnDStarTGs, dstCallsign);
			if (tg != NULL_ID16) {
				uint16_t id = m_nxdnLookup.lookup(srcCallsign);
				if (id != NULL_ID16) {
					LogDebug("NXDN <= D-Star, %u>TG%u <- %s>%s", id, tg, srcCallsign.c_str(), dstCallsign.c_str());

					m_rf.m_mode     = DATA_MODE::NXDN;
					m_rf.NXDN.srcId = id;
					m_rf.NXDN.dstId = tg;
					m_rf.NXDN.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (dstCallsign == m_dstarFMDest) {
				LogDebug("FM <= D-Star, %s <- %s>%s", srcCallsign.c_str(), srcCallsign.c_str(), dstCallsign.c_str());

				m_rf.m_mode      = DATA_MODE::FM;
				m_rf.FM.callsign = srcCallsign;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_toDStar) {
				LogDebug("D-Star <= D-Star, %s>%s <- %s>%s", srcCallsign.c_str(), dstCallsign.c_str(), srcCallsign.c_str(), dstCallsign.c_str());

				m_rf.m_mode            = DATA_MODE::DSTAR;
				m_rf.DStar.srcCallsign = srcCallsign;
				m_rf.DStar.dstCallsign = dstCallsign;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			LogDebug(" <= D-Star, <- %s>%s", srcCallsign.c_str(), dstCallsign.c_str());

			writeJSONStatus();

			m_net.reset();
		}
	}
}

void CMetaData::setDMR(NETWORK network, uint8_t slot, uint32_t source, uint32_t destination, bool group)
{
	assert((slot == 1U) || (slot == 2U));
	assert(source > 0U);
	assert(destination > 0U);

	if (network == NETWORK::RF) {
		m_net.reset();

		m_rf.m_mode    = DATA_MODE::DMR;
		m_rf.DMR.slot  = slot;
		m_rf.DMR.srcId = source;
		m_rf.DMR.dstId = destination;
		m_rf.DMR.group = true;

		m_direction = DIRECTION::RF_TO_NET;

		std::string dst = find(m_dmrDStarTGs, slot, destination);
		if (dst != NULL_CALLSIGN) {
			std::string src = m_dmrLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("DMR => D-Star, %u>%u:TG%u -> %s>%s", source, slot, destination, src.c_str(), dst.c_str());

				m_net.m_mode            = DATA_MODE::DSTAR;
				m_net.DStar.srcCallsign = src;
				m_net.DStar.dstCallsign = dst;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_dmrYSFTGs, slot, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("DMR => YSF, %u>%u:TG%u -> %s>%u", source, slot, destination, src.c_str(), dgId);

					m_net.m_mode       = DATA_MODE::YSF;
					m_net.YSF.callsign = src;
					m_net.YSF.dgId     = dgId;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint32_t tg = find(m_dmrP25TGs, slot, destination);
			if (tg != NULL_ID32) {
				LogDebug("DMR => P25, %u>%u:TG%u -> %u>TG%u", source, slot, destination, source, tg);

				m_rf.m_mode  = DATA_MODE::P25;
				m_net.P25.srcId = source;
				m_net.P25.dstId = tg;
				m_net.P25.group = true;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint16_t tg = find(m_dmrNXDNTGs, slot, destination);
			if (tg != NULL_ID16) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint16_t id = m_nxdnLookup.lookup(src);
					if (id != NULL_ID16) {
						LogDebug("DMR => NXDN, %u>%u:TG%u -> %u>TG%u", source, slot, destination, id, tg);

						m_net.m_mode     = DATA_MODE::NXDN;
						m_net.NXDN.srcId = id;
						m_net.NXDN.dstId = tg;
						m_net.NXDN.group = true;

						writeJSONStatus();
					}
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> tg = std::make_pair(slot, destination);
			if (tg == m_dmrFMTG) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("DMR => FM, %u>%u:TG%u -> %s", source, slot, destination, src.c_str());

					m_net.m_mode      = DATA_MODE::FM;
					m_net.FM.callsign = src;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if ((slot == 1U) && m_toDMR1) {
				LogDebug("DMR => DMR, %u>%u:TG%u -> %u>%u:TG%u", source, slot, destination, source, slot, destination);

				m_net.m_mode    = DATA_MODE::DMR;
				m_net.DMR.slot  = slot;
				m_net.DMR.srcId = source;
				m_net.DMR.dstId = destination;
				m_net.DMR.group = true;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if ((slot == 2U) && m_toDMR2) {
				LogDebug("DMR => DMR, %u>%u:TG%u -> %u>%u:TG%u", source, slot, destination, source, slot, destination);

				m_net.m_mode    = DATA_MODE::DMR;
				m_net.DMR.slot  = slot;
				m_net.DMR.srcId = source;
				m_net.DMR.dstId = destination;
				m_net.DMR.group = true;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			LogDebug("DMR => , %u>%u:TG%u -> ", source, slot, destination);

			writeJSONStatus();
			
			m_rf.reset();
		}
	} else {
		m_rf.reset();

		m_net.m_mode    = DATA_MODE::DMR;
		m_net.DMR.slot  = slot;
		m_net.DMR.srcId = source;
		m_net.DMR.dstId = destination;
		m_net.DMR.group = true;

		m_direction = DIRECTION::NET_TO_RF;

		std::string dest = find(m_dstarDMRDests, slot, destination);
		if (dest != NULL_CALLSIGN) {
			std::string src = m_dmrLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("D-Star <= DMR, %s>%s <- %u>%u:TG%u", src.c_str(), dest.c_str(), source, slot, destination);

				m_rf.m_mode            = DATA_MODE::DSTAR;
				m_rf.DStar.srcCallsign = src;
				m_rf.DStar.dstCallsign = dest;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_ysfDMRDGIds, slot, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("YSF <= DMR, %s>%u <- %u>%u:TG%u", src.c_str(), dgId, source, slot, destination);

					m_rf.m_mode       = DATA_MODE::YSF;
					m_rf.YSF.callsign = src;
					m_rf.YSF.dgId     = dgId;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint32_t tg = find(m_p25DMRTGs, slot, destination);
			if (tg != NULL_ID32) {
				LogDebug("P25 <= DMR, %u>TG%u <- %u>%u:TG%u", source, tg, source, slot, destination);

				m_rf.m_mode    = DATA_MODE::P25;
				m_rf.P25.srcId = source;
				m_rf.P25.dstId = tg;
				m_rf.P25.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint16_t tg = find(m_nxdnDMRTGs, slot, destination);
			if (tg != NULL_ID16) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint16_t id = m_nxdnLookup.lookup(src);
					if (id != NULL_ID16) {
						LogDebug("NXDN <= DMR, %u>TG%u <- %u>%u:TG%u", id, tg, source, slot, destination);

						m_rf.m_mode     = DATA_MODE::NXDN;
						m_rf.NXDN.srcId = id;
						m_rf.NXDN.dstId = tg;
						m_rf.NXDN.group = true;

						writeJSONStatus();
					}
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> tg = std::make_pair(slot, destination);
			if (tg == m_dmrFMTG) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("FM <= DMR, %s <- %u>%u:TG%u", src.c_str(), source, slot, destination);

					m_rf.m_mode      = DATA_MODE::FM;
					m_rf.FM.callsign = src;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if ((slot == 1U) && m_toDMR1) {
				LogDebug("DMR <= DMR, %u>%u:TG%u <- %u>%u:TG%u", source, slot, destination, source, slot, destination);

				m_rf.m_mode    = DATA_MODE::DMR;
				m_rf.DMR.slot  = slot;
				m_rf.DMR.srcId = source;
				m_rf.DMR.dstId = destination;
				m_rf.DMR.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if ((slot == 2U) && m_toDMR2) {
				LogDebug("DMR <= DMR, %u>%u:TG%u <- %u>%u:TG%u", source, slot, destination, source, slot, destination);

				m_rf.m_mode    = DATA_MODE::DMR;
				m_rf.DMR.slot  = slot;
				m_rf.DMR.srcId = source;
				m_rf.DMR.dstId = destination;
				m_rf.DMR.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			LogDebug(" <= DMR, <- %u>%u:TG%u", source, slot, destination);

			writeJSONStatus();

			m_net.reset();
		}
	}
}

void CMetaData::setYSF(NETWORK network, const uint8_t* source, uint8_t dgId)
{
	assert(source != nullptr);

	std::string srcCallsign = bytesToString(source, YSF_CALLSIGN_LENGTH);

	if (network == NETWORK::RF) {
		m_net.reset();

		m_rf.m_mode       = DATA_MODE::YSF;
		m_rf.YSF.callsign = srcCallsign;
		m_rf.YSF.dgId     = dgId;

		m_direction = DIRECTION::RF_TO_NET;

		std::string dest = find(m_ysfDStarDGIds, dgId);
		if (dest != NULL_CALLSIGN) {
			LogDebug("YSF => D-Star, %s>%u -> %s>%s", srcCallsign.c_str(), dgId, srcCallsign.c_str(), dest.c_str());

			m_net.m_mode            = DATA_MODE::DSTAR;
			m_net.DStar.srcCallsign = srcCallsign;
			m_net.DStar.dstCallsign = dest;

			writeJSONStatus();
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> dst = find(m_ysfDMRDGIds, dgId);
			if (dst.second != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId != NULL_ID32) {
					LogDebug("YSF => DMR, %s>%u -> %u>%u:TG%u", srcCallsign.c_str(), dgId, srcId, dst.first, dst.second);

					m_net.m_mode    = DATA_MODE::DMR;
					m_net.DMR.slot  = dst.first;
					m_net.DMR.srcId = srcId;
					m_net.DMR.dstId = dst.second;
					m_net.DMR.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint32_t dstId = find(m_ysfP25DGIds, dgId);
			if (dstId != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId != NULL_ID32) {
					LogDebug("YSF => P25, %s>%u -> %u>TG%u", srcCallsign.c_str(), dgId, srcId, dstId);

					m_net.m_mode    = DATA_MODE::P25;
					m_net.P25.srcId = srcId;
					m_net.P25.dstId = dstId;
					m_net.P25.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint16_t dstId = find(m_ysfNXDNDGIds, dgId);
			if (dstId != NULL_ID16) {
				uint16_t srcId = m_nxdnLookup.lookup(srcCallsign);
				if (srcId != NULL_ID16) {
					LogDebug("YSF => NXDN, %s>%u -> %u>TG%u", srcCallsign.c_str(), dgId, srcId, dstId);

					m_net.m_mode     = DATA_MODE::NXDN;
					m_net.NXDN.srcId = srcId;
					m_net.NXDN.dstId = dstId;
					m_net.NXDN.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if (dgId == m_ysfFMDGId) {
				LogDebug("YSF => FM, %s>%u ->", srcCallsign.c_str(), dgId);

				m_net.m_mode      = DATA_MODE::FM;
				m_net.FM.callsign = srcCallsign;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if (m_toYSF) {
				LogDebug("YSF => YSF, %s>%u -> %s>%u", srcCallsign.c_str(), dgId, srcCallsign.c_str(), dgId);

				m_net.m_mode       = DATA_MODE::YSF;
				m_net.YSF.callsign = srcCallsign;
				m_net.YSF.dgId     = dgId;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			LogDebug("YSF => , %s>%u -> ", srcCallsign.c_str(), dgId);

			writeJSONStatus();

			m_rf.reset();
		}
	} else {
		m_rf.reset();

		m_net.m_mode       = DATA_MODE::YSF;
		m_net.YSF.callsign = srcCallsign;
		m_net.YSF.dgId     = dgId;

		m_direction = DIRECTION::NET_TO_RF;

		std::string dest = find(m_dstarYSFDests, dgId);
		if (dest != NULL_CALLSIGN) {
			LogDebug("D-Star <= YSF, %s>%s <- %s>%u", srcCallsign.c_str(), dest.c_str(), srcCallsign.c_str(), dgId);

			m_rf.m_mode            = DATA_MODE::DSTAR;
			m_rf.DStar.srcCallsign = srcCallsign;
			m_rf.DStar.dstCallsign = dest;

			writeJSONStatus();
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_dmrYSFTGs, dgId);
			if (slotTG.second != NULL_ID32) {
				uint32_t srcId = m_dmrLookup.lookup(srcCallsign);
				if (srcId != NULL_ID32) {
					LogDebug("DMR <= YSF, %u>%u:TG%u <- %s>%u", srcId, slotTG.first, slotTG.second, srcCallsign.c_str(), dgId);

					m_rf.m_mode    = DATA_MODE::DMR;
					m_rf.DMR.slot  = slotTG.first;
					m_rf.DMR.srcId = srcId;
					m_rf.DMR.dstId = slotTG.second;
					m_rf.DMR.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint32_t tg = find(m_p25YSFTGs, dgId);
			if (tg != NULL_ID32) {
				uint32_t id = m_dmrLookup.lookup(srcCallsign);
				if (id != NULL_ID32) {
					LogDebug("P25 <= YSF, %u>TG%u <- %s>%u", id, tg, srcCallsign.c_str(), dgId);

					m_rf.m_mode    = DATA_MODE::P25;
					m_rf.P25.srcId = id;
					m_rf.P25.dstId = tg;
					m_rf.P25.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint16_t tg = find(m_nxdnYSFTGs, dgId);
			if (tg != NULL_ID16) {
				uint16_t id = m_nxdnLookup.lookup(srcCallsign);
				if (id != NULL_ID16) {
					LogDebug("NXDN <= YSF, %u>TG%u <- %s>%u", id, tg, srcCallsign.c_str(), dgId);

					m_rf.m_mode     = DATA_MODE::NXDN;
					m_rf.NXDN.srcId = id;
					m_rf.NXDN.dstId = tg;
					m_rf.NXDN.group = true;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (dgId == m_ysfFMDGId) {
				LogDebug("FM <= YSF, <- %s>%u", srcCallsign.c_str(), dgId);

				m_rf.m_mode      = DATA_MODE::FM;
				m_rf.FM.callsign = srcCallsign;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_toYSF) {
				LogDebug("YSF <= YSF, %s>%u <- %s>%u", srcCallsign.c_str(), dgId, srcCallsign.c_str(), dgId);

				m_rf.m_mode       = DATA_MODE::YSF;
				m_rf.YSF.callsign = srcCallsign;
				m_rf.YSF.dgId     = dgId;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			LogDebug(" <= YSF, <- %s>%u", srcCallsign.c_str(), dgId);

			writeJSONStatus();

			m_net.reset();
		}
	}
}

void CMetaData::setP25(NETWORK network, uint32_t source, uint32_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	if (network == NETWORK::RF) {
		m_net.reset();

		m_rf.m_mode    = DATA_MODE::P25;
		m_rf.P25.srcId = source;
		m_rf.P25.dstId = destination;
		m_rf.P25.group = true;

		m_direction = DIRECTION::RF_TO_NET;

		std::string dst = find(m_p25DStarTGs, destination);
		if (dst != NULL_CALLSIGN) {
			std::string src = m_dmrLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("P25 => D-Star, %u>TG%u -> %s>%s", source, destination, src.c_str(), dst.c_str());

				m_net.m_mode            = DATA_MODE::DSTAR;
				m_net.DStar.srcCallsign = src;
				m_net.DStar.dstCallsign = dst;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_p25DMRTGs, destination);
			if (slotTG.second != NULL_ID32) {
				LogDebug("P25 => DMR, %u>TG%u -> %u>%u:TG%u", source, destination, source, slotTG.first, slotTG.second);

				m_net.m_mode    = DATA_MODE::DMR;
				m_net.DMR.slot  = slotTG.first;
				m_net.DMR.srcId = source;
				m_net.DMR.dstId = slotTG.second;
				m_net.DMR.group = true;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_p25YSFTGs, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("P25 => YSF, %u>TG%u -> %s>%u", source, destination, src.c_str(), dgId);

					m_net.m_mode       = DATA_MODE::YSF;
					m_net.YSF.callsign = src;
					m_net.YSF.dgId     = dgId;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint16_t tg = find(m_p25NXDNTGs, destination);
			if (tg != NULL_ID16) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint16_t id = m_nxdnLookup.lookup(src);
					if (id != NULL_ID16) {
						LogDebug("P25 => NXDN, %u>TG%u -> %u>TG%u", source, destination, id, tg);

						m_net.m_mode     = DATA_MODE::NXDN;
						m_net.NXDN.srcId = id;
						m_net.NXDN.dstId = tg;
						m_net.NXDN.group = true;

						writeJSONStatus();
					}
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if (destination == m_p25FMTG) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("P25 => FM, %u>TG%u -> %s", source, destination, src.c_str());

					m_net.m_mode      = DATA_MODE::FM;
					m_net.FM.callsign = src;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if (m_toP25) {
				LogDebug("P25 => P25, %u>TG%u -> %u>TG%u", source, destination, source, destination);

				m_net.m_mode    = DATA_MODE::P25;
				m_net.P25.srcId = source;
				m_net.P25.dstId = destination;
				m_net.P25.group = true;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			LogDebug("P25 => , %u>TG%u -> ", source, destination);

			writeJSONStatus();

			m_rf.reset();
		}
	} else {
		m_rf.reset();

		m_net.m_mode    = DATA_MODE::P25;
		m_net.P25.srcId = source;
		m_net.P25.dstId = destination;
		m_net.P25.group = true;

		m_direction = DIRECTION::NET_TO_RF;

		std::string dest = find(m_dstarP25Dests, destination);
		if (dest != NULL_CALLSIGN) {
			std::string src = m_dmrLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("D-Star <= P25, %s>%s <- %u>TG%u", src.c_str(), dest.c_str(), source, destination);

				m_rf.m_mode            = DATA_MODE::DSTAR;
				m_rf.DStar.srcCallsign = src;
				m_rf.DStar.dstCallsign = dest;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_dmrP25TGs, destination);
			if (slotTG.second != NULL_ID32) {
				LogDebug("DMR <= P25, %u>%u:TG%u <- %u>TG%u", source, slotTG.first, slotTG.second, source, destination);

				m_rf.m_mode    = DATA_MODE::DMR;
				m_rf.DMR.slot  = slotTG.first;
				m_rf.DMR.srcId = source;
				m_rf.DMR.dstId = slotTG.second;
				m_rf.DMR.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_ysfP25DGIds, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("YSF <= P25, %s>%u <- %u>TG%u", src.c_str(), dgId, source, destination);

					m_rf.m_mode       = DATA_MODE::YSF;
					m_rf.YSF.callsign = src;
					m_rf.YSF.dgId     = dgId;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint16_t tg = find(m_nxdnP25TGs, destination);
			if (tg != NULL_ID16) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint16_t id = m_nxdnLookup.lookup(src);
					if (id != NULL_ID16) {
						LogDebug("NXDN <= P25, %u>TG%u <- %u>TG%u", id, tg, source, destination);

						m_rf.m_mode     = DATA_MODE::NXDN;
						m_rf.NXDN.srcId = id;
						m_rf.NXDN.dstId = tg;
						m_rf.NXDN.group = true;

						writeJSONStatus();
					}
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (destination == m_p25FMTG) {
				std::string src = m_dmrLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("FM <= P25, %s <- %u>TG%u", src.c_str(), source, destination);

					m_rf.m_mode      = DATA_MODE::FM;
					m_rf.FM.callsign = src;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_toP25) {
				LogDebug("P25 <= P25, %u>TG%u <- %u>TG%u", source, destination, source, destination);

				m_rf.m_mode    = DATA_MODE::P25;
				m_rf.P25.srcId = source;
				m_rf.P25.dstId = destination;
				m_rf.P25.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			LogDebug(" <= P25, <- %u>TG%u", source, destination);

			writeJSONStatus();

			m_net.reset();
		}
	}
}

void CMetaData::setNXDN(NETWORK network, uint16_t source, uint16_t destination, bool group)
{
	assert(source > 0U);
	assert(destination > 0U);

	if (network == NETWORK::RF) {
		m_net.reset();

		m_rf.m_mode     = DATA_MODE::NXDN;
		m_rf.NXDN.srcId = source;
		m_rf.NXDN.dstId = destination;
		m_rf.NXDN.group = true;

		m_direction = DIRECTION::RF_TO_NET;

		std::string dst = find(m_nxdnDStarTGs, destination);
		if (dst != NULL_CALLSIGN) {
			std::string src = m_dmrLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("NXDN => D-Star, %u>TG%u -> %s>%s", source, destination, src.c_str(), dst.c_str());

				m_net.m_mode            = DATA_MODE::DSTAR;
				m_net.DStar.srcCallsign = src;
				m_net.DStar.dstCallsign = dst;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_nxdnDMRTGs, destination);
			if (slotTG.second != NULL_ID32) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint32_t id = m_dmrLookup.lookup(src);
					if (id != NULL_ID32) {
						LogDebug("NXDN => DMR, %u>TG%u -> %u>%u:TG%u", source, destination, source, slotTG.first, slotTG.second);

						m_net.m_mode    = DATA_MODE::DMR;
						m_net.DMR.slot  = slotTG.first;
						m_net.DMR.srcId = source;
						m_net.DMR.dstId = slotTG.second;
						m_net.DMR.group = true;

						writeJSONStatus();

					}
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_nxdnYSFTGs, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("NXDN => YSF, %u>TG%u -> %s>%u", source, destination, src.c_str(), dgId);

					m_net.m_mode       = DATA_MODE::YSF;
					m_net.YSF.callsign = src;
					m_net.YSF.dgId     = dgId;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			uint32_t tg = find(m_nxdnP25TGs, destination);
			if (tg != NULL_ID32) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint32_t id = m_dmrLookup.lookup(src);
					if (id != NULL_ID32) {
						LogDebug("NXDN => P25, %u>TG%u -> %u>TG%u", source, destination, id, tg);

						m_net.m_mode    = DATA_MODE::P25;
						m_net.P25.srcId = id;
						m_net.P25.dstId = tg;
						m_net.P25.group = true;

						writeJSONStatus();
					}
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if (destination == m_nxdnFMTG) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("NXDN => FM, %u>TG%u -> %s", source, destination, src.c_str());

					m_net.m_mode      = DATA_MODE::FM;
					m_net.FM.callsign = src;

					writeJSONStatus();
				}
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			if (m_toNXDN) {
				LogDebug("NXDN => NXDN, %u>TG%u -> %u>TG%u", source, destination, source, destination);

				m_net.m_mode     = DATA_MODE::NXDN;
				m_net.NXDN.srcId = source;
				m_net.NXDN.dstId = destination;
				m_net.NXDN.group = true;

				writeJSONStatus();
			}
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			LogDebug("NXDN => , %u>TG%u -> ", source, destination);

			writeJSONStatus();

			m_rf.reset();
		}
	} else {
		m_rf.reset();

		m_net.m_mode     = DATA_MODE::NXDN;
		m_net.NXDN.srcId = source;
		m_net.NXDN.dstId = destination;
		m_net.NXDN.group = true;

		m_direction = DIRECTION::NET_TO_RF;

		std::string dest = find(m_dstarNXDNDests, destination);
		if (dest != NULL_CALLSIGN) {
			std::string src = m_nxdnLookup.lookup(source);
			if (src != NULL_CALLSIGN) {
				LogDebug("D-Star <= NXDN, %s>%s <- %u>TG%u", src.c_str(), dest.c_str(), source, destination);

				m_rf.m_mode            = DATA_MODE::DSTAR;
				m_rf.DStar.srcCallsign = src;
				m_rf.DStar.dstCallsign = dest;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			std::pair<uint8_t, uint32_t> slotTG = find(m_dmrNXDNTGs, destination);
			if (slotTG.second != NULL_ID32) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint32_t id = m_dmrLookup.lookup(src);
					if (id != NULL_ID32) {
						LogDebug("DMR <= NXDN, %u>%u:TG%u <- %u>TG%u", source, slotTG.first, slotTG.second, source, destination);

						m_rf.m_mode    = DATA_MODE::DMR;
						m_rf.DMR.slot  = slotTG.first;
						m_rf.DMR.srcId = source;
						m_rf.DMR.dstId = slotTG.second;
						m_rf.DMR.group = true;

						writeJSONStatus();
					}
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint8_t dgId = find(m_ysfNXDNDGIds, destination);
			if (dgId != NULL_DGID) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("YSF <= NXDN, %s>%u <- %u>TG%u", src.c_str(), dgId, source, destination);

					m_rf.m_mode       = DATA_MODE::YSF;
					m_rf.YSF.callsign = src;
					m_rf.YSF.dgId     = dgId;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			uint32_t tg = find(m_p25NXDNTGs, destination);
			if (tg != NULL_ID32) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					uint32_t id = m_dmrLookup.lookup(src);
					if (id != NULL_ID32) {
						LogDebug("P25 <= NXDN, %u>TG%u <- %u>TG%u", id, tg, source, destination);

						m_rf.m_mode    = DATA_MODE::P25;
						m_rf.P25.srcId = id;
						m_rf.P25.dstId = tg;
						m_rf.P25.group = true;

						writeJSONStatus();
					}
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (destination == m_nxdnFMTG) {
				std::string src = m_nxdnLookup.lookup(source);
				if (src != NULL_CALLSIGN) {
					LogDebug("FM <= NXDN, %s <- %u>TG%u", src.c_str(), source, destination);

					m_rf.m_mode      = DATA_MODE::FM;
					m_rf.FM.callsign = src;

					writeJSONStatus();
				}
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_toNXDN) {
				LogDebug("NXDN <= NXDN, %u>TG%u <- %u>TG%u", source, destination, source, destination);

				m_rf.m_mode     = DATA_MODE::NXDN;
				m_rf.NXDN.srcId = source;
				m_rf.NXDN.dstId = destination;
				m_rf.NXDN.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			LogDebug("NXDN <= , %u>TG%u <- ", source, destination);

			writeJSONStatus();

			m_net.reset();
		}
	}
}

void CMetaData::setFM(NETWORK network, const uint8_t* source)
{
	assert(source != nullptr);

	std::string src = bytesToString(source, ::strlen((char*)source));
	if (src.empty())
		src = m_defaultCallsign;

	if (network == NETWORK::RF) {
		m_net.reset();

		m_rf.m_mode      = DATA_MODE::FM;
		m_rf.FM.callsign = src;

		m_direction = DIRECTION::RF_TO_NET;

		if (m_toFM) {
			LogDebug("FM => FM, %s -> %s", src.c_str(), src.c_str());

			m_net.m_mode      = DATA_MODE::FM;
			m_net.FM.callsign = src;

			writeJSONStatus();
		}

		if (m_net.m_mode == DATA_MODE::NONE) {
			LogDebug("FM => , %s -> ", src.c_str());

			writeJSONStatus();

			m_rf.reset();
		}
	} else {
		m_rf.reset();

		m_net.m_mode      = DATA_MODE::FM;
		m_net.FM.callsign = src;

		m_direction = DIRECTION::NET_TO_RF;

		if (m_dstarFMDest != NULL_CALLSIGN) {
			LogDebug("D-Star <= FM, %s>%s <- %s", m_defaultCallsign.c_str(), m_dstarFMDest.c_str(), m_defaultCallsign.c_str());

			m_rf.m_mode            = DATA_MODE::DSTAR;
			m_rf.DStar.srcCallsign = src;
			m_rf.DStar.dstCallsign = m_dstarFMDest;

			writeJSONStatus();
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_dmrFMTG.second != NULL_ID32) {
				LogDebug("DMR <= FM, %u>%u:%u <- %s", m_defaultDMRId, m_dmrFMTG.first, m_dmrFMTG.second, src.c_str());

				m_rf.m_mode    = DATA_MODE::DMR;
				m_rf.DMR.srcId = m_defaultDMRId;
				m_rf.DMR.dstId = m_dmrFMTG.second;
				m_rf.DMR.slot  = m_dmrFMTG.first;
				m_rf.DMR.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_ysfFMDGId != NULL_DGID) {
				LogDebug("YSF <= FM, %s>%u <- %s", m_defaultCallsign.c_str(), m_ysfFMDGId, src.c_str());

				m_rf.m_mode       = DATA_MODE::YSF;
				m_rf.YSF.callsign = src;
				m_rf.YSF.dgId     = m_ysfFMDGId;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_p25FMTG != NULL_ID32) {
				LogDebug("P25 <= FM, %s>%u <- %s", m_defaultCallsign.c_str(), m_p25FMTG, src.c_str());

				m_rf.m_mode    = DATA_MODE::P25;
				m_rf.P25.srcId = m_defaultDMRId;
				m_rf.P25.dstId = m_p25FMTG;
				m_rf.P25.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_nxdnFMTG != NULL_ID16) {
				LogDebug("NXDN <= FM, %s>%u <- %s", m_defaultCallsign.c_str(), m_nxdnFMTG, src.c_str());

				m_rf.m_mode     = DATA_MODE::NXDN;
				m_rf.NXDN.srcId = m_defaultNXDNId;
				m_rf.NXDN.dstId = m_nxdnFMTG;
				m_rf.NXDN.group = true;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			if (m_toFM) {
				LogDebug("FM <= FM, %s <- %s", src.c_str(), src.c_str());

				m_rf.m_mode      = DATA_MODE::FM;
				m_rf.FM.callsign = src;

				writeJSONStatus();
			}
		}

		if (m_rf.m_mode == DATA_MODE::NONE) {
			LogDebug(" <= FM, <- %s", src.c_str());

			writeJSONStatus();

			m_net.reset();
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

	if ((m_rf.m_mode == DATA_MODE::NONE) || (m_net.m_mode == DATA_MODE::NONE))
		return false;

	bool ret = m_transcoder.write(data);
	if (!ret)
		return false;

	m_count++;

	return true;
}

void CMetaData::setEnd()
{
	if ((m_rf.m_mode != DATA_MODE::NONE) && (m_net.m_mode != DATA_MODE::NONE)) {
		writeJSONStatus("end");

		LogDebug("END");

		m_end = true;
	}
}

void CMetaData::setLost()
{
	if ((m_rf.m_mode != DATA_MODE::NONE) && (m_net.m_mode != DATA_MODE::NONE)) {
		writeJSONStatus("lost");

		LogDebug("LOST");
	}
}

void CMetaData::getDStar(NETWORK network, uint8_t* source, uint8_t* destination) const
{
	assert(source != nullptr);
	assert(destination != nullptr);

	switch (network) {
	case NETWORK::RF:
		assert(m_rf.m_mode == DATA_MODE::DSTAR);
		stringToBytes(source,      DSTAR_LONG_CALLSIGN_LENGTH, m_rf.DStar.srcCallsign);
		stringToBytes(destination, DSTAR_LONG_CALLSIGN_LENGTH, m_rf.DStar.dstCallsign);
		break;

	case NETWORK::NET:
		assert(m_net.m_mode == DATA_MODE::DSTAR);
		stringToBytes(source,      DSTAR_LONG_CALLSIGN_LENGTH, m_net.DStar.srcCallsign);
		stringToBytes(destination, DSTAR_LONG_CALLSIGN_LENGTH, m_net.DStar.dstCallsign);
		break;

	default:
		break;
	}
}

void CMetaData::getDMR(NETWORK network, uint8_t& slot, uint32_t& source, uint32_t& destination, bool& group) const
{
	switch (network) {
	case NETWORK::RF:
		assert(m_rf.m_mode == DATA_MODE::DMR);
		slot        = m_rf.DMR.slot;
		source      = m_rf.DMR.srcId;
		destination = m_rf.DMR.dstId;
		group       = m_rf.DMR.group;
		break;

	case NETWORK::NET:
		assert(m_net.m_mode == DATA_MODE::DMR);
		slot        = m_net.DMR.slot;
		source      = m_net.DMR.srcId;
		destination = m_net.DMR.dstId;
		group       = m_net.DMR.group;
		break;

	default:
		slot        = 0U;
		source      = 0U;
		destination = 0U;
		group       = false;
		break;
	}
}

void CMetaData::getYSF(NETWORK network, uint8_t* source, uint8_t& dgId) const
{
	assert(source != nullptr);

	switch (network) {
	case NETWORK::RF:
		assert(m_rf.m_mode == DATA_MODE::YSF);
		stringToBytes(source, YSF_CALLSIGN_LENGTH, m_rf.YSF.callsign);
		dgId = m_rf.YSF.dgId;
		break;

	case NETWORK::NET:
		assert(m_net.m_mode == DATA_MODE::YSF);
		stringToBytes(source, YSF_CALLSIGN_LENGTH, m_net.YSF.callsign);
		dgId = m_net.YSF.dgId;
		break;

	default:
		break;
	}
}

void CMetaData::getP25(NETWORK network, uint32_t& source, uint32_t& destination, bool& group) const
{
	switch (network) {
	case NETWORK::RF:
		assert(m_rf.m_mode == DATA_MODE::P25);
		source      = m_rf.P25.srcId;
		destination = m_rf.P25.dstId;
		group       = m_rf.P25.group;
		break;

	case NETWORK::NET:
		assert(m_net.m_mode == DATA_MODE::P25);
		source      = m_net.P25.srcId;
		destination = m_net.P25.dstId;
		group       = m_net.P25.group;
		break;

	default:
		source      = 0U;
		destination = 0U;
		group       = false;
		break;
	}
}

void CMetaData::getNXDN(NETWORK network, uint16_t& source, uint16_t& destination, bool& group) const
{
	switch (network) {
	case NETWORK::RF:
		assert(m_rf.m_mode == DATA_MODE::NXDN);
		source      = m_rf.NXDN.srcId;
		destination = m_rf.NXDN.dstId;
		group       = m_rf.NXDN.group;
		break;

	case NETWORK::NET:
		assert(m_net.m_mode == DATA_MODE::NXDN);
		source      = m_net.NXDN.srcId;
		destination = m_net.NXDN.dstId;
		group       = m_net.NXDN.group;
		break;

	default:
		source      = 0U;
		destination = 0U;
		group       = false;
		break;
	}
}

void CMetaData::getFM(NETWORK network, uint8_t* source) const
{
	assert(source != nullptr);

	switch (network) {
	case NETWORK::RF: {
			assert(m_rf.m_mode == DATA_MODE::FM);
			uint16_t length = uint16_t(m_rf.FM.callsign.size());
			stringToBytes(source, length, m_rf.FM.callsign);
			source[length] = 0x00U;
		}
		break;

	case NETWORK::NET: {
			assert(m_net.m_mode == DATA_MODE::FM);
			uint16_t length = uint16_t(m_net.FM.callsign.size());
			stringToBytes(source, length, m_net.FM.callsign);
			source[length] = 0x00U;
		}
		break;

	default:
		break;
	}
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
	return m_rf.m_mode != m_net.m_mode;
}

void CMetaData::reset()
{
	m_rf.reset();
	m_net.reset();

	m_end       = false;
	m_length    = 0U;
	m_count     = 0U;
	m_rawLength = 0U;

	m_direction = DIRECTION::NONE;
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

void CMetaData::writeJSONStatus(const std::string& action) const
{
	nlohmann::json json;

	try {
		json["timestamp"] = CUtils::createTimestamp();

		if (action.empty()) {
			json["action"] = "begin";

			switch (m_direction) {
			case DIRECTION::RF_TO_NET:
				if (m_rf.m_mode != DATA_MODE::NONE)
					json["src"] = "RF";
				if (m_net.m_mode != DATA_MODE::NONE)
					json["dst"] = "Net";
				break;

			case DIRECTION::NET_TO_RF:
				if (m_net.m_mode != DATA_MODE::NONE)
					json["src"] = "Net";
				if (m_rf.m_mode != DATA_MODE::NONE)
					json["dst"] = "RF";
				break;

			default:
				break;
			}

			if (m_rf.m_mode != DATA_MODE::NONE)
				json["RF"]  = createAddress(m_rf);

			if (m_net.m_mode != DATA_MODE::NONE)
				json["Net"] = createAddress(m_net);
		} else {
			json["action"] = action;
		}

		WriteJSON("Status", json);
	}
	catch (nlohmann::json::exception& ex) {
		LogError("Error creating JSON - %s", ex.what());
	}
}
	
nlohmann::json CMetaData::createAddress(const CDestination& destination) const
{
	nlohmann::json json;

	json["mode"] = CUtils::getModeName(destination.m_mode);

	switch (destination.m_mode) {
	case DATA_MODE::DSTAR:
		json["src_callsign"] = destination.DStar.srcCallsign;
		json["dst_callsign"] = destination.DStar.dstCallsign;
		break;
	case DATA_MODE::DMR:
		json["slot"]   = destination.DMR.slot;
		json["src_id"] = destination.DMR.srcId;
		json["dst_id"] = destination.DMR.dstId;
		json["group"]  = destination.DMR.group ? "yes" : "no";
		break;
	case DATA_MODE::YSF:
		json["src_callsign"] = destination.YSF.callsign;
		json["dg-id"]        = destination.YSF.dgId;
		break;
	case DATA_MODE::P25:
		json["src_id"] = destination.P25.srcId;
		json["dst_id"] = destination.P25.dstId;
		json["group"]  = destination.P25.group ? "yes" : "no";
		break;
	case DATA_MODE::NXDN:
		json["src_id"] = destination.NXDN.srcId;
		json["dst_id"] = destination.NXDN.dstId;
		json["group"]  = destination.NXDN.group ? "yes" : "no";
		break;
	case DATA_MODE::FM:
		json["src_callsign"] = destination.FM.callsign;
		break;
	default:
		break;
	}

	return json;
}
