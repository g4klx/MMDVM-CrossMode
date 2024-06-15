/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021,2024 by Jonathan Naylor G4KLX
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

#if !defined(DMRNetwork_H)
#define	DMRNetwork_H

#include "Network.h"
#include "Defines.h"
#include "UDPSocket.h"
#include "Timer.h"
#include "RingBuffer.h"
#include "Data.h"

#include <string>
#include <cstdint>
#include <random>

// #define	DUMP_DMR

class CDMRNetwork : public INetwork
{
public:
	CDMRNetwork(NETWORK network, uint32_t id, const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug);
	virtual ~CDMRNetwork();

	virtual bool open();

	virtual bool writeRaw(CData& data);
	virtual bool writeData(CData& data);

	virtual bool read(CData& data);
	virtual bool read();

	virtual bool hasData();

	virtual void reset();

	virtual void clock(unsigned int ms);

	virtual void close();

private:
	NETWORK          m_network;
	CUDPSocket       m_socket;
	sockaddr_storage m_addr;
	size_t           m_addrLen;
	uint8_t*         m_id;
	bool             m_debug;
	uint8_t*         m_buffer;
	uint32_t         m_streamId;
	CRingBuffer<uint8_t> m_rxData;
	std::mt19937     m_random;
	CTimer           m_pingTimer;
	uint8_t*         m_audio;
	uint8_t          m_audioCount;
	uint16_t         m_seqNo;
	uint8_t          m_N;
#if defined(DUMP_DMR)
	FILE*            m_fpIn;
	FILE*            m_fpOut;
#endif

	bool writeHeader(CData& data);
	bool writeAudio(CData& data);
	bool writeTrailer(CData& data);
};

#endif
