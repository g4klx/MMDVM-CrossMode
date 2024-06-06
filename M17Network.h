/*
 *   Copyright (C) 2020,2021,2024 by Jonathan Naylor G4KLX
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

#if !defined(M17Network_H)
#define	M17Network_H

#include "M17Defines.h"
#include "RingBuffer.h"
#include "UDPSocket.h"
#include "Network.h"
#include "Timer.h"

#include <random>
#include <cstdint>

// #define	DUMP_M17

class CM17Network : public INetwork {
public:
	CM17Network(NETWORK network, const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug);
	virtual ~CM17Network();

	virtual bool open();

	virtual bool writeRaw(CData& data);
	virtual bool writeData(CData& data);

	virtual bool read(CData& data);
	virtual bool read();

	virtual bool hasData();

	virtual void reset();

	virtual void close();

	virtual void clock(unsigned int ms);

private:
	NETWORK          m_network;
	CUDPSocket       m_socket;
	sockaddr_storage m_addr;
	size_t           m_addrLen;
	bool             m_debug;
	uint16_t         m_outId;
	uint16_t         m_outSeq;
	uint16_t         m_inId;
	CRingBuffer<uint8_t> m_buffer;
	std::mt19937     m_random;
	CTimer           m_timer;
	uint8_t*         m_lich;
	bool             m_hasMeta;
	uint8_t*         m_audio;
	uint8_t          m_audioCount;
#if defined(DUMP_M17)
	FILE*            m_fpIn;
	FILE*            m_fpOut;
#endif

	void sendPing();
	void createLICH(const CData& data);
	void encodeCallsign(const std::string& callsign, uint8_t* encoded) const;
	void decodeCallsign(const uint8_t* encoded, std::string& callsign) const;
};

#endif
