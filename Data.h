/*
 *   Copyright (C) 2024,2026 by Jonathan Naylor G4KLX
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

#if !defined(Data_H)
#define	Data_H

#include "TranscoderDefines.h"
#include "Transcoder.h"
#include "NXDNLookup.h"
#include "DMRLookup.h"
#include "Defines.h"

#include <string>
#include <cstdint>
#include <vector>
#include <tuple>

class CData {
public:
	CData(const std::string& callsign, uint32_t dmrId, uint16_t nxdnId, bool debug);
	~CData();

	void setUARTConnection(const std::string& port, uint32_t speed);
	void setUDPConnection(const std::string& remoteAddress, uint16_t remotePort, const std::string& localAddress, uint16_t localPort);

	bool setFromMode(DATA_MODE mode);
	bool setToMode(DATA_MODE mode);
	bool setDirection(DIRECTION direction);

	void setThroughModes(bool toDStar, bool toDMR1, bool toDMR2, bool toYSF, bool toP25, bool toNXDN, bool toFM);

	bool setDMRLookup(const std::string& filename, unsigned int reloadTime);
	bool setNXDNLookup(const std::string& filename, unsigned int reloadTime);

	DATA_MODE getToMode() const;

	void setDStarDMRDests(const std::vector<std::tuple<std::string, uint8_t, uint32_t>>& dests);
	void setDStarYSFDests(const std::vector<std::pair<std::string, uint8_t>>& dests);
	void setDStarP25Dests(const std::vector<std::pair<std::string, uint32_t>>& dests);
	void setDStarNXDNDests(const std::vector<std::pair<std::string, uint16_t>>& dests);
	void setDStarFMDest(const std::string& dest);

	void setDMRDStarTGs(const std::vector<std::tuple<uint8_t, uint32_t, std::string>>& tgs);
	void setDMRYSFTGs(const std::vector<std::tuple<uint8_t, uint32_t, uint8_t>>& tgs);
	void setDMRP25TGs(const std::vector<std::tuple<uint8_t, uint32_t, uint32_t>>& tgs);
	void setDMRNXDNTGs(const std::vector<std::tuple<uint8_t, uint32_t, uint16_t>>& tgs);
	void setDMRFMTG(const std::pair<uint8_t, uint32_t>& tg);

	void setYSFDStarDGIds(const std::vector<std::pair<uint8_t, std::string>>& dgIds);
	void setYSFDMRDGIds(const std::vector<std::tuple<uint8_t, uint8_t, uint32_t>>& dgIds);
	void setYSFP25DGIds(const std::vector<std::pair<uint8_t, uint32_t>>& dgIds);
	void setYSFNXDNDGIds(const std::vector<std::pair<uint8_t, uint16_t>>& dgIds);
	void setYSFFMDGId(uint8_t dgId);

	void setP25DStarTGs(const std::vector<std::pair<uint32_t, std::string>>& tgs);
	void setP25DMRTGs(const std::vector<std::tuple<uint32_t, uint8_t, uint32_t>>& tgs);
	void setP25YSFTGs(const std::vector<std::pair<uint32_t, uint8_t>>& tgs);
	void setP25NXDNTGs(const std::vector<std::pair<uint32_t, uint16_t>>& tgs);
	void setP25FMTG(uint32_t tg);

	void setNXDNDStarTGs(const std::vector<std::pair<uint16_t, std::string>>& tgs);
	void setNXDNDMRTGs(const std::vector<std::tuple<uint16_t, uint8_t, uint32_t>>& tgs);
	void setNXDNYSFTGs(const std::vector<std::pair<uint16_t, uint8_t>>& tgs);
	void setNXDNP25TGs(const std::vector<std::pair<uint16_t, uint32_t>>& tgs);
	void setNXDNFMTG(uint16_t tg);

	bool open();

	void setDStar(NETWORK network, const uint8_t* source, const uint8_t* destination);
	void setDMR(NETWORK network, uint8_t slot, uint32_t source, uint32_t destination, bool group);
	void setYSF(NETWORK network, const uint8_t* source, uint8_t dgId);
	void setNXDN(NETWORK network, uint16_t source, uint16_t destination, bool group);
	void setP25(NETWORK network, uint32_t source, uint32_t destination, bool group);
	void setFM(NETWORK network);

	void setEnd();

	void getDStar(NETWORK network, uint8_t* source, uint8_t* destination) const;
	void getDMR(NETWORK network, uint8_t& slot, uint32_t& source, uint32_t& destination, bool& group) const;
	void getYSF(NETWORK network, uint8_t* source, uint8_t* destination, uint8_t& dgId) const;
	void getNXDN(NETWORK network, uint16_t& source, uint16_t& destination, bool& group) const;
	void getP25(NETWORK network, uint32_t& source, uint32_t& destination, bool& group) const;

	void     setRaw(const uint8_t* data, uint16_t length);
	bool     setData(const uint8_t* data);

	bool     hasRaw() const;
	bool     hasData() const;

	uint16_t getRaw(uint8_t* data);
	bool     getData(uint8_t* data);

	bool isEnd() const;

	bool isTranscode() const;

	void clock(unsigned int ms);

	void reset();

	void close();

private:
	CTranscoder m_transcoder;
	std::string m_defaultCallsign;
	uint32_t    m_defaultDMRId;
	uint16_t    m_defaultNXDNId;
	CDMRLookup  m_dmrLookup;
	CNXDNLookup m_nxdnLookup;
	bool        m_debug;

	bool        m_toDStar;
	bool        m_toDMR1;
	bool        m_toDMR2;
	bool        m_toYSF;
	bool        m_toP25;
	bool        m_toNXDN;
	bool        m_toFM;

	std::vector<std::tuple<std::string, uint8_t, uint32_t>> m_dstarDMRDests;
	std::vector<std::pair<std::string, uint8_t>>            m_dstarYSFDests;
	std::vector<std::pair<std::string, uint32_t>>           m_dstarP25Dests;
	std::vector<std::pair<std::string, uint16_t>>           m_dstarNXDNDests;
	std::string                                             m_dstarFMDest;

	std::vector<std::tuple<uint8_t, uint32_t, std::string>> m_dmrDStarTGs;
	std::vector<std::tuple<uint8_t, uint32_t, uint8_t>>     m_dmrYSFTGs;
	std::vector<std::tuple<uint8_t, uint32_t, uint32_t>>    m_dmrP25TGs;
	std::vector<std::tuple<uint8_t, uint32_t, uint16_t>>    m_dmrNXDNTGs;
	std::pair<uint8_t, uint32_t>                            m_dmrFMTG;

	std::vector<std::pair<uint8_t, std::string>>            m_ysfDStarDGIds;
	std::vector<std::tuple<uint8_t, uint8_t, uint32_t>>     m_ysfDMRDGIds;
	std::vector<std::pair<uint8_t, uint32_t>>               m_ysfP25DGIds;
	std::vector<std::pair<uint8_t, uint16_t>>               m_ysfNXDNDGIds;
	uint8_t                                                 m_ysfFMDGId;

	std::vector<std::pair<uint32_t, std::string>>           m_p25DStarTGs;
	std::vector<std::tuple<uint32_t, uint8_t, uint32_t>>    m_p25DMRTGs;
	std::vector<std::pair<uint32_t, uint8_t>>               m_p25YSFTGs;
	std::vector<std::pair<uint32_t, uint16_t>>              m_p25NXDNTGs;
	uint32_t                                                m_p25FMTG;

	std::vector<std::pair<uint16_t, std::string>>           m_nxdnDStarTGs;
	std::vector<std::tuple<uint16_t, uint8_t, uint32_t>>    m_nxdnDMRTGs;
	std::vector<std::pair<uint16_t, uint8_t>>               m_nxdnYSFTGs;
	std::vector<std::pair<uint16_t, uint32_t>>              m_nxdnP25TGs;
	uint16_t                                                m_nxdnFMTG;

	DATA_MODE   m_fromMode;
	DATA_MODE   m_toMode;
	DIRECTION   m_direction;
	std::string m_srcCallsign;	// D-Star, YSF, M17
	std::string m_dstCallsign;	// D-Star, M17
	uint8_t     m_dgId;			// YSF
	uint8_t     m_slot;			// DMR
	uint32_t    m_srcId;		// DMR, NXDN, P25
	uint32_t    m_dstId;		// DMR, NXDN, P25
	bool        m_group;		// DMR, NXDN, P25
	bool        m_end;
	uint8_t*    m_data;
	uint16_t    m_length;
	uint8_t*    m_rawData;
	uint16_t    m_rawLength;
	uint16_t    m_count;

	bool setTranscoder();

	// uint8_t <=> std::string
	uint8_t find(const std::vector<std::pair<std::string, uint8_t>>& mapping, const std::string& dest) const;
	std::string find(const std::vector<std::pair<std::string, uint8_t>>& mapping, uint8_t dgId) const;

	uint8_t find(const std::vector<std::pair<uint8_t, std::string>>& mapping, const std::string& dest) const;
	std::string find(const std::vector<std::pair<uint8_t, std::string>>& mapping, uint8_t dgId) const;

	// uint16_t <=> std::string
	uint16_t find(const std::vector<std::pair<std::string, uint16_t>>& mapping, const std::string& dest) const;
	std::string find(const std::vector<std::pair<std::string, uint16_t>>& mapping, uint16_t tgId) const;

	uint16_t find(const std::vector<std::pair<uint16_t, std::string>>& mapping, const std::string& dest) const;
	std::string find(const std::vector<std::pair<uint16_t, std::string>>& mapping, uint16_t tgId) const;

	// uint32_t <=> std::string
	uint32_t find(const std::vector<std::pair<std::string, uint32_t>>& mapping, const std::string& dest) const;
	std::string find(const std::vector<std::pair<std::string, uint32_t>>& mapping, uint32_t tgId) const;

	uint32_t find(const std::vector<std::pair<uint32_t, std::string>>& mapping, const std::string& dest) const;
	std::string find(const std::vector<std::pair<uint32_t, std::string>>& mapping, uint32_t tgId) const;

	// <uint8_t, uint32_t> <=> std::string
	std::pair<uint8_t, uint32_t> find(const std::vector<std::tuple<std::string, uint8_t, uint32_t>>& mapping, const std::string& dest) const;
	std::string find(const std::vector<std::tuple<std::string, uint8_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const;

	std::string find(const std::vector<std::tuple<uint8_t, uint32_t, std::string>>& mapping, uint8_t slot, uint32_t tgId) const;
	std::pair<uint8_t, uint32_t> find(const std::vector<std::tuple<uint8_t, uint32_t, std::string>>& mapping, const std::string& dest) const;

	// <uint8_t, uint32_t> <=> uint16_t
	std::pair<uint8_t, uint32_t> find(const std::vector<std::tuple<uint8_t, uint32_t, uint16_t>>& mapping, uint16_t tgId) const;
	uint16_t find(const std::vector<std::tuple<uint8_t, uint32_t, uint16_t>>& mapping, uint8_t slot, uint32_t tgId) const;

	std::pair<uint8_t, uint32_t> find(const std::vector<std::tuple<uint16_t, uint8_t, uint32_t>>& mapping, uint16_t tgId) const;
	uint16_t find(const std::vector<std::tuple<uint16_t, uint8_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const;

	// <uint8_t, uint32_t> <=> uint32_t
	std::pair<uint8_t, uint32_t> find(const std::vector<std::tuple<uint8_t, uint32_t, uint32_t>>& mapping, uint32_t tgId) const;
	uint32_t find(const std::vector<std::tuple<uint8_t, uint32_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const;

	std::pair<uint8_t, uint32_t> find(const std::vector<std::tuple<uint32_t, uint8_t, uint32_t>>& mapping, uint32_t tgId) const;
	uint32_t find(const std::vector<std::tuple<uint32_t, uint8_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const;

	// uint8_t <=> uint16_t
	uint8_t find(const std::vector<std::pair<uint8_t, uint16_t>>& mapping, uint16_t tgId) const;
	uint16_t find(const std::vector<std::pair<uint8_t, uint16_t>>& mapping, uint8_t dgId) const;

	uint8_t find(const std::vector<std::pair<uint16_t, uint8_t>>& mapping, uint16_t tgId) const;
	uint16_t find(const std::vector<std::pair<uint16_t, uint8_t>>& mapping, uint8_t dgId) const;

	// uint32_t <=> uint16_t
	uint32_t find(const std::vector<std::pair<uint32_t, uint16_t>>& mapping, uint16_t tgId) const;
	uint16_t find(const std::vector<std::pair<uint32_t, uint16_t>>& mapping, uint32_t tgId) const;

	uint32_t find(const std::vector<std::pair<uint16_t, uint32_t>>& mapping, uint16_t tgId) const;
	uint16_t find(const std::vector<std::pair<uint16_t, uint32_t>>& mapping, uint32_t tgId) const;

	// uint8_t <=> uint32_t
	uint8_t find(const std::vector<std::pair<uint8_t, uint32_t>>& mapping, uint32_t tgId) const;
	uint32_t find(const std::vector<std::pair<uint8_t, uint32_t>>& mapping, uint8_t dgId) const;

	uint8_t find(const std::vector<std::pair<uint32_t, uint8_t>>& mapping, uint32_t tgId) const;
	uint32_t find(const std::vector<std::pair<uint32_t, uint8_t>>& mapping, uint8_t dgId) const;

	// <uint8_t, uint32_t> <=> uint8_t
	uint8_t find(const std::vector<std::tuple<uint8_t, uint8_t, uint32_t>>& mapping, uint8_t slot, uint32_t tgId) const;
	std::pair<uint8_t, uint32_t> find(const std::vector<std::tuple<uint8_t, uint8_t, uint32_t>>& mapping, uint8_t dgId) const;

	std::pair<uint8_t, uint32_t> find(const std::vector<std::tuple<uint8_t, uint32_t, uint8_t>>& mapping, uint8_t dgId) const;
	uint8_t find(const std::vector<std::tuple<uint8_t, uint32_t, uint8_t>>& mapping, uint8_t slot, uint32_t tgid) const;

	std::string bytesToString(const uint8_t* str, size_t length) const;
	void stringToBytes(uint8_t* str, size_t length, const std::string& callsign) const;
};

#endif
