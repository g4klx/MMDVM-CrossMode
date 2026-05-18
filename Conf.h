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

#include <cstdint>

class CConf
{
public:
	CConf(const std::string& file);
	~CConf();

	bool read();

	// The General section
	std::string  getCallsign() const;
	unsigned int getRFModeHang() const;
	unsigned int getNetModeHang() const;
	bool         getDaemon() const;

	// The Log section
	unsigned int getLogDisplayLevel() const;
	unsigned int getLogMQTTLevel() const;

	// The MQTT section
	std::string  getMQTTAddress() const;
	uint16_t     getMQTTPort() const;
	unsigned int getMQTTKeepalive() const;
	std::string  getMQTTName() const;
	bool         getMQTTAuthEnabled() const;
	std::string  getMQTTUsername() const;
	std::string  getMQTTPassword() const;

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

	// The D-Star RF Network section
	std::string  getDStarRFRemoteAddress() const;
	uint16_t     getDStarRFRemotePort() const;
	std::string  getDStarRFLocalAddress() const;
	uint16_t     getDStarRFLocalPort() const;
	bool         getDStarRFDebug() const;

	// The D-Star Net Network section
	std::string  getDStarNetRemoteAddress() const;
	uint16_t     getDStarNetRemotePort() const;
	std::string  getDStarNetLocalAddress() const;
	uint16_t     getDStarNetLocalPort() const;
	bool         getDStarNetDebug() const;

	// The DMR RF Network section
	std::string  getDMRRFRemoteAddress() const;
	uint16_t     getDMRRFRemotePort() const;
	std::string  getDMRRFLocalAddress() const;
	uint16_t     getDMRRFLocalPort() const;
	bool         getDMRRFDebug() const;

	// The DMR Net Network section
	std::string  getDMRNetRemoteAddress() const;
	uint16_t     getDMRNetRemotePort() const;
	std::string  getDMRNetLocalAddress() const;
	uint16_t     getDMRNetLocalPort() const;
	bool         getDMRNetDebug() const;

	// The YSF RF Network section
	std::string  getYSFRFRemoteAddress() const;
	uint16_t     getYSFRFRemotePort() const;
	std::string  getYSFRFLocalAddress() const;
	uint16_t     getYSFRFLocalPort() const;
	bool         getYSFRFDebug() const;

	// The YSF Net Network section
	std::string  getYSFNetRemoteAddress() const;
	uint16_t     getYSFNetRemotePort() const;
	std::string  getYSFNetLocalAddress() const;
	uint16_t     getYSFNetLocalPort() const;
	bool         getYSFNetDebug() const;

	// The P25 RF Network section
	std::string  getP25RFRemoteAddress() const;
	uint16_t     getP25RFRemotePort() const;
	std::string  getP25RFLocalAddress() const;
	uint16_t     getP25RFLocalPort() const;
	bool         getP25RFDebug() const;

	// The P25 Net Network section
	std::string  getP25NetRemoteAddress() const;
	uint16_t     getP25NetRemotePort() const;
	std::string  getP25NetLocalAddress() const;
	uint16_t     getP25NetLocalPort() const;
	bool         getP25NetDebug() const;

	// The NXDN RF Network section
	std::string  getNXDNRFRemoteAddress() const;
	uint16_t     getNXDNRFRemotePort() const;
	std::string  getNXDNRFLocalAddress() const;
	uint16_t     getNXDNRFLocalPort() const;
	bool         getNXDNRFDebug() const;

	// The NXDN Net Network section
	std::string  getNXDNNetRemoteAddress() const;
	uint16_t     getNXDNNetRemotePort() const;
	std::string  getNXDNNetLocalAddress() const;
	uint16_t     getNXDNNetLocalPort() const;
	bool         getNXDNNetDebug() const;

	// The FM RF Network section
	std::string  getFMRFRemoteAddress() const;
	uint16_t     getFMRFRemotePort() const;
	std::string  getFMRFLocalAddress() const;
	uint16_t     getFMRFLocalPort() const;
	bool         getFMRFDebug() const;

	// The FM Net Network section
	std::string  getFMNetRemoteAddress() const;
	uint16_t     getFMNetRemotePort() const;
	std::string  getFMNetLocalAddress() const;
	uint16_t     getFMNetLocalPort() const;
	bool         getFMNetDebug() const;

private:
	std::string  m_file;

	std::string  m_callsign;
	unsigned int m_rfModeHang;
	unsigned int m_netModeHang;
	bool         m_daemon;

	unsigned int m_logDisplayLevel;
	unsigned int m_logMQTTLevel;

	std::string  m_mqttAddress;
	uint16_t     m_mqttPort;
	unsigned int m_mqttKeepalive;
	std::string  m_mqttName;
	bool         m_mqttAuthEnabled;
	std::string  m_mqttUsername;
	std::string  m_mqttPassword;

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

	std::string  m_dStarRFRemoteAddress;
	uint16_t     m_dStarRFRemotePort;
	std::string  m_dStarRFLocalAddress;
	uint16_t     m_dStarRFLocalPort;
	bool         m_dStarRFDebug;

	std::string  m_dStarNetRemoteAddress;
	uint16_t     m_dStarNetRemotePort;
	std::string  m_dStarNetLocalAddress;
	uint16_t     m_dStarNetLocalPort;
	bool         m_dStarNetDebug;

	std::string  m_dmrRFRemoteAddress;
	uint16_t     m_dmrRFRemotePort;
	std::string  m_dmrRFLocalAddress;
	uint16_t     m_dmrRFLocalPort;
	bool         m_dmrRFDebug;

	std::string  m_dmrNetRemoteAddress;
	uint16_t     m_dmrNetRemotePort;
	std::string  m_dmrNetLocalAddress;
	uint16_t     m_dmrNetLocalPort;
	bool         m_dmrNetDebug;

	std::string  m_ysfRFRemoteAddress;
	uint16_t     m_ysfRFRemotePort;
	std::string  m_ysfRFLocalAddress;
	uint16_t     m_ysfRFLocalPort;
	bool         m_ysfRFDebug;

	std::string  m_ysfNetRemoteAddress;
	uint16_t     m_ysfNetRemotePort;
	std::string  m_ysfNetLocalAddress;
	uint16_t     m_ysfNetLocalPort;
	bool         m_ysfNetDebug;

	std::string  m_p25RFRemoteAddress;
	uint16_t     m_p25RFRemotePort;
	std::string  m_p25RFLocalAddress;
	uint16_t     m_p25RFLocalPort;
	bool         m_p25RFDebug;

	std::string  m_p25NetRemoteAddress;
	uint16_t     m_p25NetRemotePort;
	std::string  m_p25NetLocalAddress;
	uint16_t     m_p25NetLocalPort;
	bool         m_p25NetDebug;

	std::string  m_nxdnRFRemoteAddress;
	uint16_t     m_nxdnRFRemotePort;
	std::string  m_nxdnRFLocalAddress;
	uint16_t     m_nxdnRFLocalPort;
	bool         m_nxdnRFDebug;

	std::string  m_nxdnNetRemoteAddress;
	uint16_t     m_nxdnNetRemotePort;
	std::string  m_nxdnNetLocalAddress;
	uint16_t     m_nxdnNetLocalPort;
	bool         m_nxdnNetDebug;

	std::string  m_fmRFRemoteAddress;
	uint16_t     m_fmRFRemotePort;
	std::string  m_fmRFLocalAddress;
	uint16_t     m_fmRFLocalPort;
	bool         m_fmRFDebug;

	std::string  m_fmNetRemoteAddress;
	uint16_t     m_fmNetRemotePort;
	std::string  m_fmNetLocalAddress;
	uint16_t     m_fmNetLocalPort;
	bool         m_fmNetDebug;

	std::string getString(const char* text) const;
	std::pair<uint8_t, uint32_t> getSlotTG(char* text) const;
};

#endif
