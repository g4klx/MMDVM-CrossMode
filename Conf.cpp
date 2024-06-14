/*
 *	 Copyright (C) 2015,2016,2017,2024 by Jonathan Naylor G4KLX
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
	SECTION_DSTAR_M17,
	SECTION_DMR_DSTAR,
	SECTION_DMR_DMR,
	SECTION_DMR_YSF,
	SECTION_DMR_P25,
	SECTION_DMR_NXDN,
	SECTION_DMR_FM,
	SECTION_DMR_M17,
	SECTION_YSF_DSTAR,
	SECTION_YSF_DMR,
	SECTION_YSF_YSF,
	SECTION_YSF_P25,
	SECTION_YSF_NXDN,
	SECTION_YSF_FM,
	SECTION_YSF_M17,
	SECTION_P25_DSTAR,
	SECTION_P25_DMR,
	SECTION_P25_YSF,
	SECTION_P25_P25,
	SECTION_P25_NXDN,
	SECTION_P25_FM,
	SECTION_P25_M17,
	SECTION_NXDN_DSTAR,
	SECTION_NXDN_DMR,
	SECTION_NXDN_YSF,
	SECTION_NXDN_P25,
	SECTION_NXDN_NXDN,
	SECTION_NXDN_FM,
	SECTION_NXDN_M17,
	SECTION_FM_DSTAR,
	SECTION_FM_DMR,
	SECTION_FM_YSF,
	SECTION_FM_P25,
	SECTION_FM_NXDN,
	SECTION_FM_FM,
	SECTION_FM_M17,
	SECTION_M17_DSTAR,
	SECTION_M17_DMR,
	SECTION_M17_YSF,
	SECTION_M17_P25,
	SECTION_M17_NXDN,
	SECTION_M17_FM,
	SECTION_M17_M17,
	SECTION_DSTAR_NETWORK_FROM,
	SECTION_DSTAR_NETWORK_TO,
	SECTION_DMR_NETWORK_FROM,
	SECTION_DMR_NETWORK_TO,
	SECTION_YSF_NETWORK_FROM,
	SECTION_YSF_NETWORK_TO,
	SECTION_FM_NETWORK_FROM,
	SECTION_FM_NETWORK_TO,
	SECTION_M17_NETWORK_FROM,
	SECTION_M17_NETWORK_TO
};

CConf::CConf(const std::string& file) :
m_file(file),
m_callsign("G9BF"),
m_rfModeHang(10U),
m_netModeHang(3U),
m_fromMode(),
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
m_dstarNXDNDests(),
m_dstarNXDNEnable(false),
m_dstarFMEnable(false),
m_dstarM17Enable(false),
m_dstarM17Dests(),
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
m_dmrM17Enable(false),
m_dmrM17TGs(),
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
m_ysfM17Enable(false),
m_ysfM17DGIds(),
m_p25DStarEnable(false),
m_p25DMREnable(false),
m_p25YSFEnable(false),
m_p25P25Enable(false),
m_p25NXDNEnable(false),
m_p25FMEnable(false),
m_p25M17Enable(false),
m_nxdnDStarEnable(false),
m_nxdnDMREnable(false),
m_nxdnYSFEnable(false),
m_nxdnP25Enable(false),
m_nxdnNXDNEnable(false),
m_nxdnFMEnable(false),
m_nxdnM17Enable(false),
m_fmDStarEnable(false),
m_fmDMREnable(false),
m_fmYSFEnable(false),
m_fmP25Enable(false),
m_fmNXDNEnable(false),
m_fmFMEnable(false),
m_fmM17Enable(false),
m_m17DStarEnable(false),
m_m17DStarDests(),
m_m17DMREnable(false),
m_m17YSFEnable(false),
m_m17P25Enable(false),
m_m17NXDNEnable(false),
m_m17FMEnable(false),
m_m17FMDest(),
m_m17M17Enable(false),
m_dStarFromRemoteAddress("127.0.0.1"),
m_dStarFromRemotePort(20011U),
m_dStarFromLocalAddress(),
m_dStarFromLocalPort(20010U),
m_dStarFromDebug(false),
m_dStarToRemoteAddress("127.0.0.1"),
m_dStarToRemotePort(20011U),
m_dStarToLocalAddress(),
m_dStarToLocalPort(20010U),
m_dStarToDebug(false),
m_dmrFromRemoteAddress("127.0.0.1"),
m_dmrFromRemotePort(62031U),
m_dmrFromLocalAddress(),
m_dmrFromLocalPort(62032U),
m_dmrFromDebug(false),
m_dmrToRemoteAddress("127.0.0.1"),
m_dmrToRemotePort(62032U),
m_dmrToLocalAddress(),
m_dmrToLocalPort(62031U),
m_dmrToDebug(false),
m_ysfFromRemoteAddress("127.0.0.1"),
m_ysfFromRemotePort(20011U),
m_ysfFromLocalAddress(),
m_ysfFromLocalPort(20010U),
m_ysfFromDebug(false),
m_ysfToRemoteAddress("127.0.0.1"),
m_ysfToRemotePort(20011U),
m_ysfToLocalAddress(),
m_ysfToLocalPort(20010U),
m_ysfToDebug(false),
m_fmFromRemoteAddress("127.0.0.1"),
m_fmFromRemotePort(20011U),
m_fmFromLocalAddress(),
m_fmFromLocalPort(20010U),
m_fmFromDebug(false),
m_fmToRemoteAddress("127.0.0.1"),
m_fmToRemotePort(20011U),
m_fmToLocalAddress(),
m_fmToLocalPort(20010U),
m_fmToDebug(false),
m_m17FromRemoteAddress("127.0.0.1"),
m_m17FromRemotePort(17011U),
m_m17FromLocalAddress(),
m_m17FromLocalPort(17010U),
m_m17FromDebug(false),
m_m17ToRemoteAddress("127.0.0.1"),
m_m17ToRemotePort(17011U),
m_m17ToLocalAddress(),
m_m17ToLocalPort(17010U),
m_m17ToDebug(false)
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
			else if (::strncmp(buffer, "[D-Star to M17]", 15U) == 0)
				section = SECTION_DSTAR_M17;
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
			else if (::strncmp(buffer, "[DMR to M17]", 12U) == 0)
				section = SECTION_DMR_M17;
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
			else if (::strncmp(buffer, "[System Fusion to M17]", 22U) == 0)
				section = SECTION_YSF_M17;
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
			else if (::strncmp(buffer, "[P25 to M17]", 12U) == 0)
				section = SECTION_P25_M17;
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
			else if (::strncmp(buffer, "[NXDN to M17]", 13U) == 0)
				section = SECTION_NXDN_M17;
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
			else if (::strncmp(buffer, "[FM to M17]", 11U) == 0)
				section = SECTION_FM_M17;
			else if (::strncmp(buffer, "[M17 to D-Star]", 15U) == 0)
				section = SECTION_M17_DSTAR;
			else if (::strncmp(buffer, "[M17 to DMR]", 12U) == 0)
				section = SECTION_M17_DMR;
			else if (::strncmp(buffer, "[M17 to System Fusion]", 22U) == 0)
				section = SECTION_M17_YSF;
			else if (::strncmp(buffer, "[M17 to P25]", 12U) == 0)
				section = SECTION_M17_P25;
			else if (::strncmp(buffer, "[M17 to NXDN]", 13U) == 0)
				section = SECTION_M17_NXDN;
			else if (::strncmp(buffer, "[M17 to FM]", 11U) == 0)
				section = SECTION_M17_FM;
			else if (::strncmp(buffer, "[M17 to M17]", 12U) == 0)
				section = SECTION_M17_M17;
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
			else if (::strncmp(buffer, "[FM Network From]", 17U) == 0)
				section = SECTION_FM_NETWORK_FROM;
			else if (::strncmp(buffer, "[FM Network To]", 15U) == 0)
				section = SECTION_FM_NETWORK_TO;
			else if (::strncmp(buffer, "[M17 Network From]", 18U) == 0)
				section = SECTION_M17_NETWORK_FROM;
			else if (::strncmp(buffer, "[M17 Network To]", 16U) == 0)
				section = SECTION_M17_NETWORK_TO;
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

				uint16_t tgid = uint16_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "D-Star => P25, mapping \"%s\" to TG%u\n", dest.c_str(), tgid);
#endif
				m_dstarP25Dests.push_back(std::pair<std::string, uint16_t>(dest, tgid));
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
		} else if (section == SECTION_DSTAR_M17) {
			if (::strcmp(key, "Enable") == 0) {
				m_dstarM17Enable = ::atoi(value) == 1;
			} else if (::strcmp(key, "Dest") == 0) {
				std::string dest = getString(value);
				if (dest.empty())
					continue;
#if defined(TRACE_CONFIG)
				::fprintf(stdout, "D-Star => M17, mapping \"%s\"\n", dest.c_str());
#endif
				m_dstarM17Dests.push_back(dest);
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

				uint16_t tgId = uint16_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "DMR => P25, mapping %u:TG%u to TG%u\n", slotTG.first, slotTG.second, tgId);
#endif
				m_dmrP25TGs.push_back(std::tuple<uint8_t, uint32_t, uint16_t>(slotTG.first, slotTG.second, tgId));
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
		} else if (section == SECTION_DMR_M17) {
			if (::strcmp(key, "Enable") == 0) {
				m_dmrM17Enable = ::atoi(value) == 1;
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
				::fprintf(stdout, "DMR => M17, mapping %u:TG%u to \"%s\"\n", slotTG.first, slotTG.second, dest.c_str());
#endif
				m_dmrM17TGs.push_back(std::tuple<uint8_t, uint32_t, std::string>(slotTG.first, slotTG.second, dest));
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

				uint16_t tgid = uint16_t(::atoi(p + 1U));

#if defined(TRACE_CONFIG)
				::fprintf(stdout, "YSF => P25, mapping %u to TG%u\n", dgId, tgid);
#endif
				m_ysfP25DGIds.push_back(std::pair<uint8_t, uint16_t>(dgId, tgid));
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
		} else if (section == SECTION_YSF_M17) {
			if (::strcmp(key, "Enable") == 0) {
				m_ysfM17Enable = ::atoi(value) == 1;
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
				::fprintf(stdout, "YSF => M17, mapping %u to \"%s\"\n", dgId, dest.c_str());
#endif
				m_ysfM17DGIds.push_back(std::pair<uint8_t, std::string>(dgId, dest));
			}
		} else if (section == SECTION_P25_DSTAR) {
			if (::strcmp(key, "Enable") == 0)
				m_p25DStarEnable = ::atoi(value) == 1;
		} else if (section == SECTION_P25_DMR) {
			if (::strcmp(key, "Enable") == 0)
				m_p25DMREnable = ::atoi(value) == 1;
		} else if (section == SECTION_P25_YSF) {
			if (::strcmp(key, "Enable") == 0)
				m_p25YSFEnable = ::atoi(value) == 1;
		} else if (section == SECTION_P25_P25) {
			if (::strcmp(key, "Enable") == 0)
				m_p25P25Enable = ::atoi(value) == 1;
		} else if (section == SECTION_P25_NXDN) {
			if (::strcmp(key, "Enable") == 0)
				m_p25NXDNEnable = ::atoi(value) == 1;
		} else if (section == SECTION_P25_FM) {
			if (::strcmp(key, "Enable") == 0)
				m_p25FMEnable = ::atoi(value) == 1;
		} else if (section == SECTION_P25_M17) {
			if (::strcmp(key, "Enable") == 0)
				m_p25M17Enable = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_DSTAR) {
			if (::strcmp(key, "Enable") == 0)
				m_nxdnDStarEnable = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_DMR) {
			if (::strcmp(key, "Enable") == 0)
				m_nxdnDMREnable = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_YSF) {
			if (::strcmp(key, "Enable") == 0)
				m_nxdnYSFEnable = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_P25) {
			if (::strcmp(key, "Enable") == 0)
				m_nxdnP25Enable = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_NXDN) {
			if (::strcmp(key, "Enable") == 0)
				m_nxdnNXDNEnable = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_FM) {
			if (::strcmp(key, "Enable") == 0)
				m_nxdnFMEnable = ::atoi(value) == 1;
		} else if (section == SECTION_NXDN_M17) {
			if (::strcmp(key, "Enable") == 0)
				m_nxdnM17Enable = ::atoi(value) == 1;
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
		} else if (section == SECTION_FM_M17) {
			if (::strcmp(key, "Enable") == 0)
				m_fmM17Enable = ::atoi(value) == 1;
		} else if (section == SECTION_M17_DSTAR) {
			if (::strcmp(key, "Enable") == 0)
				m_m17DStarEnable = ::atoi(value) == 1;
			else if (::strcmp(key, "Dest") == 0)
				m_m17DStarDests.push_back(value);
		} else if (section == SECTION_M17_DMR) {
			if (::strcmp(key, "Enable") == 0)
				m_m17DMREnable = ::atoi(value) == 1;
		} else if (section == SECTION_M17_YSF) {
			if (::strcmp(key, "Enable") == 0)
				m_m17YSFEnable = ::atoi(value) == 1;
		} else if (section == SECTION_M17_P25) {
			if (::strcmp(key, "Enable") == 0)
				m_m17P25Enable = ::atoi(value) == 1;
		} else if (section == SECTION_M17_NXDN) {
			if (::strcmp(key, "Enable") == 0)
				m_m17NXDNEnable = ::atoi(value) == 1;
		} else if (section == SECTION_M17_FM) {
			if (::strcmp(key, "Enable") == 0)
				m_m17FMEnable = ::atoi(value) == 1;
			else if (::strcmp(key, "Dest") == 0)
				m_m17FMDest = value;
		} else if (section == SECTION_M17_M17) {
			if (::strcmp(key, "Enable") == 0)
				m_m17M17Enable = ::atoi(value) == 1;
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
		} else if (section == SECTION_M17_NETWORK_FROM) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_m17FromRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_m17FromRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_m17FromLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_m17FromLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_m17FromDebug = ::atoi(value) == 1;
		} else if (section == SECTION_M17_NETWORK_TO) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_m17ToRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_m17ToRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_m17ToLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_m17ToLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_m17ToDebug = ::atoi(value) == 1;
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

std::vector<std::pair<std::string, uint16_t>> CConf::getDStarP25Dests() const
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

bool CConf::getDStarM17Enable() const
{
	return m_dstarM17Enable;
}

std::vector<std::string> CConf::getDStarM17Dests() const
{
	return m_dstarM17Dests;
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

std::vector<std::tuple<uint8_t, uint32_t, uint16_t>> CConf::getDMRP25TGs() const
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

bool CConf::getDMRM17Enable() const
{
	return m_dmrM17Enable;
}

std::vector<std::tuple<uint8_t, uint32_t, std::string>> CConf::getDMRM17TGs() const
{
	return m_dmrM17TGs;
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

std::vector<std::pair<uint8_t, uint16_t>> CConf::getYSFP25DGIds() const
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

bool CConf::getYSFM17Enable() const
{
	return m_ysfM17Enable;
}

std::vector<std::pair<uint8_t, std::string>> CConf::getYSFM17DGIds() const
{
	return m_ysfM17DGIds;
}

bool CConf::getP25DStarEnable() const
{
	return m_p25DStarEnable;
}

bool CConf::getP25DMREnable() const
{
	return m_p25DMREnable;
}

bool CConf::getP25YSFEnable() const
{
	return m_p25YSFEnable;
}

bool CConf::getP25P25Enable() const
{
	return m_p25P25Enable;
}

bool CConf::getP25NXDNEnable() const
{
	return m_p25NXDNEnable;
}

bool CConf::getP25FMEnable() const
{
	return m_p25FMEnable;
}

bool CConf::getP25M17Enable() const
{
	return m_p25M17Enable;
}

bool CConf::getNXDNDStarEnable() const
{
	return m_nxdnDStarEnable;
}

bool CConf::getNXDNDMREnable() const
{
	return m_nxdnDMREnable;
}

bool CConf::getNXDNYSFEnable() const
{
	return m_nxdnYSFEnable;
}

bool CConf::getNXDNP25Enable() const
{
	return m_nxdnP25Enable;
}

bool CConf::getNXDNNXDNEnable() const
{
	return m_nxdnNXDNEnable;
}

bool CConf::getNXDNFMEnable() const
{
	return m_nxdnFMEnable;
}

bool CConf::getNXDNM17Enable() const
{
	return m_nxdnM17Enable;
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

bool CConf::getFMM17Enable() const
{
	return m_fmM17Enable;
}

bool CConf::getM17DStarEnable() const
{
	return m_m17DStarEnable;
}

std::vector<std::string> CConf::getM17DStarDests() const
{
	return m_m17DStarDests;
}

bool CConf::getM17DMREnable() const
{
	return m_m17DMREnable;
}

bool CConf::getM17YSFEnable() const
{
	return m_m17YSFEnable;
}

bool CConf::getM17P25Enable() const
{
	return m_m17P25Enable;
}

bool CConf::getM17NXDNEnable() const
{
	return m_m17NXDNEnable;
}

bool CConf::getM17FMEnable() const
{
	return m_m17FMEnable;
}

std::string CConf::getM17FMDest() const
{
	return m_m17FMDest;
}

bool CConf::getM17M17Enable() const
{
	return m_m17M17Enable;
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

std::string CConf::getM17FromRemoteAddress() const
{
	return m_m17FromRemoteAddress;
}

uint16_t CConf::getM17FromRemotePort() const
{
	return m_m17FromRemotePort;
}

std::string CConf::getM17FromLocalAddress() const
{
	return m_m17FromLocalAddress;
}

uint16_t CConf::getM17FromLocalPort() const
{
	return m_m17FromLocalPort;
}

bool CConf::getM17FromDebug() const
{
	return m_m17FromDebug;
}

std::string CConf::getM17ToRemoteAddress() const
{
	return m_m17ToRemoteAddress;
}

uint16_t CConf::getM17ToRemotePort() const
{
	return m_m17ToRemotePort;
}

std::string CConf::getM17ToLocalAddress() const
{
	return m_m17ToLocalAddress;
}

uint16_t CConf::getM17ToLocalPort() const
{
	return m_m17ToLocalPort;
}

bool CConf::getM17ToDebug() const
{
	return m_m17ToDebug;
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
