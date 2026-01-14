/*
 *	 Copyright (C) 2015,2016,2017,2024,2026 by Jonathan Naylor G4KLX
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

#include "Conf.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>

#include <utility>

#define	TRACE_CONFIG

const int BUFFER_SIZE = 500;

const uint8_t  NULL_SLOT = 0U;
const uint16_t NULL_ID16 = 0xFFFFU;
const uint32_t NULL_ID32 = 0xFFFFFFFFU;

enum SECTION {
	SECTION_NONE,
	SECTION_GENERAL,
	SECTION_LOG,
	SECTION_TRANSCODER,
	SECTION_LOOKUP,
	SECTION_DSTAR,
	SECTION_DMR,
	SECTION_NXDN,
	SECTION_DSTAR_DSTAR,
	SECTION_DSTAR_DMR,
	SECTION_DSTAR_YSF,
	SECTION_DSTAR_P25,
	SECTION_DSTAR_NXDN,
	SECTION_DSTAR_FM,
	SECTION_DMR_DSTAR,
	SECTION_DMR_DMR,
	SECTION_DMR_YSF,
	SECTION_DMR_P25,
	SECTION_DMR_NXDN,
	SECTION_DMR_FM,
	SECTION_YSF_DSTAR,
	SECTION_YSF_DMR,
	SECTION_YSF_YSF,
	SECTION_YSF_P25,
	SECTION_YSF_NXDN,
	SECTION_YSF_FM,
	SECTION_P25_DSTAR,
	SECTION_P25_DMR,
	SECTION_P25_YSF,
	SECTION_P25_P25,
	SECTION_P25_NXDN,
	SECTION_P25_FM,
	SECTION_NXDN_DSTAR,
	SECTION_NXDN_DMR,
	SECTION_NXDN_YSF,
	SECTION_NXDN_P25,
	SECTION_NXDN_NXDN,
	SECTION_NXDN_FM,
	SECTION_FM_DSTAR,
	SECTION_FM_DMR,
	SECTION_FM_YSF,
	SECTION_FM_P25,
	SECTION_FM_NXDN,
	SECTION_FM_FM,
	SECTION_DSTAR_NETWORK_FROM,
	SECTION_DSTAR_NETWORK_TO,
	SECTION_DMR_NETWORK_FROM,
	SECTION_DMR_NETWORK_TO,
	SECTION_YSF_NETWORK_FROM,
	SECTION_YSF_NETWORK_TO,
	SECTION_P25_NETWORK_FROM,
	SECTION_P25_NETWORK_TO,
	SECTION_NXDN_NETWORK_FROM,
	SECTION_NXDN_NETWORK_TO,
	SECTION_FM_NETWORK_FROM,
	SECTION_FM_NETWORK_TO
};

CConf::CConf(const std::string& file) :
m_file(file),
m_callsign("G9BF"),
m_fromMode(),
m_rfModeHang(10U),
m_netModeHang(3U),
m_daemon(false),
m_logDisplayLevel(0U),
m_logFileLevel(0U),
m_logFilePath(),
m_logFileRoot(),
m_logFileRotate(true),
m_transcoderPort(),
m_transcoderSpeed(460800U),
m_transcoderDebug(false),
m_dmrLookupFile(),
m_nxdnLookupFile(),
m_reloadTime(24U),
m_dStarModule("C"),
m_dmrId(123456U),
m_nxdnId(1234U),
m_dstarDStarEnable(false),
m_dstarDMREnable(false),
m_dstarDMRDests(),
m_dstarYSFEnable(false),
m_dstarYSFDests(),
m_dstarP25Enable(false),
m_dstarP25Dests(),
m_dstarNXDNEnable(false),
m_dstarNXDNDests(),
m_dstarFMEnable(false),
m_dmrDStarEnable(false),
m_dmrDStarTGs(),
m_dmrDMREnable1(false),
m_dmrDMREnable2(false),
m_dmrYSFEnable(false),
m_dmrYSFTGs(),
m_dmrP25Enable(false),
m_dmrP25TGs(),
m_dmrNXDNEnable(false),
m_dmrNXDNTGs(),
m_dmrFMEnable(false),
m_dmrFMTG(),
m_ysfDStarEnable(false),
m_ysfDStarDGIds(),
m_ysfDMREnable(false),
m_ysfDMRDGIds(),
m_ysfYSFEnable(false),
m_ysfP25Enable(false),
m_ysfP25DGIds(),
m_ysfNXDNEnable(false),
m_ysfNXDNDGIds(),
m_ysfFMEnable(false),
m_ysfFMDGId(0U),
m_p25DStarEnable(false),
m_p25DStarTGs(),
m_p25DMREnable(false),
m_p25DMRTGs(),
m_p25YSFEnable(false),
m_p25YSFTGs(),
m_p25P25Enable(false),
m_p25NXDNEnable(false),
m_p25NXDNTGs(),
m_p25FMEnable(false),
m_p25FMTG(0U),
m_nxdnDStarEnable(false),
m_nxdnDStarTGs(),
m_nxdnDMREnable(false),
m_nxdnDMRTGs(),
m_nxdnYSFEnable(false),
m_nxdnYSFTGs(),
m_nxdnP25Enable(false),
m_nxdnP25TGs(),
m_nxdnNXDNEnable(false),
m_nxdnFMEnable(false),
m_nxdnFMTG(0U),
m_fmDStarEnable(false),
m_fmDMREnable(false),
m_fmYSFEnable(false),
m_fmP25Enable(false),
m_fmNXDNEnable(false),
m_fmFMEnable(false),
m_dStarFromRemoteAddress("127.0.0.1"),
m_dStarFromRemotePort(20011U),
m_dStarFromLocalAddress("127.0.0.1"),
m_dStarFromLocalPort(20010U),
m_dStarFromDebug(false),
m_dStarToRemoteAddress("127.0.0.1"),
m_dStarToRemotePort(20011U),
m_dStarToLocalAddress("127.0.0.1"),
m_dStarToLocalPort(20010U),
m_dStarToDebug(false),
m_dmrFromRemoteAddress("127.0.0.1"),
m_dmrFromRemotePort(62031U),
m_dmrFromLocalAddress("127.0.0.1"),
m_dmrFromLocalPort(62032U),
m_dmrFromDebug(false),
m_dmrToRemoteAddress("127.0.0.1"),
m_dmrToRemotePort(62032U),
m_dmrToLocalAddress("127.0.0.1"),
m_dmrToLocalPort(62031U),
m_dmrToDebug(false),
m_ysfFromRemoteAddress("127.0.0.1"),
m_ysfFromRemotePort(20011U),
m_ysfFromLocalAddress("127.0.0.1"),
m_ysfFromLocalPort(20010U),
m_ysfFromDebug(false),
m_ysfToRemoteAddress("127.0.0.1"),
m_ysfToRemotePort(20011U),
m_ysfToLocalAddress("127.0.0.1"),
m_ysfToLocalPort(20010U),
m_ysfToDebug(false),
m_p25FromRemoteAddress("127.0.0.1"),
m_p25FromRemotePort(32010U),
m_p25FromLocalAddress("127.0.0.1"),
m_p25FromLocalPort(42020U),
m_p25FromDebug(false),
m_p25ToRemoteAddress("127.0.0.1"),
m_p25ToRemotePort(42020U),
m_p25ToLocalAddress("127.0.0.1"),
m_p25ToLocalPort(32010U),
m_p25ToDebug(false),
m_nxdnFromRemoteAddress("127.0.0.1"),
m_nxdnFromRemotePort(42021U),
m_nxdnFromLocalAddress("127.0.0.1"),
m_nxdnFromLocalPort(42022U),
m_nxdnFromDebug(false),
m_nxdnToRemoteAddress("127.0.0.1"),
m_nxdnToRemotePort(42022U),
m_nxdnToLocalAddress("127.0.0.1"),
m_nxdnToLocalPort(42021U),
m_nxdnToDebug(false),
m_fmFromRemoteAddress("127.0.0.1"),
m_fmFromRemotePort(20011U),
m_fmFromLocalAddress("127.0.0.1"),
m_fmFromLocalPort(20010U),
m_fmFromDebug(false),
m_fmToRemoteAddress("127.0.0.1"),
m_fmToRemotePort(20011U),
m_fmToLocalAddress("127.0.0.1"),
m_fmToLocalPort(20010U),
m_fmToDebug(false)
{
}

CConf::~CConf()
{
}

bool CConf::read()
{
	FILE* fp = ::fopen(m_file.c_str(), "rt");
	if (fp == nullptr) {
		::fprintf(stderr, "Couldn't open the .ini file - %s\n", m_file.c_str());
		return false;
	}

	SECTION section = SECTION_NONE;

	char buffer[BUFFER_SIZE];
	while (::fgets(buffer, BUFFER_SIZE, fp) != nullptr) {
		if (buffer[0U] == '#')
			continue;

		if (buffer[0U] == '[') {
			if (::strncmp(buffer, "[General]", 9U) == 0)
				section = SECTION_GENERAL;
			else if (::strncmp(buffer, "[Log]", 5U) == 0)
				section = SECTION_LOG;
			else if (::strncmp(buffer, "[Transcoder]", 12U) == 0)
				section = SECTION_TRANSCODER;
			else if (::strncmp(buffer, "[Lookup]", 8U) == 0)
				section = SECTION_LOOKUP;
			else if (::strncmp(buffer, "[D-Star]", 8U) == 0)
				section = SECTION_DSTAR;
			else if (::strncmp(buffer, "[DMR]", 5U) == 0)
				section = SECTION_DMR;
			else if (::strncmp(buffer, "[NXDN]", 6U) == 0)
				section = SECTION_NXDN;
			else if (::strncmp(buffer, "[D-Star to D-Star]", 18U) == 0)
				section = SECTION_DSTAR_DSTAR;
			else if (::strncmp(buffer, "[D-Star to DMR]", 15U) == 0)
				section = SECTION_DSTAR_DMR;
			else if (::strncmp(buffer, "[D-Star to System Fusion]", 25U) == 0)
				section = SECTION_DSTAR_YSF;
			else if (::strncmp(buffer, "[D-Star to P25]", 15U) == 0)
				section = SECTION_DSTAR_P25;
			else if (::strncmp(buffer, "[D-Star to NXDN]", 16U) == 0)
				section = SECTION_DSTAR_NXDN;
			else if (::strncmp(buffer, "[D-Star to FM]", 14U) == 0)
				section = SECTION_DSTAR_FM;
			else if (::strncmp(buffer, "[DMR to D-Star]", 15U) == 0)
				section = SECTION_DMR_DSTAR;
			else if (::strncmp(buffer, "[DMR to DMR]", 12U) == 0)
				section = SECTION_DMR_DMR;
			else if (::strncmp(buffer, "[DMR to System Fusion]", 22U) == 0)
				section = SECTION_DMR_YSF;
			else if (::strncmp(buffer, "[DMR to P25]", 12U) == 0)
				section = SECTION_DMR_P25;
			else if (::strncmp(buffer, "[DMR to NXDN]", 13U) == 0)
				section = SECTION_DMR_NXDN;
			else if (::strncmp(buffer, "[DMR to FM]", 11U) == 0)
				section = SECTION_DMR_FM;
			else if (::strncmp(buffer, "[System Fusion to D-Star]", 25U) == 0)
				section = SECTION_YSF_DSTAR;
			else if (::strncmp(buffer, "[System Fusion to DMR]", 22U) == 0)
				section = SECTION_YSF_DMR;
			else if (::strncmp(buffer, "[System Fusion to System Fusion]", 32U) == 0)
				section = SECTION_YSF_YSF;
			else if (::strncmp(buffer, "[System Fusion to P25]", 22U) == 0)
				section = SECTION_YSF_P25;
			else if (::strncmp(buffer, "[System Fusion to NXDN]", 23U) == 0)
				section = SECTION_YSF_NXDN;
			else if (::strncmp(buffer, "[System Fusion to FM]", 21U) == 0)
				section = SECTION_YSF_FM;
			else if (::strncmp(buffer, "[P25 to D-Star]", 15U) == 0)
				section = SECTION_P25_DSTAR;
			else if (::strncmp(buffer, "[P25 to DMR]", 12U) == 0)
				section = SECTION_P25_DMR;
			else if (::strncmp(buffer, "[P25 to System Fusion]", 22U) == 0)
				section = SECTION_P25_YSF;
			else if (::strncmp(buffer, "[P25 to P25]", 12U) == 0)
				section = SECTION_P25_P25;
			else if (::strncmp(buffer, "[P25 to NXDN]", 13U) == 0)
				section = SECTION_P25_NXDN;
			else if (::strncmp(buffer, "[P25 to FM]", 11U) == 0)
				section = SECTION_P25_FM;
			else if (::strncmp(buffer, "[NXDN to D-Star]", 16U) == 0)
				section = SECTION_NXDN_DSTAR;
			else if (::strncmp(buffer, "[NXDN to DMR]", 13U) == 0)
				section = SECTION_NXDN_DMR;
			else if (::strncmp(buffer, "[NXDN to System Fusion]", 23U) == 0)
				section = SECTION_NXDN_YSF;
			else if (::strncmp(buffer, "[NXDN to P25]", 13U) == 0)
				section = SECTION_NXDN_P25;
			else if (::strncmp(buffer, "[NXDN to NXDN]", 14U) == 0)
				section = SECTION_NXDN_NXDN;
			else if (::strncmp(buffer, "[NXDN to FM]", 12U) == 0)
				section = SECTION_NXDN_FM;
			else if (::strncmp(buffer, "[FM to D-Star]", 14U) == 0)
				section = SECTION_FM_DSTAR;
			else if (::strncmp(buffer, "[FM to DMR]", 11U) == 0)
				section = SECTION_FM_DMR;
			else if (::strncmp(buffer, "[FM to System Fusion]", 21U) == 0)
				section = SECTION_FM_YSF;
			else if (::strncmp(buffer, "[FM to P25]", 11U) == 0)
				section = SECTION_FM_P25;
			else if (::strncmp(buffer, "[FM to NXDN]", 12U) == 0)
				section = SECTION_FM_NXDN;
			else if (::strncmp(buffer, "[FM to FM]", 10U) == 0)
				section = SECTION_FM_FM;
			else if (::strncmp(buffer, "[D-Star Network From]", 21U) == 0)
				section = SECTION_DSTAR_NETWORK_FROM;
			else if (::strncmp(buffer, "[D-Star Network To]", 19U) == 0)
				section = SECTION_DSTAR_NETWORK_TO;
			else if (::strncmp(buffer, "[DMR Network From]", 18U) == 0)
				section = SECTION_DMR_NETWORK_FROM;
			else if (::strncmp(buffer, "[DMR Network To]", 16U) == 0)
				section = SECTION_DMR_NETWORK_TO;
			else if (::strncmp(buffer, "[System Fusion Network From]", 28U) == 0)
				section = SECTION_YSF_NETWORK_FROM;
			else if (::strncmp(buffer, "[System Fusion Network To]", 26U) == 0)
				section = SECTION_YSF_NETWORK_TO;
			else if (::strncmp(buffer, "[P25 Network From]", 18U) == 0)
				section = SECTION_P25_NETWORK_FROM;
			else if (::strncmp(buffer, "[P25 Network To]", 16U) == 0)
				section = SECTION_P25_NETWORK_TO;
			else if (::strncmp(buffer, "[NXDN Network From]", 19U) == 0)
				section = SECTION_NXDN_NETWORK_FROM;
			else if (::strncmp(buffer, "[NXDN Network To]", 17U) == 0)
				section = SECTION_NXDN_NETWORK_TO;
			else if (::strncmp(buffer, "[FM Network From]", 17U) == 0)
				section = SECTION_FM_NETWORK_FROM;
			else if (::strncmp(buffer, "[FM Network To]", 15U) == 0)
				section = SECTION_FM_NETWORK_TO;
			else
				section = SECTION_NONE;

			continue;
		}

		char* key = ::strtok(buffer, " \t=\r\n");
		if (key == nullptr)
			continue;

		char* value = ::strtok(nullptr, "\r\n");
		if (value == nullptr)
			continue;

		// Remove quotes from the value
		size_t len = ::strlen(value);
		if (len > 1U && *value == '"' && value[len - 1U] == '"') {
			value[len - 1U] = '\0';
			value++;
		}

		if (section == SECTION_GENERAL) {
			if (::strcmp(key, "Callsign") == 0)
				m_callsign = value;
			else if (::strcmp(key, "FromMode") == 0)
				m_fromMode = value;
			else if (::strcmp(key, "RFModeHang") == 0)
				m_rfModeHang = (unsigned int)::atoi(value);
			else if (::strcmp(key, "NetModeHang") == 0)
				m_netModeHang = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Daemon") == 0)
				m_daemon = ::atoi(value) == 1;
		} else if (section == SECTION_LOG) {
			if (::strcmp(key, "FilePath") == 0)
				m_logFilePath = value;
			else if (::strcmp(key, "FileRoot") == 0)
				m_logFileRoot = value;
			else if (::strcmp(key, "FileLevel") == 0)
				m_logFileLevel = (uint32_t)::atoi(value);
			else if (::strcmp(key, "DisplayLevel") == 0)
				m_logDisplayLevel = (uint32_t)::atoi(value);
			else if (::strcmp(key, "FileRotate") == 0)
				m_logFileRotate = ::atoi(value) == 1;
		} else if (section == SECTION_TRANSCODER) {
			if (::strcmp(key, "Port") == 0)
				m_transcoderPort = value;
			else if (::strcmp(key, "Speed") == 0)
				m_transcoderSpeed = uint32_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_transcoderDebug = ::atoi(value) == 1;
		} else if (section == SECTION_LOOKUP) {
			if (::strcmp(key, "DMRLookup") == 0)
				m_dmrLookupFile = value;
			else if (::strcmp(key, "NXDNLookup") == 0)
				m_nxdnLookupFile = value;
			else if (::strcmp(key, "ReloadTime") == 0)
				m_reloadTime = (unsigned int)::atoi(value);
		} else if (section == SECTION_DSTAR) {
			if (::strcmp(key, "Module") == 0)
				m_dStarModule = value;
		} else if (section == SECTION_DMR) {
			if (::strcmp(key, "Id") == 0)
				m_dmrId = uint32_t(::atoi(value));
		} else if (section == SECTION_NXDN) {
			if (::strcmp(key, "Id") == 0)
				m_nxdnId = uint16_t(::atoi(value));
		} else if (section == SECTION_DSTAR_DSTAR) {
			if (::strcmp(key, "Enable") == 0)
				m_dstarDStarEnable = ::atoi(value) == 1;
		} else if (section == SECTION_DSTAR_DMR) {
			if (::strcmp(key, "Enable") == 0) {
				m_dstarDMREnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "Dest") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				std::string dest = getString(value);
				if (dest.empty())
					continue;

				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(p + 1U);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "D-Star => DMR, mapping \"%s\" to %u:TG%u\n", dest.c_str(), slotTG.first, slotTG.second);
#endif
				m_dstarDMRDests.push_back(std::tuple<std::string, uint8_t, uint32_t>(dest, slotTG.first, slotTG.second));
			}
		} else if (section == SECTION_DSTAR_YSF) {
			if (::strcmp(key, "Enable") == 0) {
				m_dstarYSFEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "Dest") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				std::string dest = getString(value);
				if (dest.empty())
					continue;

				uint8_t dgid = uint8_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "D-Star => YSF, mapping \"%s\" to %u\n", dest.c_str(), dgid);
#endif
				m_dstarYSFDests.push_back(std::pair<std::string, uint8_t>(dest, dgid));
			}
		} else if (section == SECTION_DSTAR_P25) {
			if (::strcmp(key, "Enable") == 0) {
				m_dstarP25Enable = ::atoi(value) == 1;
			} else if (::strcmp(key, "Dest") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				std::string dest = getString(value);
				if (dest.empty())
					continue;

				uint32_t tgid = uint32_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "D-Star => P25, mapping \"%s\" to TG%u\n", dest.c_str(), tgid);
#endif
				m_dstarP25Dests.push_back(std::pair<std::string, uint32_t>(dest, tgid));
			}
		} else if (section == SECTION_DSTAR_NXDN) {
			if (::strcmp(key, "Enable") == 0) {
				m_dstarNXDNEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "Dest") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				std::string dest = getString(value);
				if (dest.empty())
					continue;

				uint16_t tgid = uint16_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "D-Star => NXDN, mapping \"%s\" to TG%u\n", dest.c_str(), tgid);
#endif
				m_dstarNXDNDests.push_back(std::pair<std::string, uint16_t>(dest, tgid));
			}
		} else if (section == SECTION_DSTAR_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_dstarFMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "Dest") == 0) {
				std::string dest = getString(value);
				if (dest.empty())
					continue;
#if defined(TRACE_CONFIG)
				::fprintf(stdout, "D-Star => FM, mapping \"%s\"\n", dest.c_str());
#endif
				m_dstarFMDest = dest;
			}
		} else if (section == SECTION_DMR_DSTAR) {
			if (::strcmp(key, "Enable") == 0) {
				m_dmrDStarEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(value);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

				std::string dest = getString(p + 1U);
				if (dest.empty())
					continue;

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "DMR => D-Star, mapping %u:TG%u to \"%s\"\n", slotTG.first, slotTG.second, dest.c_str());
#endif
				m_dmrDStarTGs.push_back(std::tuple<uint8_t, uint32_t, std::string>(slotTG.first, slotTG.second, dest));
			}
		} else if (section == SECTION_DMR_DMR) {
			if (::strcmp(key, "Enable1") == 0)
				m_dmrDMREnable1 = ::atoi(value) == 1;
			else if (::strcmp(key, "Enable2") == 0)
				m_dmrDMREnable2 = ::atoi(value) == 1;
		} else if (section == SECTION_DMR_YSF) {
			if (::strcmp(key, "Enable") == 0) {
				m_dmrYSFEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(value);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

				uint8_t dgId = uint8_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "DMR => YSF, mapping %u:TG%u to %u\n", slotTG.first, slotTG.second, dgId);
#endif
				m_dmrYSFTGs.push_back(std::tuple<uint8_t, uint32_t, uint8_t>(slotTG.first, slotTG.second, dgId));
			}
		} else if (section == SECTION_DMR_P25) {
			if (::strcmp(key, "Enable") == 0) {
				m_dmrP25Enable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(value);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

				uint32_t tgId = uint32_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "DMR => P25, mapping %u:TG%u to TG%u\n", slotTG.first, slotTG.second, tgId);
#endif
				m_dmrP25TGs.push_back(std::tuple<uint8_t, uint32_t, uint32_t>(slotTG.first, slotTG.second, tgId));
			}
		} else if (section == SECTION_DMR_NXDN) {
			if (::strcmp(key, "Enable") == 0) {
				m_dmrNXDNEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(value);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

				uint16_t tgId = uint16_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "DMR => NXDN, mapping %u:TG%u to TG%u\n", slotTG.first, slotTG.second, tgId);
#endif
				m_dmrNXDNTGs.push_back(std::tuple<uint8_t, uint32_t, uint16_t>(slotTG.first, slotTG.second, tgId));
			}
		} else if (section == SECTION_DMR_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_dmrFMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(value);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "DMR => FM, mapping %u:TG%u\n", slotTG.first, slotTG.second);
#endif
				m_dmrFMTG = std::make_pair(slotTG.first, slotTG.second);
			}
		} else if (section == SECTION_YSF_DSTAR) {
			if (::strcmp(key, "Enable") == 0) {
				m_ysfDStarEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "DGId") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint8_t dgId = uint8_t(::atoi(value));

				std::string dest = getString(p + 1U);
				if (dest.empty())
					continue;

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "YSF => D-Star, mapping %u to \"%s\"\n", dgId, dest.c_str());
#endif
				m_ysfDStarDGIds.push_back(std::pair<uint8_t, std::string>(dgId, dest));
			}
		} else if (section == SECTION_YSF_DMR) {
			if (::strcmp(key, "Enable") == 0) {
				m_ysfDMREnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "DGId") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint8_t dgId = uint8_t(::atoi(value));

				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(p + 1U);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "YSF => DMR, mapping %u to %u:TG%u\n", dgId, slotTG.first, slotTG.second);
#endif
				m_ysfDMRDGIds.push_back(std::tuple<uint8_t, uint8_t, uint32_t>(dgId, slotTG.first, slotTG.second));
			}
		} else if (section == SECTION_YSF_YSF) {
			if (::strcmp(key, "Enable") == 0)
				m_ysfYSFEnable = ::atoi(value) == 1;
		} else if (section == SECTION_YSF_P25) {
			if (::strcmp(key, "Enable") == 0) {
				m_ysfP25Enable = ::atoi(value) == 1;
			} else if (::strcmp(key, "DGId") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint8_t dgId = uint8_t(::atoi(value));

				uint32_t tgid = uint32_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "YSF => P25, mapping %u to TG%u\n", dgId, tgid);
#endif
				m_ysfP25DGIds.push_back(std::pair<uint8_t, uint32_t>(dgId, tgid));
			}
		} else if (section == SECTION_YSF_NXDN) {
			if (::strcmp(key, "Enable") == 0) {
				m_ysfNXDNEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "DGId") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint8_t dgId = uint8_t(::atoi(value));

				uint16_t tgid = uint16_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "YSF => NXDN, mapping %u to TG%u\n", dgId, tgid);
#endif
				m_ysfNXDNDGIds.push_back(std::pair<uint8_t, uint16_t>(dgId, tgid));
			}
		} else if (section == SECTION_YSF_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_ysfFMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "DGId") == 0) {
				m_ysfFMDGId = uint8_t(::atoi(value));
#if defined(TRACE_CONFIG)
				::fprintf(stdout, "YSF => FM, mapping %u\n", m_ysfFMDGId);
#endif
			}
		} else if (section == SECTION_P25_DSTAR) {
			if (::strcmp(key, "Enable") == 0) {
				m_p25DStarEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint32_t tg = uint32_t(::atoi(value));

				std::string dest = getString(p + 1U);
				if (dest.empty())
					continue;

	#if defined(TRACE_CONFIG)
				::fprintf(stdout, "P25 => D-Star, mapping TG%u to \"%s\"\n", tg, dest.c_str());
	#endif
				m_p25DStarTGs.push_back(std::pair<uint32_t, std::string>(tg, dest));
			}
		} else if (section == SECTION_P25_DMR) {
			if (::strcmp(key, "Enable") == 0) {
				m_p25DMREnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint32_t tg = uint32_t(::atoi(value));

				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(p + 1U);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "P25 => DMR, mapping TG%u to %u:TG%u\n", tg, slotTG.first, slotTG.second);
#endif
				m_p25DMRTGs.push_back(std::tuple<uint32_t, uint8_t, uint32_t>(tg, slotTG.first, slotTG.second));
			}
		} else if (section == SECTION_P25_YSF) {
			if (::strcmp(key, "Enable") == 0) {
				m_p25YSFEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint32_t tg = uint32_t(::atoi(value));

				uint8_t dgId = uint8_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "P25 => YSF, mapping TG%u to %u\n", tg, dgId);
#endif
				m_p25YSFTGs.push_back(std::pair<uint32_t, uint8_t>(tg, dgId));
			}
		} else if (section == SECTION_P25_P25) {
			if (::strcmp(key, "Enable") == 0)
				m_p25P25Enable = ::atoi(value) == 1;
		} else if (section == SECTION_P25_NXDN) {
			if (::strcmp(key, "Enable") == 0) {
				m_p25NXDNEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint32_t tg1 = uint32_t(::atoi(value));

				uint16_t tg2 = uint16_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "P25 => NXDN, mapping TG%u to TG%u\n", tg1, tg2);
#endif
				m_p25NXDNTGs.push_back(std::pair<uint32_t, uint16_t>(tg1, tg2));
			}
		} else if (section == SECTION_P25_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_p25FMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				m_p25FMTG = uint32_t(::atoi(value));
#if defined(TRACE_CONFIG)
				::fprintf(stdout, "P25 => FM, mapping TG%u\n", m_p25FMTG);
#endif
			}
		} else if (section == SECTION_NXDN_DSTAR) {
			if (::strcmp(key, "Enable") == 0) {
				m_nxdnDStarEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint16_t tg = uint16_t(::atoi(value));

				std::string dest = getString(p + 1U);
				if (dest.empty())
					continue;

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "NXDN => D-Star, mapping TG%u to \"%s\"\n", tg, dest.c_str());
#endif
				m_nxdnDStarTGs.push_back(std::pair<uint16_t, std::string>(tg, dest));
			}
		} else if (section == SECTION_NXDN_DMR) {
			if (::strcmp(key, "Enable") == 0) {
				m_nxdnDMREnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint16_t tg = uint16_t(::atoi(value));

				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(p + 1U);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "NXDN => DMR, mapping TG%u to %u:TG%u\n", tg, slotTG.first, slotTG.second);
#endif
				m_nxdnDMRTGs.push_back(std::tuple<uint16_t, uint8_t, uint32_t>(tg, slotTG.first, slotTG.second));
			}
		} else if (section == SECTION_NXDN_YSF) {
			if (::strcmp(key, "Enable") == 0) {
				m_nxdnYSFEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint16_t tg = uint16_t(::atoi(value));

				uint8_t dgid = uint8_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "NXDN => YSF, mapping TG%u to %u\n", tg, dgid);
#endif
				m_nxdnYSFTGs.push_back(std::pair<uint16_t, uint8_t>(tg, dgid));
			}
		} else if (section == SECTION_NXDN_P25) {
			if (::strcmp(key, "Enable") == 0) {
				m_nxdnP25Enable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					continue;
				*p = '\0';

				uint16_t tg1 = uint16_t(::atoi(value));

				uint32_t tg2 = uint32_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "NXDN => P25, mapping TG%u to TG%u\n", tg1, tg2);
#endif
				m_nxdnP25TGs.push_back(std::pair<uint16_t, uint32_t>(tg1, tg2));
			}
		} else if (section == SECTION_NXDN_NXDN) {
			if (::strcmp(key, "Enable") == 0)
				m_nxdnNXDNEnable = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_nxdnFMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				m_nxdnFMTG = uint16_t(::atoi(value));
#if defined(TRACE_CONFIG)
				::fprintf(stdout, "NXDN => FM, mapping TG%u\n", m_nxdnFMTG);
#endif
			}
		} else if (section == SECTION_FM_DSTAR) {
			if (::strcmp(key, "Enable") == 0)
				m_fmDStarEnable = ::atoi(value) == 1;
		} else if (section == SECTION_FM_DMR) {
			if (::strcmp(key, "Enable") == 0)
				m_fmDMREnable = ::atoi(value) == 1;
		} else if (section == SECTION_FM_YSF) {
			if (::strcmp(key, "Enable") == 0)
				m_fmYSFEnable = ::atoi(value) == 1;
		} else if (section == SECTION_FM_P25) {
			if (::strcmp(key, "Enable") == 0)
				m_fmP25Enable = ::atoi(value) == 1;
		} else if (section == SECTION_FM_NXDN) {
			if (::strcmp(key, "Enable") == 0)
				m_fmNXDNEnable = ::atoi(value) == 1;
		} else if (section == SECTION_FM_FM) {
			if (::strcmp(key, "Enable") == 0)
				m_fmFMEnable = ::atoi(value) == 1;
		} else if (section == SECTION_DSTAR_NETWORK_FROM) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_dStarFromRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_dStarFromRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dStarFromLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dStarFromLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dStarFromDebug = ::atoi(value) == 1;
		} else if (section == SECTION_DSTAR_NETWORK_TO) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_dStarToRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_dStarToRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dStarToLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dStarToLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dStarToDebug = ::atoi(value) == 1;
		} else if (section == SECTION_DMR_NETWORK_FROM) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_dmrFromRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_dmrFromRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dmrFromLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dmrFromLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dmrFromDebug = ::atoi(value) == 1;
		} else if (section == SECTION_DMR_NETWORK_TO) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_dmrToRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_dmrToRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dmrToLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dmrToLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dmrToDebug = ::atoi(value) == 1;
		} else if (section == SECTION_YSF_NETWORK_FROM) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_ysfFromRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_ysfFromRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_ysfFromLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_ysfFromLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_ysfFromDebug = ::atoi(value) == 1;
		} else if (section == SECTION_YSF_NETWORK_TO) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_ysfToRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_ysfToRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_ysfToLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_ysfToLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_ysfToDebug = ::atoi(value) == 1;
		} else if (section == SECTION_P25_NETWORK_FROM) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_p25FromRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_p25FromRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_p25FromLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_p25FromLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_p25FromDebug = ::atoi(value) == 1;
		} else if (section == SECTION_P25_NETWORK_TO) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_p25ToRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_p25ToRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_p25ToLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_p25ToLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_p25ToDebug = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_NETWORK_FROM) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_nxdnFromRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_nxdnFromRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_nxdnFromLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_nxdnFromLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_nxdnFromDebug = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_NETWORK_TO) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_nxdnToRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_nxdnToRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_nxdnToLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_nxdnToLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_nxdnToDebug = ::atoi(value) == 1;
		} else if (section == SECTION_FM_NETWORK_FROM) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_fmFromRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_fmFromRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_fmFromLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_fmFromLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_fmFromDebug = ::atoi(value) == 1;
		} else if (section == SECTION_FM_NETWORK_TO) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_fmToRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_fmToRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_fmToLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_fmToLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_fmToDebug = ::atoi(value) == 1;
		}
	}

	::fclose(fp);

	return true;
}

bool CConf::getDaemon() const
{
	return m_daemon;
}

std::string CConf::getCallsign() const
{
	return m_callsign;
}

unsigned int CConf::getRFModeHang() const
{
	return m_rfModeHang;
}

unsigned int CConf::getNetModeHang() const
{
	return m_netModeHang;
}

std::string CConf::getFromMode() const
{
	return m_fromMode;
}

uint32_t CConf::getLogDisplayLevel() const
{
	return m_logDisplayLevel;
}

uint32_t CConf::getLogFileLevel() const
{
	return m_logFileLevel;
}

std::string CConf::getLogFilePath() const
{
	return m_logFilePath;
}

std::string CConf::getLogFileRoot() const
{
	return m_logFileRoot;
}

bool CConf::getLogFileRotate() const
{
	return m_logFileRotate;
}

std::string CConf::getTranscoderPort() const
{
	return m_transcoderPort;
}

uint32_t CConf::getTranscoderSpeed() const
{
	return m_transcoderSpeed;
}

bool CConf::getTranscoderDebug() const
{
	return m_transcoderDebug;
}

std::string CConf::getDMRLookupFile() const
{
	return m_dmrLookupFile;
}

std::string CConf::getNXDNLookupFile() const
{
	return m_nxdnLookupFile;
}

unsigned int CConf::getReloadTime() const
{
	return m_reloadTime;
}

std::string CConf::getDStarModule() const
{
	return m_dStarModule;
}

uint32_t CConf::getDMRId() const
{
	return m_dmrId;
}

uint16_t CConf::getNXDNId() const
{
	return m_nxdnId;
}

bool CConf::getDStarDStarEnable() const
{
	return m_dstarDStarEnable;
}

bool CConf::getDStarDMREnable() const
{
	return m_dstarDMREnable;
}

std::vector<std::tuple<std::string, uint8_t, uint32_t>> CConf::getDStarDMRDests() const
{
	return m_dstarDMRDests;
}

bool CConf::getDStarYSFEnable() const
{
	return m_dstarYSFEnable;
}

std::vector<std::pair<std::string, uint8_t>> CConf::getDStarYSFDests() const
{
	return m_dstarYSFDests;
}

bool CConf::getDStarP25Enable() const
{
	return m_dstarP25Enable;
}

std::vector<std::pair<std::string, uint32_t>> CConf::getDStarP25Dests() const
{
	return m_dstarP25Dests;
}

bool CConf::getDStarNXDNEnable() const
{
	return m_dstarNXDNEnable;
}

std::vector<std::pair<std::string, uint16_t>> CConf::getDStarNXDNDests() const
{
	return m_dstarNXDNDests;
}

bool CConf::getDStarFMEnable() const
{
	return m_dstarFMEnable;
}

std::string CConf::getDStarFMDest() const
{
	return m_dstarFMDest;
}

bool CConf::getDMRDStarEnable() const
{
	return m_dmrDStarEnable;
}

std::vector<std::tuple<uint8_t, uint32_t, std::string>> CConf::getDMRDStarTGs() const
{
	return m_dmrDStarTGs;
}

bool CConf::getDMRDMREnable1() const
{
	return m_dmrDMREnable1;
}

bool CConf::getDMRDMREnable2() const
{
	return m_dmrDMREnable2;
}

bool CConf::getDMRYSFEnable() const
{
	return m_dmrYSFEnable;
}

std::vector<std::tuple<uint8_t, uint32_t, uint8_t>> CConf::getDMRYSFTGs() const
{
	return m_dmrYSFTGs;
}

bool CConf::getDMRP25Enable() const
{
	return m_dmrP25Enable;
}

std::vector<std::tuple<uint8_t, uint32_t, uint32_t>> CConf::getDMRP25TGs() const
{
	return m_dmrP25TGs;
}

bool CConf::getDMRNXDNEnable() const
{
	return m_dmrNXDNEnable;
}

std::vector<std::tuple<uint8_t, uint32_t, uint16_t>> CConf::getDMRNXDNTGs() const
{
	return m_dmrNXDNTGs;
}

bool CConf::getDMRFMEnable() const
{
	return m_dmrFMEnable;
}

std::pair<uint8_t, uint32_t> CConf::getDMRFMTG() const
{
	return m_dmrFMTG;
}

bool CConf::getYSFDStarEnable() const
{
	return m_ysfDStarEnable;
}

std::vector<std::pair<uint8_t, std::string>> CConf::getYSFDStarDGIds() const
{
	return m_ysfDStarDGIds;
}

bool CConf::getYSFDMREnable() const
{
	return m_ysfDMREnable;
}

std::vector<std::tuple<uint8_t, uint8_t, uint32_t>> CConf::getYSFDMRDGIds() const
{
	return m_ysfDMRDGIds;
}

bool CConf::getYSFYSFEnable() const
{
	return m_ysfYSFEnable;
}

bool CConf::getYSFP25Enable() const
{
	return m_ysfP25Enable;
}

std::vector<std::pair<uint8_t, uint32_t>> CConf::getYSFP25DGIds() const
{
	return m_ysfP25DGIds;
}

bool CConf::getYSFNXDNEnable() const
{
	return m_ysfNXDNEnable;
}

std::vector<std::pair<uint8_t, uint16_t>> CConf::getYSFNXDNDGIds() const
{
	return m_ysfNXDNDGIds;
}

bool CConf::getYSFFMEnable() const
{
	return m_ysfFMEnable;
}

uint8_t CConf::getYSFFMDGId() const
{
	return m_ysfFMDGId;
}

bool CConf::getP25DStarEnable() const
{
	return m_p25DStarEnable;
}

std::vector<std::pair<uint32_t, std::string>> CConf::getP25DStarTGs() const
{
	return m_p25DStarTGs;
}

bool CConf::getP25DMREnable() const
{
	return m_p25DMREnable;
}

std::vector<std::tuple<uint32_t, uint8_t, uint32_t>> CConf::getP25DMRTGs() const
{
	return m_p25DMRTGs;
}

bool CConf::getP25YSFEnable() const
{
	return m_p25YSFEnable;
}

std::vector<std::pair<uint32_t, uint8_t>> CConf::getP25YSFTGs() const
{
	return m_p25YSFTGs;
}

bool CConf::getP25P25Enable() const
{
	return m_p25P25Enable;
}

bool CConf::getP25NXDNEnable() const
{
	return m_p25NXDNEnable;
}

std::vector<std::pair<uint32_t, uint16_t>> CConf::getP25NXDNTGs() const
{
	return m_p25NXDNTGs;
}

bool CConf::getP25FMEnable() const
{
	return m_p25FMEnable;
}

uint32_t CConf::getP25FMTG() const
{
	return m_p25FMTG;
}

bool CConf::getNXDNDStarEnable() const
{
	return m_nxdnDStarEnable;
}

std::vector<std::pair<uint16_t, std::string>> CConf::getNXDNDStarTGs() const
{
	return m_nxdnDStarTGs;
}

bool CConf::getNXDNDMREnable() const
{
	return m_nxdnDMREnable;
}

std::vector<std::tuple<uint16_t, uint8_t, uint32_t>> CConf::getNXDNDMRTGs() const
{
	return m_nxdnDMRTGs;
}

bool CConf::getNXDNYSFEnable() const
{
	return m_nxdnYSFEnable;
}

std::vector<std::pair<uint16_t, uint8_t>> CConf::getNXDNYSFTGs() const
{
	return m_nxdnYSFTGs;
}

bool CConf::getNXDNP25Enable() const
{
	return m_nxdnP25Enable;
}

std::vector<std::pair<uint16_t, uint32_t>> CConf::getNXDNP25TGs() const
{
	return m_nxdnP25TGs;
}

bool CConf::getNXDNNXDNEnable() const
{
	return m_nxdnNXDNEnable;
}

bool CConf::getNXDNFMEnable() const
{
	return m_nxdnFMEnable;
}

uint16_t CConf::getNXDNFMTG() const
{
	return m_nxdnFMTG;
}

bool CConf::getFMDStarEnable() const
{
	return m_fmDStarEnable;
}

bool CConf::getFMDMREnable() const
{
	return m_fmDMREnable;
}

bool CConf::getFMYSFEnable() const
{
	return m_fmYSFEnable;
}

bool CConf::getFMP25Enable() const
{
	return m_fmP25Enable;
}

bool CConf::getFMNXDNEnable() const
{
	return m_fmNXDNEnable;
}

bool CConf::getFMFMEnable() const
{
	return m_fmFMEnable;
}

std::string CConf::getDStarFromRemoteAddress() const
{
	return m_dStarFromRemoteAddress;
}

uint16_t CConf::getDStarFromRemotePort() const
{
	return m_dStarFromRemotePort;
}

std::string CConf::getDStarFromLocalAddress() const
{
	return m_dStarFromLocalAddress;
}

uint16_t CConf::getDStarFromLocalPort() const
{
	return m_dStarFromLocalPort;
}

bool CConf::getDStarFromDebug() const
{
	return m_dStarFromDebug;
}

std::string CConf::getDStarToRemoteAddress() const
{
	return m_dStarToRemoteAddress;
}

uint16_t CConf::getDStarToRemotePort() const
{
	return m_dStarToRemotePort;
}

std::string CConf::getDStarToLocalAddress() const
{
	return m_dStarToLocalAddress;
}

uint16_t CConf::getDStarToLocalPort() const
{
	return m_dStarToLocalPort;
}

bool CConf::getDStarToDebug() const
{
	return m_dStarToDebug;
}

std::string CConf::getDMRFromRemoteAddress() const
{
	return m_dmrFromRemoteAddress;
}

uint16_t CConf::getDMRFromRemotePort() const
{
	return m_dmrFromRemotePort;
}

std::string CConf::getDMRFromLocalAddress() const
{
	return m_dmrFromLocalAddress;
}

uint16_t CConf::getDMRFromLocalPort() const
{
	return m_dmrFromLocalPort;
}

bool CConf::getDMRFromDebug() const
{
	return m_dmrFromDebug;
}

std::string CConf::getDMRToRemoteAddress() const
{
	return m_dmrToRemoteAddress;
}

uint16_t CConf::getDMRToRemotePort() const
{
	return m_dmrToRemotePort;
}

std::string CConf::getDMRToLocalAddress() const
{
	return m_dmrToLocalAddress;
}

uint16_t CConf::getDMRToLocalPort() const
{
	return m_dmrToLocalPort;
}

bool CConf::getDMRToDebug() const
{
	return m_dmrToDebug;
}

std::string CConf::getYSFFromRemoteAddress() const
{
	return m_ysfFromRemoteAddress;
}

uint16_t CConf::getYSFFromRemotePort() const
{
	return m_ysfFromRemotePort;
}

std::string CConf::getYSFFromLocalAddress() const
{
	return m_ysfFromLocalAddress;
}

uint16_t CConf::getYSFFromLocalPort() const
{
	return m_ysfFromLocalPort;
}

bool CConf::getYSFFromDebug() const
{
	return m_ysfFromDebug;
}

std::string CConf::getYSFToRemoteAddress() const
{
	return m_ysfToRemoteAddress;
}

uint16_t CConf::getYSFToRemotePort() const
{
	return m_ysfToRemotePort;
}

std::string CConf::getYSFToLocalAddress() const
{
	return m_ysfToLocalAddress;
}

uint16_t CConf::getYSFToLocalPort() const
{
	return m_ysfToLocalPort;
}

bool CConf::getYSFToDebug() const
{
	return m_ysfToDebug;
}

std::string CConf::getP25FromRemoteAddress() const
{
	return m_p25FromRemoteAddress;
}

uint16_t CConf::getP25FromRemotePort() const
{
	return m_p25FromRemotePort;
}

std::string CConf::getP25FromLocalAddress() const
{
	return m_p25FromLocalAddress;
}

uint16_t CConf::getP25FromLocalPort() const
{
	return m_p25FromLocalPort;
}

bool CConf::getP25FromDebug() const
{
	return m_p25FromDebug;
}

std::string CConf::getP25ToRemoteAddress() const
{
	return m_p25ToRemoteAddress;
}

uint16_t CConf::getP25ToRemotePort() const
{
	return m_p25ToRemotePort;
}

std::string CConf::getP25ToLocalAddress() const
{
	return m_p25ToLocalAddress;
}

uint16_t CConf::getP25ToLocalPort() const
{
	return m_p25ToLocalPort;
}

bool CConf::getP25ToDebug() const
{
	return m_p25ToDebug;
}

std::string CConf::getNXDNFromRemoteAddress() const
{
	return m_nxdnFromRemoteAddress;
}

uint16_t CConf::getNXDNFromRemotePort() const
{
	return m_nxdnFromRemotePort;
}

std::string CConf::getNXDNFromLocalAddress() const
{
	return m_nxdnFromLocalAddress;
}

uint16_t CConf::getNXDNFromLocalPort() const
{
	return m_nxdnFromLocalPort;
}

bool CConf::getNXDNFromDebug() const
{
	return m_nxdnFromDebug;
}

std::string CConf::getNXDNToRemoteAddress() const
{
	return m_nxdnToRemoteAddress;
}

uint16_t CConf::getNXDNToRemotePort() const
{
	return m_nxdnToRemotePort;
}

std::string CConf::getNXDNToLocalAddress() const
{
	return m_nxdnToLocalAddress;
}

uint16_t CConf::getNXDNToLocalPort() const
{
	return m_nxdnToLocalPort;
}

bool CConf::getNXDNToDebug() const
{
	return m_nxdnToDebug;
}

std::string CConf::getFMFromRemoteAddress() const
{
	return m_fmFromRemoteAddress;
}

uint16_t CConf::getFMFromRemotePort() const
{
	return m_fmFromRemotePort;
}

std::string CConf::getFMFromLocalAddress() const
{
	return m_fmFromLocalAddress;
}

uint16_t CConf::getFMFromLocalPort() const
{
	return m_fmFromLocalPort;
}

bool CConf::getFMFromDebug() const
{
	return m_fmFromDebug;
}

std::string CConf::getFMToRemoteAddress() const
{
	return m_fmToRemoteAddress;
}

uint16_t CConf::getFMToRemotePort() const
{
	return m_fmToRemotePort;
}

std::string CConf::getFMToLocalAddress() const
{
	return m_fmToLocalAddress;
}

uint16_t CConf::getFMToLocalPort() const
{
	return m_fmToLocalPort;
}

bool CConf::getFMToDebug() const
{
	return m_fmToDebug;
}

std::string CConf::getString(const char* text) const
{
	assert(text != nullptr);

	size_t len = ::strlen(text);
	if (len > 1U && *text == '"' && text[len - 1U] == '"')
		return std::string(text + 1U, len - 2U);
	else
		return std::string(text);
}

std::pair<uint8_t, uint32_t> CConf::getSlotTG(char* text) const
{
	assert(text != nullptr);

	char* p = ::strchr(text, ',');
	if (p == nullptr)
		return std::pair<uint8_t, uint32_t>(NULL_SLOT, NULL_ID32);

	*p = '\0';

	uint8_t  slot = uint8_t(::atoi(text));
	uint32_t   id = uint32_t(::atoi(p + 1U));

	return std::pair<uint8_t, uint32_t>(slot, id);
}
