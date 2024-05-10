/*
 *   Copyright (C) 2009-2014,2016,2020,2021,2024 by Jonathan Naylor G4KLX
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

#if !defined(DStarNetwork_H)
#define	DStarNetwork_H

#include "DStarDefines.h"
#include "RingBuffer.h"
#include "UDPSocket.h"
#include "Network.h"
#include "Timer.h"

#include <cstdint>
#include <string>
#include <random>

// #define	DUMP_DSTAR

class CDStarNetwork : public INetwork {
public:
	CDStarNetwork(const std::string& callsign, const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug);
	virtual ~CDStarNetwork();

	virtual bool open();

	virtual bool write(CData& data);

	virtual bool read(CData& data);

	virtual bool hasData();

	virtual void reset();

	virtual void close();

	virtual void clock(unsigned int ms);

private:
	std::string      m_callsign;
	CUDPSocket       m_socket;
	sockaddr_storage m_addr;
	size_t           m_addrLen;
	bool             m_debug;
	uint16_t         m_outId;
	uint8_t          m_outSeq;
	uint16_t         m_inId;
	CRingBuffer<uint8_t> m_buffer;
	CTimer           m_pollTimer;
	std::mt19937     m_random;
	uint8_t*         m_header;
#if defined(DUMP_DSTAR)
	FILE*            m_fpIn;
	FILE*            m_fpOut;
#endif

	bool writeHeader(const CData& data);
	bool writeData(CData& data);
	bool writeTrailer(CData& data);
	bool writePoll(const char* text);

	void createHeader(const CData& data);
	void addSlowData(uint8_t* buffer);
	void stringToBytes(uint8_t* str, const std::string& callsign) const;
	void addHeaderCRC();
};

#endif
