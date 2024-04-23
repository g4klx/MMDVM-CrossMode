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
#include "Log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

const int BUFFER_SIZE = 500;

enum SECTION {
	SECTION_NONE,
	SECTION_GENERAL,
	SECTION_TRANSCODER,
	SECTION_DSTAR_NETWORK,
	SECTION_M17_NETWORK,
	SECTION_LOG
};

CConf::CConf(const std::string& file) :
m_file(file),
m_daemon(false),
m_transcoderPort(),
m_transcoderSpeed(460800U),
m_dStarDestAddress("127.0.0.1"),
m_dStarDestPort(20011U),
m_dStarLocalAddress(),
m_dStarLocalPort(20010U),
m_dStarDebug(false),
m_m17DestAddress("127.0.0.1"),
m_m17DestPort(17011U),
m_m17LocalAddress(),
m_m17LocalPort(17010U),
m_m17Debug(false),
m_logDisplayLevel(0U),
m_logFileLevel(0U),
m_logFilePath(),
m_logFileRoot(),
m_logFileRotate(true)
{
}

CConf::~CConf()
{
}

bool CConf::read()
{
	FILE* fp = ::fopen(m_file.c_str(), "rt");
	if (fp == NULL) {
		::fprintf(stderr, "Couldn't open the .ini file - %s\n", m_file.c_str());
		return false;
	}

	SECTION section = SECTION_NONE;

	char buffer[BUFFER_SIZE];
	while (::fgets(buffer, BUFFER_SIZE, fp) != NULL) {
		if (buffer[0U] == '#')
			continue;

		if (buffer[0U] == '[') {
			if (::strncmp(buffer, "[General]", 9U) == 0)
				section = SECTION_GENERAL;
			else if (::strncmp(buffer, "[Transcoder]", 12U) == 0)
				section = SECTION_TRANSCODER;
			else if (::strncmp(buffer, "[D-Star Network]", 16U) == 0)
				section = SECTION_DSTAR_NETWORK;
			else if (::strncmp(buffer, "[M17 Network]", 13U) == 0)
					section = SECTION_M17_NETWORK;
			else if (::strncmp(buffer, "[Log]", 5U) == 0)
				section = SECTION_LOG;
			else
				section = SECTION_NONE;

			continue;
		}

		char* key = ::strtok(buffer, " \t=\r\n");
		if (key == NULL)
			continue;

		char* value = ::strtok(NULL, "\r\n");
		if (value == NULL)
			continue;

		// Remove quotes from the value
		size_t len = ::strlen(value);
		if (len > 1U && *value == '"' && value[len - 1U] == '"') {
			value[len - 1U] = '\0';
			value++;
		}

		if (section == SECTION_GENERAL) {
			if (::strcmp(key, "Daemon") == 0)
				m_daemon = ::atoi(value) == 1;
		} else if (section == SECTION_TRANSCODER) {
			if (::strcmp(key, "Port") == 0)
				m_transcoderPort = value;
			else if (::strcmp(key, "Speed") == 0)
				m_transcoderSpeed = uint32_t(::atoi(value));
		} else if (section == SECTION_DSTAR_NETWORK) {
			if (::strcmp(key, "DestAddress") == 0)
				m_dStarDestAddress = value;
			else if (::strcmp(key, "DestPort") == 0)
				m_dStarDestPort = uint32_t(::atoi(value));
			else if (::strcmp(key, "LocalAddress") == 0)
				m_dStarLocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_dStarLocalPort = uint32_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_dStarDebug = ::atoi(value) == 1;
		} else if (section == SECTION_M17_NETWORK) {
			if (::strcmp(key, "DestAddress") == 0)
				m_m17DestAddress = value;
			else if (::strcmp(key, "DestPort") == 0)
				m_m17DestPort = (uint32_t)::atoi(value);
			else if (::strcmp(key, "LocalAddress") == 0)
				m_m17LocalAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_m17LocalPort = uint32_t(::atoi(value));
			else if (::strcmp(key, "Debug") == 0)
				m_m17Debug = ::atoi(value) == 1;
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
		}
	}

	::fclose(fp);

	return true;
}

bool CConf::getDaemon() const
{
	return m_daemon;
}

std::string CConf::getTranscoderPort() const
{
	return m_transcoderPort;
}

uint32_t CConf::getTranscoderSpeed() const
{
	return m_transcoderSpeed;
}

std::string CConf::getDStarDestAddress() const
{
	return m_dStarDestAddress;
}

uint16_t CConf::getDStarDestPort() const
{
	return m_dStarDestPort;
}

std::string CConf::getDStarLocalAddress() const
{
	return m_dStarLocalAddress;
}

uint16_t CConf::getDStarLocalPort() const
{
	return m_dStarLocalPort;
}

bool CConf::getDStarDebug() const
{
	return m_dStarDebug;
}

std::string CConf::getM17DestAddress() const
{
	return m_m17DestAddress;
}

uint16_t CConf::getM17DestPort() const
{
	return m_m17DestPort;
}

std::string CConf::getM17LocalAddress() const
{
	return m_m17LocalAddress;
}

uint16_t CConf::getM17LocalPort() const
{
	return m_m17LocalPort;
}

bool CConf::getM17Debug() const
{
	return m_m17Debug;
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
