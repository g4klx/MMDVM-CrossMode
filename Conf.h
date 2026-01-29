/*
 *   Copyright (C) 2015,2016,2017,2024,2026 by Jonathan Naylor G4KLX
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
#include <tuple>

class CConf
{
public:
	CConf(const std::string& file);
	~CConf();

	bool read();

	// The General section
	std::string  getCallsign() const;
	std::string  getFromMode() const;
	unsigned int getRFModeHang() const;
	unsigned int getNetModeHang() const;
	bool         getDaemon() const;

	// The Log section
	uint32_t     getLogDisplayLevel() const;
	uint32_t     getLogFileLevel() const;
	std::string  getLogFilePath() const;
	std::string  getLogFileRoot() const;
	bool         getLogFileRotate() const;

	// The Transcoder section
	std::string  getTranscoderProtocol() const;
	std::string  getTranscoderUARTPort() const;
	uint32_t     getTranscoderUARTSpeed() const;
	std::string  getTranscoderRemoteAddress() const;
	uint16_t     getTranscoderRemotePort() const;
	std::string  getTranscoderLocalAddress() const;
	uint16_t     getTranscoderLocalPort() const;
	bool         getTranscoderDebug() const;

	// The Lookup section
	std::string  getDMRLookupFile() const;
	std::string  getNXDNLookupFile() const;
	unsigned int getReloadTime() const;

	// The D-Star section
	std::string  getDStarModule() const;

	// The DMR section
	uint32_t     getDMRId() const;

	// The NXDN section
	uint16_t     getNXDNId() const;

	// The D-Star to D-Star section
	bool         getDStarDStarEnable() const;

	// The D-Star to DMR section
	bool         getDStarDMREnable() const;
	std::vector<std::tuple<std::string, uint8_t, uint32_t>> getDStarDMRDests() const;

	// The D-Star to System Fusion section
	bool         getDStarYSFEnable() const;
	std::vector<std::pair<std::string, uint8_t>> getDStarYSFDests() const;

	// The D-Star to P25 section
	bool         getDStarP25Enable() const;
	std::vector<std::pair<std::string, uint32_t>> getDStarP25Dests() const;

	// The D-Star to NXDN section
	bool         getDStarNXDNEnable() const;
	std::vector<std::pair<std::string, uint16_t>> getDStarNXDNDests() const;

	// The D-Star to FM section
	bool         getDStarFMEnable() const;
	std::string  getDStarFMDest() const;

	// The DMR to D-Star section
	bool         getDMRDStarEnable() const;
	std::vector<std::tuple<uint8_t, uint32_t, std::string>> getDMRDStarTGs() const;

	// The DMR to DMR section
	bool         getDMRDMREnable1() const;
	bool         getDMRDMREnable2() const;

	// The DMR to System Fusion section
	bool         getDMRYSFEnable() const;
	std::vector<std::tuple<uint8_t, uint32_t, uint8_t>> getDMRYSFTGs() const;

	// The DMR to P25 section
	bool         getDMRP25Enable() const;
	std::vector<std::tuple<uint8_t, uint32_t, uint32_t>> getDMRP25TGs() const;

	// The DMR to NXDN section
	bool         getDMRNXDNEnable() const;
	std::vector<std::tuple<uint8_t, uint32_t, uint16_t>> getDMRNXDNTGs() const;

	// The DMR to FM section
	bool         getDMRFMEnable() const;
	std::pair<uint8_t, uint32_t> getDMRFMTG() const;

	// The YSF to D-Star section
	bool         getYSFDStarEnable() const;
	std::vector<std::pair<uint8_t, std::string>> getYSFDStarDGIds() const;

	// The YSF to DMR section
	bool         getYSFDMREnable() const;
	std::vector<std::tuple<uint8_t, uint8_t, uint32_t>> getYSFDMRDGIds() const;

	// The YSF to System Fusion section
	bool         getYSFYSFEnable() const;

	// The YSF to P25 section
	bool         getYSFP25Enable() const;
	std::vector<std::pair<uint8_t, uint32_t>> getYSFP25DGIds() const;

	// The YSF to NXDN section
	bool         getYSFNXDNEnable() const;
	std::vector<std::pair<uint8_t, uint16_t>> getYSFNXDNDGIds() const;

	// The YSF to FM section
	bool         getYSFFMEnable() const;
	uint8_t      getYSFFMDGId() const;

	// The P25 to D-Star section
	bool         getP25DStarEnable() const;
	std::vector<std::pair<uint32_t, std::string>> getP25DStarTGs() const;

	// The P25 to YSF section
	bool         getP25YSFEnable() const;
	std::vector<std::pair<uint32_t, uint8_t>> getP25YSFTGs() const;

	// The P25 to DMR section
	bool         getP25DMREnable() const;
	std::vector<std::tuple<uint32_t, uint8_t, uint32_t>> getP25DMRTGs() const;

	// The P25 to P25 section
	bool         getP25P25Enable() const;

	// The P25 to NXDN section
	bool         getP25NXDNEnable() const;
	std::vector<std::pair<uint32_t, uint16_t>> getP25NXDNTGs() const;

	// The P25 to FM section
	bool         getP25FMEnable() const;
	uint32_t     getP25FMTG() const;

	// The NXDN to D-Star section
	bool         getNXDNDStarEnable() const;
	std::vector<std::pair<uint16_t, std::string>> getNXDNDStarTGs() const;

	// The NXDN to DMR section
	bool         getNXDNDMREnable() const;
	std::vector<std::tuple<uint16_t, uint8_t, uint32_t>> getNXDNDMRTGs() const;

	// The NXDN to System Fusion section
	bool         getNXDNYSFEnable() const;
	std::vector<std::pair<uint16_t, uint8_t>> getNXDNYSFTGs() const;

	// The NXDN to P25 section
	bool         getNXDNP25Enable() const;
	std::vector<std::pair<uint16_t, uint32_t>> getNXDNP25TGs() const;

	// The NXDN to NXDN section
	bool         getNXDNNXDNEnable() const;

	// The NXDN to FM section
	bool         getNXDNFMEnable() const;
	uint16_t     getNXDNFMTG() const;

	// The FM to D-Star section
	bool         getFMDStarEnable() const;

	// The FM to DMR section
	bool         getFMDMREnable() const;

	// The FM to System Fusion section
	bool         getFMYSFEnable() const;

	// The FM to P25 section
	bool         getFMP25Enable() const;

	// The FM to NXDN section
	bool         getFMNXDNEnable() const;

	// The FM to FM section
	bool         getFMFMEnable() const;

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

	// The From DMR Network section
	std::string  getDMRFromRemoteAddress() const;
	uint16_t     getDMRFromRemotePort() const;
	std::string  getDMRFromLocalAddress() const;
	uint16_t     getDMRFromLocalPort() const;
	bool         getDMRFromDebug() const;

	// The To DMR Network section
	std::string  getDMRToRemoteAddress() const;
	uint16_t     getDMRToRemotePort() const;
	std::string  getDMRToLocalAddress() const;
	uint16_t     getDMRToLocalPort() const;
	bool         getDMRToDebug() const;

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

	// The From P25 Network section
	std::string  getP25FromRemoteAddress() const;
	uint16_t     getP25FromRemotePort() const;
	std::string  getP25FromLocalAddress() const;
	uint16_t     getP25FromLocalPort() const;
	bool         getP25FromDebug() const;

	// The To P25 Network section
	std::string  getP25ToRemoteAddress() const;
	uint16_t     getP25ToRemotePort() const;
	std::string  getP25ToLocalAddress() const;
	uint16_t     getP25ToLocalPort() const;
	bool         getP25ToDebug() const;

	// The From NXDN Network section
	std::string  getNXDNFromRemoteAddress() const;
	uint16_t     getNXDNFromRemotePort() const;
	std::string  getNXDNFromLocalAddress() const;
	uint16_t     getNXDNFromLocalPort() const;
	bool         getNXDNFromDebug() const;

	// The To NXDN Network section
	std::string  getNXDNToRemoteAddress() const;
	uint16_t     getNXDNToRemotePort() const;
	std::string  getNXDNToLocalAddress() const;
	uint16_t     getNXDNToLocalPort() const;
	bool         getNXDNToDebug() const;

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

private:
	std::string  m_file;

	std::string  m_callsign;
	std::string  m_fromMode;
	unsigned int m_rfModeHang;
	unsigned int m_netModeHang;
	bool         m_daemon;

	uint32_t     m_logDisplayLevel;
	uint32_t     m_logFileLevel;
	std::string  m_logFilePath;
	std::string  m_logFileRoot;
	bool         m_logFileRotate;

	std::string  m_transcoderProtocol;
	std::string  m_transcoderUARTPort;
	uint32_t     m_transcoderUARTSpeed;
	std::string  m_transcoderRemoteAddress;
	uint16_t     m_transcoderRemotePort;
	std::string  m_transcoderLocalAddress;
	uint16_t     m_transcoderLocalPort;
	bool         m_transcoderDebug;

	std::string  m_dmrLookupFile;
	std::string  m_nxdnLookupFile;
	unsigned int m_reloadTime;

	std::string  m_dStarModule;

	uint32_t     m_dmrId;

	uint16_t     m_nxdnId;

	bool         m_dstarDStarEnable;

	bool         m_dstarDMREnable;
	std::vector<std::tuple<std::string, uint8_t, uint32_t>> m_dstarDMRDests;

	bool         m_dstarYSFEnable;
	std::vector<std::pair<std::string, uint8_t>> m_dstarYSFDests;

	bool         m_dstarP25Enable;
	std::vector<std::pair<std::string, uint32_t>> m_dstarP25Dests;

	bool         m_dstarNXDNEnable;
	std::vector<std::pair<std::string, uint16_t>> m_dstarNXDNDests;

	bool         m_dstarFMEnable;
	std::string  m_dstarFMDest;

	bool         m_dmrDStarEnable;
	std::vector<std::tuple<uint8_t, uint32_t, std::string>> m_dmrDStarTGs;

	bool         m_dmrDMREnable1;
	bool         m_dmrDMREnable2;

	bool         m_dmrYSFEnable;
	std::vector<std::tuple<uint8_t, uint32_t, uint8_t>> m_dmrYSFTGs;

	bool         m_dmrP25Enable;
	std::vector<std::tuple<uint8_t, uint32_t, uint32_t>> m_dmrP25TGs;

	bool         m_dmrNXDNEnable;
	std::vector<std::tuple<uint8_t, uint32_t, uint16_t>> m_dmrNXDNTGs;

	bool         m_dmrFMEnable;
	std::pair<uint8_t, uint32_t> m_dmrFMTG;

	bool         m_ysfDStarEnable;
	std::vector<std::pair<uint8_t, std::string>> m_ysfDStarDGIds;

	bool         m_ysfDMREnable;
	std::vector<std::tuple<uint8_t, uint8_t, uint32_t>> m_ysfDMRDGIds;

	bool         m_ysfYSFEnable;

	bool         m_ysfP25Enable;
	std::vector<std::pair<uint8_t, uint32_t>> m_ysfP25DGIds;

	bool         m_ysfNXDNEnable;
	std::vector<std::pair<uint8_t, uint16_t>> m_ysfNXDNDGIds;

	bool         m_ysfFMEnable;
	uint8_t      m_ysfFMDGId;

	bool         m_p25DStarEnable;
	std::vector<std::pair<uint32_t, std::string>> m_p25DStarTGs;

	bool         m_p25DMREnable;
	std::vector<std::tuple<uint32_t, uint8_t, uint32_t>> m_p25DMRTGs;

	bool         m_p25YSFEnable;
	std::vector<std::pair<uint32_t, uint8_t>> m_p25YSFTGs;

	bool         m_p25P25Enable;

	bool         m_p25NXDNEnable;
	std::vector<std::pair<uint32_t, uint16_t>> m_p25NXDNTGs;

	bool         m_p25FMEnable;
	uint32_t     m_p25FMTG;

	bool         m_nxdnDStarEnable;
	std::vector<std::pair<uint16_t, std::string>> m_nxdnDStarTGs;

	bool         m_nxdnDMREnable;
	std::vector<std::tuple<uint16_t, uint8_t, uint32_t>> m_nxdnDMRTGs;

	bool         m_nxdnYSFEnable;
	std::vector<std::pair<uint16_t, uint8_t>> m_nxdnYSFTGs;

	bool         m_nxdnP25Enable;
	std::vector<std::pair<uint16_t, uint32_t>> m_nxdnP25TGs;

	bool         m_nxdnNXDNEnable;

	bool         m_nxdnFMEnable;
	uint16_t     m_nxdnFMTG;

	bool         m_fmDStarEnable;

	bool         m_fmDMREnable;

	bool         m_fmYSFEnable;

	bool         m_fmP25Enable;

	bool         m_fmNXDNEnable;

	bool         m_fmFMEnable;

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

	std::string  m_dmrFromRemoteAddress;
	uint16_t     m_dmrFromRemotePort;
	std::string  m_dmrFromLocalAddress;
	uint16_t     m_dmrFromLocalPort;
	bool         m_dmrFromDebug;

	std::string  m_dmrToRemoteAddress;
	uint16_t     m_dmrToRemotePort;
	std::string  m_dmrToLocalAddress;
	uint16_t     m_dmrToLocalPort;
	bool         m_dmrToDebug;

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

	std::string  m_p25FromRemoteAddress;
	uint16_t     m_p25FromRemotePort;
	std::string  m_p25FromLocalAddress;
	uint16_t     m_p25FromLocalPort;
	bool         m_p25FromDebug;

	std::string  m_p25ToRemoteAddress;
	uint16_t     m_p25ToRemotePort;
	std::string  m_p25ToLocalAddress;
	uint16_t     m_p25ToLocalPort;
	bool         m_p25ToDebug;

	std::string  m_nxdnFromRemoteAddress;
	uint16_t     m_nxdnFromRemotePort;
	std::string  m_nxdnFromLocalAddress;
	uint16_t     m_nxdnFromLocalPort;
	bool         m_nxdnFromDebug;

	std::string  m_nxdnToRemoteAddress;
	uint16_t     m_nxdnToRemotePort;
	std::string  m_nxdnToLocalAddress;
	uint16_t     m_nxdnToLocalPort;
	bool         m_nxdnToDebug;

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

	std::string getString(const char* text) const;
	std::pair<uint8_t, uint32_t> getSlotTG(char* text) const;
};

#endif
