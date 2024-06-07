/*
 *   Copyright (C) 2024 by Jonathan Naylor G4KLX
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
#include "Defines.h"

#include <string>
#include <cstdint>
#include <vector>

class CData {
public:
	CData(const std::string& port, uint32_t speed, bool debug, const std::string& callsign, uint32_t dmrId, uint16_t nxdnId);
	~CData();

	bool setFromMode(DATA_MODE mode);
	bool setToMode(DATA_MODE mode);
	bool setDirection(DIRECTION direction);

	void setToModes(bool toDStar, bool toDMR, bool toYSF, bool toP25, bool toNXDN, bool toFM, bool toM17);

	DATA_MODE getToMode() const;

	void setDStarFMDest(const std::string& dest);
	void setDStarM17Dests(const std::vector<std::string>& dests);

	void setYSFDStarDGIds(const std::vector<std::pair<uint8_t, std::string>>& dgIds);
	void setYSFFMDGId(uint8_t dgId);
	void setYSFM17DGIds(const std::vector<std::pair<uint8_t, std::string>>& dgIds);

	void setM17DStarDests(const std::vector<std::string>& dests);
	void setM17FMDest(const std::string& dest);

	bool open();

	void setDStar(NETWORK network, const uint8_t* source, const uint8_t* destination);
	void setDMR(NETWORK network, uint32_t source, uint32_t destination, bool group);
	void setYSF(NETWORK network, const uint8_t* source, uint8_t dgId);
	void setNXDN(NETWORK network, uint16_t source, uint16_t destination, bool group);
	void setP25(NETWORK network, uint32_t source, uint32_t destination, bool group);
	void setFM(NETWORK network);
	void setM17(NETWORK network, const std::string& source, const std::string& destination);

	void setEnd();

	void getDStar(NETWORK network, uint8_t* source, uint8_t* destination) const;
	void getDMR(NETWORK network, uint32_t& source, uint32_t& destination, bool& group);
	void getYSF(NETWORK network, uint8_t* source, uint8_t* destination, uint8_t& dgId) const;
	void getNXDN(NETWORK network, uint16_t& source, uint16_t& destination, bool& group);
	void getP25(NETWORK network, uint32_t& source, uint32_t& destination, bool& group);
	void getM17(NETWORK network, std::string& source, std::string& destination) const;

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

	bool        m_toDStar;
	bool        m_toDMR;
	bool        m_toYSF;
	bool        m_toP25;
	bool        m_toNXDN;
	bool        m_toFM;
	bool        m_toM17;

	std::string                                  m_dstarFMDest;
	std::vector<std::string>                     m_dstarM17Dests;
	std::vector<std::pair<uint8_t, std::string>> m_ysfDStarDGIds;
	uint8_t                                      m_ysfFMDGId;
	std::vector<std::pair<uint8_t, std::string>> m_ysfM17DGIds;
	std::vector<std::string>                     m_m17DStarDests;
	std::string                                  m_m17FMDest;

	DATA_MODE   m_fromMode;
	DATA_MODE   m_toMode;
	DIRECTION   m_direction;
	std::string m_srcCallsign;	// D-Star, YSF, M17
	std::string m_dstCallsign;	// D-Star, M17
	uint8_t     m_dgId;			// YSF
	uint32_t    m_srcId32;		// DMR, P25
	uint32_t    m_dstId32;		// DMR, P25
	uint16_t    m_srcId16;		// NXDN
	uint16_t    m_dstId16;		// NXDN
	bool        m_group;		// DMR, NXDN, P25
	bool        m_end;
	uint8_t*    m_data;
	uint16_t    m_length;
	uint8_t*    m_rawData;
	uint16_t    m_rawLength;
	uint16_t    m_count;

	bool setTranscoder();
	uint8_t find(const std::vector<std::pair<uint8_t, std::string>>& mapping, const std::string& dest) const;
	std::string find(const std::vector<std::pair<uint8_t, std::string>>& mapping, uint8_t dgId) const;
	std::string bytesToString(const uint8_t* str, size_t length) const;
	void stringToBytes(uint8_t* str, size_t length, const std::string& callsign) const;
};

#endif
