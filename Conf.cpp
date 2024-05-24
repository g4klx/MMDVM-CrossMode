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
#include <utility>

const int BUFFER_SIZE = 500;

enum SECTION {
	SECTION_NONE,
	SECTION_GENERAL,
	SECTION_LOG,
	SECTION_MQTT,
	SECTION_TRANSCODER,
	SECTION_DSTAR,
	SECTION_YSF_M17,
	SECTION_DSTAR_NETWORK_FROM,
	SECTION_DSTAR_NETWORK_TO,
	SECTION_YSF_NETWORK_FROM,
	SECTION_YSF_NETWORK_TO,
	SECTION_FM_NETWORK_FROM,
	SECTION_FM_NETWORK_TO,
	SECTION_M17_NETWORK_FROM,
	SECTION_M17_NETWORK_TO
};

CConf::CConf(const std::string& file) :
m_file(file),
m_defaultCallsign("G9BF"),
m_defaultDMRId(12345U),
m_defaultNXDNId(1234U),
m_daemon(false),
m_logDisplayLevel(0U),
m_logMQTTLevel(0U),
m_mqttAddress("127.0.0.1"),
m_mqttPort(1883U),
m_mqttKeepalive(60U),
m_mqttName("mmdvm-crossmode"),
m_transcoderPort(),
m_transcoderSpeed(460800U),
m_transcoderDebug(false),
m_dStarCallsign("G9BF   B"),
m_ysfM17Mapping(),
m_m17YSFMapping(),
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
			else if (::strncmp(buffer, "[MQTT]", 6U) == 0)
				section = SECTION_MQTT;
			else if (::strncmp(buffer, "[Transcoder]", 12U) == 0)
				section = SECTION_TRANSCODER;
			else if (::strncmp(buffer, "[D-Star]", 8U) == 0)
				section = SECTION_DSTAR;
			else if (::strncmp(buffer, "[System Fusion to M17]", 22U) == 0)
				section = SECTION_YSF_M17;
			else if (::strncmp(buffer, "[D-Star Network From]", 21U) == 0)
				section = SECTION_DSTAR_NETWORK_FROM;
			else if (::strncmp(buffer, "[D-Star Network To]", 19U) == 0)
				section = SECTION_DSTAR_NETWORK_TO;
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
			if (::strcmp(key, "DefaultCallsign") == 0)
				m_defaultCallsign = value;
			else if (::strcmp(key, "DefaultDMRId") == 0)
				m_defaultDMRId = uint32_t(::atoi(value));
			else if (::strcmp(key, "DefaultNXDNId") == 0)
				m_defaultNXDNId = uint16_t(::atoi(value));
			else if (::strcmp(key, "Daemon") == 0)
				m_daemon = ::atoi(value) == 1;
		} else if (section == SECTION_LOG) {
			if (::strcmp(key, "DisplayLevel") == 0)
				m_logDisplayLevel = (unsigned int)::atoi(value);
			else if (::strcmp(key, "MQTTLevel") == 0)
				m_logMQTTLevel = (unsigned int)::atoi(value);
		} else if (section == SECTION_MQTT) {
			if (::strcmp(key, "Address") == 0)
				m_mqttAddress = value;
			else if (::strcmp(key, "Port") == 0)
				m_mqttPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Keepalive") == 0)
				m_mqttKeepalive = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Name") == 0)
				m_mqttName = value;
		} else if (section == SECTION_DSTAR) {
			if (::strcmp(key, "RepeaterCallsign") == 0)
				m_dStarCallsign = value;
		} else if (section == SECTION_YSF_M17) {
			if (::strcmp(key, "DGId") == 0) {
				char* p = ::strchr(value, '=');
				if (p == nullptr)
					break;
				*p = '\0';
				uint8_t dgId = uint8_t(::atoi(value));

				p++;
				size_t len = ::strlen(p);
				if (len > 1U && *p == '"' && p[len - 1U] == '"') {
					p[len - 1U] = '\0';
					p++;
				}
				std::string text = std::string(p);

				m_ysfM17Mapping.insert(std::pair<uint8_t, std::string>(dgId, text));
				m_m17YSFMapping.insert(std::pair<std::string, uint8_t>(text, dgId));
			}
		} else if (section == SECTION_TRANSCODER) {
			if (::strcmp(key, "Port") == 0)
				m_transcoderPort = value;
			else if (::strcmp(key, "Speed") == 0)
				m_transcoderSpeed = uint32_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_transcoderDebug = ::atoi(value) == 1;
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

std::string CConf::getDefaultCallsign() const
{
	return m_defaultCallsign;
}

uint32_t CConf::getDefaultDMRId() const
{
	return m_defaultDMRId;
}

uint16_t CConf::getDefaultNXDNId() const
{
	return m_defaultNXDNId;
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

std::string CConf::getDStarCallsign() const
{
	return m_dStarCallsign;
}

std::map<uint8_t, std::string> CConf::getYSFM17Mapping() const
{
	return m_ysfM17Mapping;
}

std::map<std::string, uint8_t> CConf::getM17YSFMapping() const
{
	return m_m17YSFMapping;
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

