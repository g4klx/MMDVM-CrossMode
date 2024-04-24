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
	std::string  getDefaultCallsign() const;
	uint32_t     getDefaultDMRId() const;
	uint16_t     getDefaultNXDNId() const;
	bool         getDaemon() const;

	// The Transcoder section
	std::string  getTranscoderPort() const;
	uint32_t     getTranscoderSpeed() const;

	// The From D-Star Network section
	std::string  getDStarFromRemoteAddress() const;
	uint16_t     getDStarFromRemotePort() const;
	std::string  getDStarFromLocalAddress() const;
	uint16_t     getDStarFromLocalPort() const;
	bool         getDStarFromDebug() const;

	// The To D-Star Network section
	std::string  getDStarToRemoteAddress() const;
	uint16_t     getDStarToRemotePort() const;
	std::string  getDStarToLocalAddress() const;
	uint16_t     getDStarToLocalPort() const;
	bool         getDStarToDebug() const;

	// The From YSF Network section
	std::string  getYSFFromRemoteAddress() const;
	uint16_t     getYSFFromRemotePort() const;
	std::string  getYSFFromLocalAddress() const;
	uint16_t     getYSFFromLocalPort() const;
	bool         getYSFFromDebug() const;

	// The To YSF Network section
	std::string  getYSFToRemoteAddress() const;
	uint16_t     getYSFToRemotePort() const;
	std::string  getYSFToLocalAddress() const;
	uint16_t     getYSFToLocalPort() const;
	bool         getYSFToDebug() const;

	// The From FM Network section
	std::string  getFMFromRemoteAddress() const;
	uint16_t     getFMFromRemotePort() const;
	std::string  getFMFromLocalAddress() const;
	uint16_t     getFMFromLocalPort() const;
	bool         getFMFromDebug() const;

	// The To FM Network section
	std::string  getFMToRemoteAddress() const;
	uint16_t     getFMToRemotePort() const;
	std::string  getFMToLocalAddress() const;
	uint16_t     getFMToLocalPort() const;
	bool         getFMToDebug() const;

	// The From M17 Network section
	std::string  getM17FromRemoteAddress() const;
	uint16_t     getM17FromRemotePort() const;
	std::string  getM17FromLocalAddress() const;
	uint16_t     getM17FromLocalPort() const;
	bool         getM17FromDebug() const;

	// The To M17 Network section
	std::string  getM17ToRemoteAddress() const;
	uint16_t     getM17ToRemotePort() const;
	std::string  getM17ToLocalAddress() const;
	uint16_t     getM17ToLocalPort() const;
	bool         getM17ToDebug() const;

	// The Log section
	uint32_t     getLogDisplayLevel() const;
	uint32_t     getLogFileLevel() const;
	std::string  getLogFilePath() const;
	std::string  getLogFileRoot() const;
	bool         getLogFileRotate() const;

private:
	std::string  m_file;

	std::string  m_defaultCallsign;
	uint32_t     m_defaultDMRId;
	uint16_t     m_defaultNXDNId;
	bool         m_daemon;

	std::string  m_transcoderPort;
	uint32_t     m_transcoderSpeed;

	std::string  m_dStarFromRemoteAddress;
	uint16_t     m_dStarFromRemotePort;
	std::string  m_dStarFromLocalAddress;
	uint16_t     m_dStarFromLocalPort;
	bool         m_dStarFromDebug;

	std::string  m_dStarToRemoteAddress;
	uint16_t     m_dStarToRemotePort;
	std::string  m_dStarToLocalAddress;
	uint16_t     m_dStarToLocalPort;
	bool         m_dStarToDebug;

	std::string  m_ysfFromRemoteAddress;
	uint16_t     m_ysfFromRemotePort;
	std::string  m_ysfFromLocalAddress;
	uint16_t     m_ysfFromLocalPort;
	bool         m_ysfFromDebug;

	std::string  m_ysfToRemoteAddress;
	uint16_t     m_ysfToRemotePort;
	std::string  m_ysfToLocalAddress;
	uint16_t     m_ysfToLocalPort;
	bool         m_ysfToDebug;

	std::string  m_m17FromRemoteAddress;
	uint16_t     m_m17FromRemotePort;
	std::string  m_m17FromLocalAddress;
	uint16_t     m_m17FromLocalPort;
	bool         m_m17FromDebug;

	std::string  m_m17ToRemoteAddress;
	uint16_t     m_m17ToRemotePort;
	std::string  m_m17ToLocalAddress;
	uint16_t     m_m17ToLocalPort;
	bool         m_m17ToDebug;

	std::string  m_fmFromRemoteAddress;
	uint16_t     m_fmFromRemotePort;
	std::string  m_fmFromLocalAddress;
	uint16_t     m_fmFromLocalPort;
	bool         m_fmFromDebug;

	std::string  m_fmToRemoteAddress;
	uint16_t     m_fmToRemotePort;
	std::string  m_fmToLocalAddress;
	uint16_t     m_fmToLocalPort;
	bool         m_fmToDebug;

	uint32_t     m_logDisplayLevel;
	uint32_t     m_logFileLevel;
	std::string  m_logFilePath;
	std::string  m_logFileRoot;
	bool         m_logFileRotate;
};

#endif
