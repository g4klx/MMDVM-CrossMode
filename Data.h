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
#include "Defines.h"

#include <string>
#include <cstdint>

class CData {
public:
	CData(const std::string& callsign, uint32_t dmrId, uint16_t nxdnId);
	~CData();

	void setModes(DATA_MODE fromMode, DATA_MODE toMode);

	void setDStar(const std::string& source, const std::string& destination);
	void setDMR(uint32_t source, uint32_t destination, bool group);
	void setYSF(const std::string& source, uint8_t dgId);
	void setNXDN(uint16_t source, uint16_t destination, bool group);
	void setP25(uint32_t source, uint32_t destination, bool group);
	void setM17(const std::string& source, const std::string& destination);

	void setData(const uint8_t* data);

	void setEnd();
	bool isEnd() const;

	void reset();

private:
	std::string m_defaultCallsign;
	uint32_t    m_defaultDMRId;
	uint16_t    m_defaultNXDNId;
	DATA_MODE   m_fromMode;
	DATA_MODE   m_toMode;
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
};

#endif
