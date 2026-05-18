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

enum class SECTION {
	NONE,
	GENERAL,
	LOG,
	MQTT,
	TRANSCODER,
	INFO,
	LOOKUP,
	DSTAR,
	DMR,
	NXDN,
	DSTAR_DSTAR,
	DSTAR_DMR,
	DSTAR_YSF,
	DSTAR_P25,
	DSTAR_NXDN,
	DSTAR_FM,
	DMR_DSTAR,
	DMR_DMR,
	DMR_YSF,
	DMR_P25,
	DMR_NXDN,
	DMR_FM,
	YSF_DSTAR,
	YSF_DMR,
	YSF_YSF,
	YSF_P25,
	YSF_NXDN,
	YSF_FM,
	P25_DSTAR,
	P25_DMR,
	P25_YSF,
	P25_P25,
	P25_NXDN,
	P25_FM,
	NXDN_DSTAR,
	NXDN_DMR,
	NXDN_YSF,
	NXDN_P25,
	NXDN_NXDN,
	NXDN_FM,
	FM_DSTAR,
	FM_DMR,
	FM_YSF,
	FM_P25,
	FM_NXDN,
	FM_FM,
	DSTAR_NETWORK_RF,
	DSTAR_NETWORK_NET,
	DMR_NETWORK_RF,
	DMR_NETWORK_NET,
	YSF_NETWORK_RF,
	YSF_NETWORK_NET,
	P25_NETWORK_RF,
	P25_NETWORK_NET,
	NXDN_NETWORK_RF,
	NXDN_NETWORK_NET,
	FM_NETWORK_RF,
	FM_NETWORK_NET
};

CConf::CConf(const std::string& file) :
m_file(file),
m_callsign("G9BF"),
m_rfModeHang(10U),
m_netModeHang(3U),
m_daemon(false),
m_logDisplayLevel(0U),
m_logMQTTLevel(0U),
m_mqttAddress("127.0.0.1"),
m_mqttPort(1883U),
m_mqttKeepalive(60U),
m_mqttName("crossmode"), 
m_mqttAuthEnabled(false),
m_mqttUsername(),
m_mqttPassword(),
m_transcoderProtocol("uart"),
m_transcoderUARTPort(),
m_transcoderUARTSpeed(460800U),
m_transcoderRemoteAddress(),
m_transcoderRemotePort(0U),
m_transcoderLocalAddress(),
m_transcoderLocalPort(0U),
m_transcoderDebug(false),
m_dmrLookupFile(),
m_nxdnLookupFile(),
m_reloadTime(24U),
m_infoTXFrequency(0U),
m_infoRXFrequency(0U),
m_infoColorCode(0U),
m_infoPower(0U),
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
m_dStarRFRemoteAddress("127.0.0.1"),
m_dStarRFRemotePort(20011U),
m_dStarRFLocalAddress("127.0.0.1"),
m_dStarRFLocalPort(20010U),
m_dStarRFDebug(false),
m_dStarNetRemoteAddress("127.0.0.1"),
m_dStarNetRemotePort(20011U),
m_dStarNetLocalAddress("127.0.0.1"),
m_dStarNetLocalPort(20010U),
m_dStarNetDebug(false),
m_dmrRFRemoteAddress("127.0.0.1"),
m_dmrRFRemotePort(62031U),
m_dmrRFLocalAddress("127.0.0.1"),
m_dmrRFLocalPort(62032U),
m_dmrRFDebug(false),
m_dmrNetRemoteAddress("127.0.0.1"),
m_dmrNetRemotePort(62032U),
m_dmrNetLocalAddress("127.0.0.1"),
m_dmrNetLocalPort(62031U),
m_dmrNetDebug(false),
m_ysfRFRemoteAddress("127.0.0.1"),
m_ysfRFRemotePort(20011U),
m_ysfRFLocalAddress("127.0.0.1"),
m_ysfRFLocalPort(20010U),
m_ysfRFDebug(false),
m_ysfNetRemoteAddress("127.0.0.1"),
m_ysfNetRemotePort(20011U),
m_ysfNetLocalAddress("127.0.0.1"),
m_ysfNetLocalPort(20010U),
m_ysfNetDebug(false),
m_p25RFRemoteAddress("127.0.0.1"),
m_p25RFRemotePort(32010U),
m_p25RFLocalAddress("127.0.0.1"),
m_p25RFLocalPort(42020U),
m_p25RFDebug(false),
m_p25NetRemoteAddress("127.0.0.1"),
m_p25NetRemotePort(42020U),
m_p25NetLocalAddress("127.0.0.1"),
m_p25NetLocalPort(32010U),
m_p25NetDebug(false),
m_nxdnRFRemoteAddress("127.0.0.1"),
m_nxdnRFRemotePort(42021U),
m_nxdnRFLocalAddress("127.0.0.1"),
m_nxdnRFLocalPort(42022U),
m_nxdnRFDebug(false),
m_nxdnNetRemoteAddress("127.0.0.1"),
m_nxdnNetRemotePort(42022U),
m_nxdnNetLocalAddress("127.0.0.1"),
m_nxdnNetLocalPort(42021U),
m_nxdnNetDebug(false),
m_fmRFRemoteAddress("127.0.0.1"),
m_fmRFRemotePort(20011U),
m_fmRFLocalAddress("127.0.0.1"),
m_fmRFLocalPort(20010U),
m_fmRFDebug(false),
m_fmNetRemoteAddress("127.0.0.1"),
m_fmNetRemotePort(20011U),
m_fmNetLocalAddress("127.0.0.1"),
m_fmNetLocalPort(20010U),
m_fmNetDebug(false)
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

	SECTION section = SECTION::NONE;

	char buffer[BUFFER_SIZE];
	while (::fgets(buffer, BUFFER_SIZE, fp) != nullptr) {
		if (buffer[0U] == '#')
			continue;

		if (buffer[0U] == '[') {
			if (::strncmp(buffer, "[General]", 9U) == 0)
				section = SECTION::GENERAL;
			else if (::strncmp(buffer, "[Log]", 5U) == 0)
				section = SECTION::LOG;
			else if (::strncmp(buffer, "[MQTT]", 6U) == 0)
				section = SECTION::MQTT;
			else if (::strncmp(buffer, "[Transcoder]", 12U) == 0)
				section = SECTION::TRANSCODER;
			else if (::strncmp(buffer, "[Lookup]", 8U) == 0)
				section = SECTION::LOOKUP;
			else if (::strncmp(buffer, "[Info]", 6U) == 0)
				section = SECTION::INFO;
			else if (::strncmp(buffer, "[D-Star]", 8U) == 0)
				section = SECTION::DSTAR;
			else if (::strncmp(buffer, "[DMR]", 5U) == 0)
				section = SECTION::DMR;
			else if (::strncmp(buffer, "[NXDN]", 6U) == 0)
				section = SECTION::NXDN;
			else if (::strncmp(buffer, "[D-Star to D-Star]", 18U) == 0)
				section = SECTION::DSTAR_DSTAR;
			else if (::strncmp(buffer, "[D-Star to DMR]", 15U) == 0)
				section = SECTION::DSTAR_DMR;
			else if (::strncmp(buffer, "[D-Star to System Fusion]", 25U) == 0)
				section = SECTION::DSTAR_YSF;
			else if (::strncmp(buffer, "[D-Star to P25]", 15U) == 0)
				section = SECTION::DSTAR_P25;
			else if (::strncmp(buffer, "[D-Star to NXDN]", 16U) == 0)
				section = SECTION::DSTAR_NXDN;
			else if (::strncmp(buffer, "[D-Star to FM]", 14U) == 0)
				section = SECTION::DSTAR_FM;
			else if (::strncmp(buffer, "[DMR to D-Star]", 15U) == 0)
				section = SECTION::DMR_DSTAR;
			else if (::strncmp(buffer, "[DMR to DMR]", 12U) == 0)
				section = SECTION::DMR_DMR;
			else if (::strncmp(buffer, "[DMR to System Fusion]", 22U) == 0)
				section = SECTION::DMR_YSF;
			else if (::strncmp(buffer, "[DMR to P25]", 12U) == 0)
				section = SECTION::DMR_P25;
			else if (::strncmp(buffer, "[DMR to NXDN]", 13U) == 0)
				section = SECTION::DMR_NXDN;
			else if (::strncmp(buffer, "[DMR to FM]", 11U) == 0)
				section = SECTION::DMR_FM;
			else if (::strncmp(buffer, "[System Fusion to D-Star]", 25U) == 0)
				section = SECTION::YSF_DSTAR;
			else if (::strncmp(buffer, "[System Fusion to DMR]", 22U) == 0)
				section = SECTION::YSF_DMR;
			else if (::strncmp(buffer, "[System Fusion to System Fusion]", 32U) == 0)
				section = SECTION::YSF_YSF;
			else if (::strncmp(buffer, "[System Fusion to P25]", 22U) == 0)
				section = SECTION::YSF_P25;
			else if (::strncmp(buffer, "[System Fusion to NXDN]", 23U) == 0)
				section = SECTION::YSF_NXDN;
			else if (::strncmp(buffer, "[System Fusion to FM]", 21U) == 0)
				section = SECTION::YSF_FM;
			else if (::strncmp(buffer, "[P25 to D-Star]", 15U) == 0)
				section = SECTION::P25_DSTAR;
			else if (::strncmp(buffer, "[P25 to DMR]", 12U) == 0)
				section = SECTION::P25_DMR;
			else if (::strncmp(buffer, "[P25 to System Fusion]", 22U) == 0)
				section = SECTION::P25_YSF;
			else if (::strncmp(buffer, "[P25 to P25]", 12U) == 0)
				section = SECTION::P25_P25;
			else if (::strncmp(buffer, "[P25 to NXDN]", 13U) == 0)
				section = SECTION::P25_NXDN;
			else if (::strncmp(buffer, "[P25 to FM]", 11U) == 0)
				section = SECTION::P25_FM;
			else if (::strncmp(buffer, "[NXDN to D-Star]", 16U) == 0)
				section = SECTION::NXDN_DSTAR;
			else if (::strncmp(buffer, "[NXDN to DMR]", 13U) == 0)
				section = SECTION::NXDN_DMR;
			else if (::strncmp(buffer, "[NXDN to System Fusion]", 23U) == 0)
				section = SECTION::NXDN_YSF;
			else if (::strncmp(buffer, "[NXDN to P25]", 13U) == 0)
				section = SECTION::NXDN_P25;
			else if (::strncmp(buffer, "[NXDN to NXDN]", 14U) == 0)
				section = SECTION::NXDN_NXDN;
			else if (::strncmp(buffer, "[NXDN to FM]", 12U) == 0)
				section = SECTION::NXDN_FM;
			else if (::strncmp(buffer, "[FM to D-Star]", 14U) == 0)
				section = SECTION::FM_DSTAR;
			else if (::strncmp(buffer, "[FM to DMR]", 11U) == 0)
				section = SECTION::FM_DMR;
			else if (::strncmp(buffer, "[FM to System Fusion]", 21U) == 0)
				section = SECTION::FM_YSF;
			else if (::strncmp(buffer, "[FM to P25]", 11U) == 0)
				section = SECTION::FM_P25;
			else if (::strncmp(buffer, "[FM to NXDN]", 12U) == 0)
				section = SECTION::FM_NXDN;
			else if (::strncmp(buffer, "[FM to FM]", 10U) == 0)
				section = SECTION::FM_FM;
			else if (::strncmp(buffer, "[D-Star RF Network]", 19U) == 0)
				section = SECTION::DSTAR_NETWORK_RF;
			else if (::strncmp(buffer, "[D-Star Net Network]", 20U) == 0)
				section = SECTION::DSTAR_NETWORK_NET;
			else if (::strncmp(buffer, "[DMR RF Network]", 16U) == 0)
				section = SECTION::DMR_NETWORK_RF;
			else if (::strncmp(buffer, "[DMR Net Network]", 17U) == 0)
				section = SECTION::DMR_NETWORK_NET;
			else if (::strncmp(buffer, "[System Fusion RF Network]", 26U) == 0)
				section = SECTION::YSF_NETWORK_RF;
			else if (::strncmp(buffer, "[System Fusion Net Network]", 27U) == 0)
				section = SECTION::YSF_NETWORK_NET;
			else if (::strncmp(buffer, "[P25 RF Network]", 16U) == 0)
				section = SECTION::P25_NETWORK_RF;
			else if (::strncmp(buffer, "[P25 Net Network]", 17U) == 0)
				section = SECTION::P25_NETWORK_NET;
			else if (::strncmp(buffer, "[NXDN RF Network]", 17U) == 0)
				section = SECTION::NXDN_NETWORK_RF;
			else if (::strncmp(buffer, "[NXDN Net Network]", 18U) == 0)
				section = SECTION::NXDN_NETWORK_NET;
			else if (::strncmp(buffer, "[FM RF Network]", 15U) == 0)
				section = SECTION::FM_NETWORK_RF;
			else if (::strncmp(buffer, "[FM Net Network]", 16U) == 0)
				section = SECTION::FM_NETWORK_NET;
			else
				section = SECTION::NONE;

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

		if (section == SECTION::GENERAL) {
			if (::strcmp(key, "Callsign") == 0)
				m_callsign = value;
			else if (::strcmp(key, "RFModeHang") == 0)
				m_rfModeHang = (unsigned int)::atoi(value);
			else if (::strcmp(key, "NetModeHang") == 0)
				m_netModeHang = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Daemon") == 0)
				m_daemon = ::atoi(value) == 1;
		} else if (section == SECTION::LOG) {
			if (::strcmp(key, "DisplayLevel") == 0)
				m_logDisplayLevel = (unsigned int)::atoi(value);
			else if (::strcmp(key, "MQTTLevel") == 0)
				m_logMQTTLevel = (unsigned int)::atoi(value);
		} else if (section == SECTION::MQTT) {
			if (::strcmp(key, "Address") == 0)
				m_mqttAddress = value;
			else if (::strcmp(key, "Port") == 0)
				m_mqttPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Keepalive") == 0)
				m_mqttKeepalive = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Name") == 0)
				m_mqttName = value;
			else if (::strcmp(key, "Auth") == 0)
				m_mqttAuthEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Username") == 0)
				m_mqttUsername = value;
			else if (::strcmp(key, "Password") == 0)
				m_mqttPassword = value;
		} else if (section == SECTION::TRANSCODER) {
			if (::strcmp(key, "Protocol") == 0)
				m_transcoderProtocol = value;
			else if (::strcmp(key, "UARTPort") == 0)
				m_transcoderUARTPort = value;
			else if (::strcmp(key, "UARTSpeed") == 0)
				m_transcoderUARTSpeed = uint32_t(::atoi(value));
			else if (::strcmp(key, "RemoteAddress") == 0)
				m_transcoderRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_transcoderRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_transcoderLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_transcoderLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_transcoderDebug = ::atoi(value) == 1;
		} else if (section == SECTION::LOOKUP) {
			if (::strcmp(key, "DMRLookup") == 0)
				m_dmrLookupFile = value;
			else if (::strcmp(key, "NXDNLookup") == 0)
				m_nxdnLookupFile = value;
			else if (::strcmp(key, "ReloadTime") == 0)
				m_reloadTime = (unsigned int)::atoi(value);
		} else if (section == SECTION::INFO) {
			if (::strcmp(key, "TXFrequency") == 0)
				m_infoTXFrequency = uint32_t(::atoi(value));
			else if (::strcmp(key, "RXFrequency") == 0)
				m_infoRXFrequency = uint32_t(::atoi(value));
			else if (::strcmp(key, "ColorCode") == 0)
				m_infoColorCode = uint8_t(::atoi(value));
			else if (::strcmp(key, "Power") == 0)
				m_infoPower = uint16_t(::atoi(value));
		} else if (section == SECTION::DSTAR) {
			if (::strcmp(key, "Module") == 0)
				m_dStarModule = value;
		} else if (section == SECTION::DMR) {
			if (::strcmp(key, "Id") == 0)
				m_dmrId = uint32_t(::atoi(value));
		} else if (section == SECTION::NXDN) {
			if (::strcmp(key, "Id") == 0)
				m_nxdnId = uint16_t(::atoi(value));
		} else if (section == SECTION::DSTAR_DSTAR) {
			if (::strcmp(key, "Enable") == 0) {
				m_dstarDStarEnable = ::atoi(value) == 1;
#if defined(TRACE_CONFIG)
				if (m_dstarDStarEnable)
					::fprintf(stdout, "D-Star => D-Star, mapping remainder\n");
#endif
			}
		} else if (section == SECTION::DSTAR_DMR) {
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
				if (m_dstarDMREnable)
					::fprintf(stdout, "D-Star => DMR, mapping \"%s\" to %u:TG%u\n", dest.c_str(), slotTG.first, slotTG.second);
#endif
				m_dstarDMRDests.push_back(std::tuple<std::string, uint8_t, uint32_t>(dest, slotTG.first, slotTG.second));
			}
		} else if (section == SECTION::DSTAR_YSF) {
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
				if (m_dstarYSFEnable)
					::fprintf(stdout, "D-Star => YSF, mapping \"%s\" to %u\n", dest.c_str(), dgid);
#endif
				m_dstarYSFDests.push_back(std::pair<std::string, uint8_t>(dest, dgid));
			}
		} else if (section == SECTION::DSTAR_P25) {
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
				if (m_dstarP25Enable)
					::fprintf(stdout, "D-Star => P25, mapping \"%s\" to TG%u\n", dest.c_str(), tgid);
#endif
				m_dstarP25Dests.push_back(std::pair<std::string, uint32_t>(dest, tgid));
			}
		} else if (section == SECTION::DSTAR_NXDN) {
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
				if (m_dstarNXDNEnable)
					::fprintf(stdout, "D-Star => NXDN, mapping \"%s\" to TG%u\n", dest.c_str(), tgid);
#endif
				m_dstarNXDNDests.push_back(std::pair<std::string, uint16_t>(dest, tgid));
			}
		} else if (section == SECTION::DSTAR_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_dstarFMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "Dest") == 0) {
				std::string dest = getString(value);
				if (dest.empty())
					continue;
#if defined(TRACE_CONFIG)
				if (m_dstarFMEnable)
					::fprintf(stdout, "D-Star => FM, mapping \"%s\"\n", dest.c_str());
#endif
				m_dstarFMDest = dest;
			}
		} else if (section == SECTION::DMR_DSTAR) {
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
				if (m_dmrDStarEnable)
					::fprintf(stdout, "DMR => D-Star, mapping %u:TG%u to \"%s\"\n", slotTG.first, slotTG.second, dest.c_str());
#endif
				m_dmrDStarTGs.push_back(std::tuple<uint8_t, uint32_t, std::string>(slotTG.first, slotTG.second, dest));
			}
		} else if (section == SECTION::DMR_DMR) {
			if (::strcmp(key, "Enable1") == 0) {
				m_dmrDMREnable1 = ::atoi(value) == 1;
#if defined(TRACE_CONFIG)
				if (m_dmrDMREnable1)
					::fprintf(stdout, "DMR => DMR, mapping slot 1 remainder\n");
#endif
			}
			if (::strcmp(key, "Enable2") == 0) {
				m_dmrDMREnable2 = ::atoi(value) == 1;
#if defined(TRACE_CONFIG)
				if (m_dmrDMREnable2)
					::fprintf(stdout, "DMR => DMR, mapping slot 2 remainder\n");
#endif
			}
		} else if (section == SECTION::DMR_YSF) {
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
				if (m_dmrYSFEnable)
					::fprintf(stdout, "DMR => YSF, mapping %u:TG%u to %u\n", slotTG.first, slotTG.second, dgId);
#endif
				m_dmrYSFTGs.push_back(std::tuple<uint8_t, uint32_t, uint8_t>(slotTG.first, slotTG.second, dgId));
			}
		} else if (section == SECTION::DMR_P25) {
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
				if (m_dmrP25Enable)
					::fprintf(stdout, "DMR => P25, mapping %u:TG%u to TG%u\n", slotTG.first, slotTG.second, tgId);
#endif
				m_dmrP25TGs.push_back(std::tuple<uint8_t, uint32_t, uint32_t>(slotTG.first, slotTG.second, tgId));
			}
		} else if (section == SECTION::DMR_NXDN) {
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
				if (m_dmrNXDNEnable)
					::fprintf(stdout, "DMR => NXDN, mapping %u:TG%u to TG%u\n", slotTG.first, slotTG.second, tgId);
#endif
				m_dmrNXDNTGs.push_back(std::tuple<uint8_t, uint32_t, uint16_t>(slotTG.first, slotTG.second, tgId));
			}
		} else if (section == SECTION::DMR_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_dmrFMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				std::pair<uint8_t, uint32_t> slotTG = getSlotTG(value);
				if ((slotTG.first == NULL_SLOT) || (slotTG.second == NULL_ID32))
					continue;

#if defined(TRACE_CONFIG)
				if (m_dmrFMEnable)
					::fprintf(stdout, "DMR => FM, mapping %u:TG%u\n", slotTG.first, slotTG.second);
#endif
				m_dmrFMTG = std::make_pair(slotTG.first, slotTG.second);
			}
		} else if (section == SECTION::YSF_DSTAR) {
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
				if (m_ysfDStarEnable)
					::fprintf(stdout, "YSF => D-Star, mapping %u to \"%s\"\n", dgId, dest.c_str());
#endif
				m_ysfDStarDGIds.push_back(std::pair<uint8_t, std::string>(dgId, dest));
			}
		} else if (section == SECTION::YSF_DMR) {
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
				if (m_ysfDMREnable)
					::fprintf(stdout, "YSF => DMR, mapping %u to %u:TG%u\n", dgId, slotTG.first, slotTG.second);
#endif
				m_ysfDMRDGIds.push_back(std::tuple<uint8_t, uint8_t, uint32_t>(dgId, slotTG.first, slotTG.second));
			}
		} else if (section == SECTION::YSF_YSF) {
			if (::strcmp(key, "Enable") == 0) {
				m_ysfYSFEnable = ::atoi(value) == 1;
#if defined(TRACE_CONFIG)
				if (m_ysfYSFEnable)
					::fprintf(stdout, "YSF => YSF, mapping remainder\n");
#endif
			}
		} else if (section == SECTION::YSF_P25) {
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
				if (m_ysfP25Enable)
					::fprintf(stdout, "YSF => P25, mapping %u to TG%u\n", dgId, tgid);
#endif
				m_ysfP25DGIds.push_back(std::pair<uint8_t, uint32_t>(dgId, tgid));
			}
		} else if (section == SECTION::YSF_NXDN) {
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
				if (m_ysfNXDNEnable)
					::fprintf(stdout, "YSF => NXDN, mapping %u to TG%u\n", dgId, tgid);
#endif
				m_ysfNXDNDGIds.push_back(std::pair<uint8_t, uint16_t>(dgId, tgid));
			}
		} else if (section == SECTION::YSF_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_ysfFMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "DGId") == 0) {
				m_ysfFMDGId = uint8_t(::atoi(value));
#if defined(TRACE_CONFIG)
				if (m_ysfFMEnable)
					::fprintf(stdout, "YSF => FM, mapping %u\n", m_ysfFMDGId);
#endif
			}
		} else if (section == SECTION::P25_DSTAR) {
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
				if (m_p25DStarEnable)
					::fprintf(stdout, "P25 => D-Star, mapping TG%u to \"%s\"\n", tg, dest.c_str());
#endif
				m_p25DStarTGs.push_back(std::pair<uint32_t, std::string>(tg, dest));
			}
		} else if (section == SECTION::P25_DMR) {
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
				if (m_p25DMREnable)
					::fprintf(stdout, "P25 => DMR, mapping TG%u to %u:TG%u\n", tg, slotTG.first, slotTG.second);
#endif
				m_p25DMRTGs.push_back(std::tuple<uint32_t, uint8_t, uint32_t>(tg, slotTG.first, slotTG.second));
			}
		} else if (section == SECTION::P25_YSF) {
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
				if (m_p25YSFEnable)
					::fprintf(stdout, "P25 => YSF, mapping TG%u to %u\n", tg, dgId);
#endif
				m_p25YSFTGs.push_back(std::pair<uint32_t, uint8_t>(tg, dgId));
			}
		} else if (section == SECTION::P25_P25) {
			if (::strcmp(key, "Enable") == 0) {
				m_p25P25Enable = ::atoi(value) == 1;
#if defined(TRACE_CONFIG)
				if (m_p25P25Enable)
					::fprintf(stdout, "P25 => P25, mapping remainder\n");
#endif
			}
		} else if (section == SECTION::P25_NXDN) {
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
				if (m_p25NXDNEnable)
					::fprintf(stdout, "P25 => NXDN, mapping TG%u to TG%u\n", tg1, tg2);
#endif
				m_p25NXDNTGs.push_back(std::pair<uint32_t, uint16_t>(tg1, tg2));
			}
		} else if (section == SECTION::P25_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_p25FMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				m_p25FMTG = uint32_t(::atoi(value));
#if defined(TRACE_CONFIG)
				if (m_p25FMEnable)
					::fprintf(stdout, "P25 => FM, mapping TG%u\n", m_p25FMTG);
#endif
			}
		} else if (section == SECTION::NXDN_DSTAR) {
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
				if (m_nxdnDStarEnable)
					::fprintf(stdout, "NXDN => D-Star, mapping TG%u to \"%s\"\n", tg, dest.c_str());
#endif
				m_nxdnDStarTGs.push_back(std::pair<uint16_t, std::string>(tg, dest));
			}
		} else if (section == SECTION::NXDN_DMR) {
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
				if (m_nxdnDMREnable)
					::fprintf(stdout, "NXDN => DMR, mapping TG%u to %u:TG%u\n", tg, slotTG.first, slotTG.second);
#endif
				m_nxdnDMRTGs.push_back(std::tuple<uint16_t, uint8_t, uint32_t>(tg, slotTG.first, slotTG.second));
			}
		} else if (section == SECTION::NXDN_YSF) {
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
				if (m_nxdnYSFEnable)
					::fprintf(stdout, "NXDN => YSF, mapping TG%u to %u\n", tg, dgid);
#endif
				m_nxdnYSFTGs.push_back(std::pair<uint16_t, uint8_t>(tg, dgid));
			}
		} else if (section == SECTION::NXDN_P25) {
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
				if (m_nxdnP25Enable)
					::fprintf(stdout, "NXDN => P25, mapping TG%u to TG%u\n", tg1, tg2);
#endif
				m_nxdnP25TGs.push_back(std::pair<uint16_t, uint32_t>(tg1, tg2));
			}
		} else if (section == SECTION::NXDN_NXDN) {
			if (::strcmp(key, "Enable") == 0) {
				m_nxdnNXDNEnable = ::atoi(value) == 1;
#if defined(TRACE_CONFIG)
				if (m_nxdnNXDNEnable)
					::fprintf(stdout, "NXDN => NXDN, mapping remainder\n");
#endif
			}
		} else if (section == SECTION::NXDN_FM) {
			if (::strcmp(key, "Enable") == 0) {
				m_nxdnFMEnable = ::atoi(value) == 1;
			} else if (::strcmp(key, "TG") == 0) {
				m_nxdnFMTG = uint16_t(::atoi(value));
#if defined(TRACE_CONFIG)
				if (m_nxdnFMEnable)
					::fprintf(stdout, "NXDN => FM, mapping TG%u\n", m_nxdnFMTG);
#endif
			}
		} else if (section == SECTION::FM_DSTAR) {
			if (::strcmp(key, "Enable") == 0)
				m_fmDStarEnable = ::atoi(value) == 1;
		} else if (section == SECTION::FM_DMR) {
			if (::strcmp(key, "Enable") == 0)
				m_fmDMREnable = ::atoi(value) == 1;
		} else if (section == SECTION::FM_YSF) {
			if (::strcmp(key, "Enable") == 0)
				m_fmYSFEnable = ::atoi(value) == 1;
		} else if (section == SECTION::FM_P25) {
			if (::strcmp(key, "Enable") == 0)
				m_fmP25Enable = ::atoi(value) == 1;
		} else if (section == SECTION::FM_NXDN) {
			if (::strcmp(key, "Enable") == 0)
				m_fmNXDNEnable = ::atoi(value) == 1;
		} else if (section == SECTION::FM_FM) {
			if (::strcmp(key, "Enable") == 0)
				m_fmFMEnable = ::atoi(value) == 1;
		} else if (section == SECTION::DSTAR_NETWORK_RF) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_dStarRFRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_dStarRFRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dStarRFLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dStarRFLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dStarRFDebug = ::atoi(value) == 1;
		} else if (section == SECTION::DSTAR_NETWORK_NET) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_dStarNetRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_dStarNetRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dStarNetLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dStarNetLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dStarNetDebug = ::atoi(value) == 1;
		} else if (section == SECTION::DMR_NETWORK_RF) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_dmrRFRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_dmrRFRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dmrRFLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dmrRFLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dmrRFDebug = ::atoi(value) == 1;
		} else if (section == SECTION::DMR_NETWORK_NET) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_dmrNetRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_dmrNetRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dmrNetLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dmrNetLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetDebug = ::atoi(value) == 1;
		} else if (section == SECTION::YSF_NETWORK_RF) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_ysfRFRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_ysfRFRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_ysfRFLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_ysfRFLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_ysfRFDebug = ::atoi(value) == 1;
		} else if (section == SECTION::YSF_NETWORK_NET) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_ysfNetRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_ysfNetRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_ysfNetLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_ysfNetLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_ysfNetDebug = ::atoi(value) == 1;
		} else if (section == SECTION::P25_NETWORK_RF) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_p25RFRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_p25RFRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_p25RFLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_p25RFLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_p25RFDebug = ::atoi(value) == 1;
		} else if (section == SECTION::P25_NETWORK_NET) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_p25NetRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_p25NetRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_p25NetLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_p25NetLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_p25NetDebug = ::atoi(value) == 1;
		} else if (section == SECTION::NXDN_NETWORK_RF) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_nxdnRFRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_nxdnRFRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_nxdnRFLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_nxdnRFLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_nxdnRFDebug = ::atoi(value) == 1;
		} else if (section == SECTION::NXDN_NETWORK_NET) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_nxdnNetRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_nxdnNetRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_nxdnNetLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_nxdnNetLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_nxdnNetDebug = ::atoi(value) == 1;
		} else if (section == SECTION::FM_NETWORK_RF) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_fmRFRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_fmRFRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_fmRFLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_fmRFLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_fmRFDebug = ::atoi(value) == 1;
		} else if (section == SECTION::FM_NETWORK_NET) {
			if (::strcmp(key, "RemoteAddress") == 0)
				m_fmNetRemoteAddress = value;
			else if (::strcmp(key, "RemotePort") == 0)
				m_fmNetRemotePort = uint16_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_fmNetLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_fmNetLocalPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_fmNetDebug = ::atoi(value) == 1;
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

unsigned int CConf::getLogDisplayLevel() const
{
	return m_logDisplayLevel;
}

unsigned int CConf::getLogMQTTLevel() const
{
	return m_logMQTTLevel;
}

std::string CConf::getMQTTAddress() const
{
	return m_mqttAddress;
}

uint16_t CConf::getMQTTPort() const
{
	return m_mqttPort;
}

unsigned int CConf::getMQTTKeepalive() const
{
	return m_mqttKeepalive;
}

std::string CConf::getMQTTName() const
{
	return m_mqttName;
}

bool CConf::getMQTTAuthEnabled() const
{
	return m_mqttAuthEnabled;
}

std::string CConf::getMQTTUsername() const
{
	return m_mqttUsername;
}

std::string CConf::getMQTTPassword() const
{
	return m_mqttPassword;
}

std::string CConf::getTranscoderProtocol() const
{
	return m_transcoderProtocol;
}

std::string CConf::getTranscoderUARTPort() const
{
	return m_transcoderUARTPort;
}

uint32_t CConf::getTranscoderUARTSpeed() const
{
	return m_transcoderUARTSpeed;
}

std::string CConf::getTranscoderRemoteAddress() const
{
	return m_transcoderRemoteAddress;
}

uint16_t CConf::getTranscoderRemotePort() const
{
	return m_transcoderRemotePort;
}

std::string CConf::getTranscoderLocalAddress() const
{
	return m_transcoderLocalAddress;
}

uint16_t CConf::getTranscoderLocalPort() const
{
	return m_transcoderLocalPort;
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

uint32_t CConf::getInfoTXFrequency() const
{
	return m_infoTXFrequency;
}

uint32_t CConf::getInfoRXFrequency() const
{
	return m_infoRXFrequency;
}

uint8_t CConf::getInfoColorCode() const
{
	return m_infoColorCode;
}

uint16_t CConf::getInfoPower() const
{
	return m_infoPower;
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

std::string CConf::getDStarRFRemoteAddress() const
{
	return m_dStarRFRemoteAddress;
}

uint16_t CConf::getDStarRFRemotePort() const
{
	return m_dStarRFRemotePort;
}

std::string CConf::getDStarRFLocalAddress() const
{
	return m_dStarRFLocalAddress;
}

uint16_t CConf::getDStarRFLocalPort() const
{
	return m_dStarRFLocalPort;
}

bool CConf::getDStarRFDebug() const
{
	return m_dStarRFDebug;
}

std::string CConf::getDStarNetRemoteAddress() const
{
	return m_dStarNetRemoteAddress;
}

uint16_t CConf::getDStarNetRemotePort() const
{
	return m_dStarNetRemotePort;
}

std::string CConf::getDStarNetLocalAddress() const
{
	return m_dStarNetLocalAddress;
}

uint16_t CConf::getDStarNetLocalPort() const
{
	return m_dStarNetLocalPort;
}

bool CConf::getDStarNetDebug() const
{
	return m_dStarNetDebug;
}

std::string CConf::getDMRRFRemoteAddress() const
{
	return m_dmrRFRemoteAddress;
}

uint16_t CConf::getDMRRFRemotePort() const
{
	return m_dmrRFRemotePort;
}

std::string CConf::getDMRRFLocalAddress() const
{
	return m_dmrRFLocalAddress;
}

uint16_t CConf::getDMRRFLocalPort() const
{
	return m_dmrRFLocalPort;
}

bool CConf::getDMRRFDebug() const
{
	return m_dmrRFDebug;
}

std::string CConf::getDMRNetRemoteAddress() const
{
	return m_dmrNetRemoteAddress;
}

uint16_t CConf::getDMRNetRemotePort() const
{
	return m_dmrNetRemotePort;
}

std::string CConf::getDMRNetLocalAddress() const
{
	return m_dmrNetLocalAddress;
}

uint16_t CConf::getDMRNetLocalPort() const
{
	return m_dmrNetLocalPort;
}

bool CConf::getDMRNetDebug() const
{
	return m_dmrNetDebug;
}

std::string CConf::getYSFRFRemoteAddress() const
{
	return m_ysfRFRemoteAddress;
}

uint16_t CConf::getYSFRFRemotePort() const
{
	return m_ysfRFRemotePort;
}

std::string CConf::getYSFRFLocalAddress() const
{
	return m_ysfRFLocalAddress;
}

uint16_t CConf::getYSFRFLocalPort() const
{
	return m_ysfRFLocalPort;
}

bool CConf::getYSFRFDebug() const
{
	return m_ysfRFDebug;
}

std::string CConf::getYSFNetRemoteAddress() const
{
	return m_ysfNetRemoteAddress;
}

uint16_t CConf::getYSFNetRemotePort() const
{
	return m_ysfNetRemotePort;
}

std::string CConf::getYSFNetLocalAddress() const
{
	return m_ysfNetLocalAddress;
}

uint16_t CConf::getYSFNetLocalPort() const
{
	return m_ysfNetLocalPort;
}

bool CConf::getYSFNetDebug() const
{
	return m_ysfNetDebug;
}

std::string CConf::getP25RFRemoteAddress() const
{
	return m_p25RFRemoteAddress;
}

uint16_t CConf::getP25RFRemotePort() const
{
	return m_p25RFRemotePort;
}

std::string CConf::getP25RFLocalAddress() const
{
	return m_p25RFLocalAddress;
}

uint16_t CConf::getP25RFLocalPort() const
{
	return m_p25RFLocalPort;
}

bool CConf::getP25RFDebug() const
{
	return m_p25RFDebug;
}

std::string CConf::getP25NetRemoteAddress() const
{
	return m_p25NetRemoteAddress;
}

uint16_t CConf::getP25NetRemotePort() const
{
	return m_p25NetRemotePort;
}

std::string CConf::getP25NetLocalAddress() const
{
	return m_p25NetLocalAddress;
}

uint16_t CConf::getP25NetLocalPort() const
{
	return m_p25NetLocalPort;
}

bool CConf::getP25NetDebug() const
{
	return m_p25NetDebug;
}

std::string CConf::getNXDNRFRemoteAddress() const
{
	return m_nxdnRFRemoteAddress;
}

uint16_t CConf::getNXDNRFRemotePort() const
{
	return m_nxdnRFRemotePort;
}

std::string CConf::getNXDNRFLocalAddress() const
{
	return m_nxdnRFLocalAddress;
}

uint16_t CConf::getNXDNRFLocalPort() const
{
	return m_nxdnRFLocalPort;
}

bool CConf::getNXDNRFDebug() const
{
	return m_nxdnRFDebug;
}

std::string CConf::getNXDNNetRemoteAddress() const
{
	return m_nxdnNetRemoteAddress;
}

uint16_t CConf::getNXDNNetRemotePort() const
{
	return m_nxdnNetRemotePort;
}

std::string CConf::getNXDNNetLocalAddress() const
{
	return m_nxdnNetLocalAddress;
}

uint16_t CConf::getNXDNNetLocalPort() const
{
	return m_nxdnNetLocalPort;
}

bool CConf::getNXDNNetDebug() const
{
	return m_nxdnNetDebug;
}

std::string CConf::getFMRFRemoteAddress() const
{
	return m_fmRFRemoteAddress;
}

uint16_t CConf::getFMRFRemotePort() const
{
	return m_fmRFRemotePort;
}

std::string CConf::getFMRFLocalAddress() const
{
	return m_fmRFLocalAddress;
}

uint16_t CConf::getFMRFLocalPort() const
{
	return m_fmRFLocalPort;
}

bool CConf::getFMRFDebug() const
{
	return m_fmRFDebug;
}

std::string CConf::getFMNetRemoteAddress() const
{
	return m_fmNetRemoteAddress;
}

uint16_t CConf::getFMNetRemotePort() const
{
	return m_fmNetRemotePort;
}

std::string CConf::getFMNetLocalAddress() const
{
	return m_fmNetLocalAddress;
}

uint16_t CConf::getFMNetLocalPort() const
{
	return m_fmNetLocalPort;
}

bool CConf::getFMNetDebug() const
{
	return m_fmNetDebug;
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
