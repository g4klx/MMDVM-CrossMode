/*
 *   Copyright (C) 2015,2016,2017,2024 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#if !defined(CONF_H)
#define	CONF_H

#include <string>
#include <vector>

class CConf
{
public:
	CConf(const std::string& file);
	~CConf();

	bool read();

	// The General section
	bool         getDaemon() const;

	// The Transcoder section
	std::string  getTranscoderPort() const;
	uint32_t     getTranscoderSpeed() const;

	// The D-Star Network section
	std::string  getDStarDestAddress() const;
	uint16_t     getDStarDestPort() const;
	std::string  getDStarLocalAddress() const;
	uint16_t     getDStarLocalPort() const;
	bool         getDStarDebug() const;

	// The M17 Network section
	std::string  getM17DestAddress() const;
	uint16_t     getM17DestPort() const;
	std::string  getM17LocalAddress() const;
	uint16_t     getM17LocalPort() const;
	bool         getM17Debug() const;

	// The Log section
	uint32_t     getLogDisplayLevel() const;
	uint32_t     getLogFileLevel() const;
	std::string  getLogFilePath() const;
	std::string  getLogFileRoot() const;
	bool         getLogFileRotate() const;

private:
	std::string  m_file;
	bool         m_daemon;

	std::string  m_transcoderPort;
	uint32_t     m_transcoderSpeed;

	std::string  m_dStarDestAddress;
	uint16_t     m_dStarDestPort;
	std::string  m_dStarLocalAddress;
	uint16_t     m_dStarLocalPort;
	bool         m_dStarDebug;

	std::string  m_m17DestAddress;
	uint16_t     m_m17DestPort;
	std::string  m_m17LocalAddress;
	uint16_t     m_m17LocalPort;
	bool         m_m17Debug;

	uint32_t     m_logDisplayLevel;
	uint32_t     m_logFileLevel;
	std::string  m_logFilePath;
	std::string  m_logFileRoot;
	bool         m_logFileRotate;
};

#endif
