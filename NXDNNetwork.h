/*
 *   Copyright (C) 2009-2014,2016,2018,2020,2024 by Jonathan Naylor G4KLX
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

#ifndef	NXDNNetwork_H
#define	NXDNNetwork_H

#include "Network.h"
#include "NXDNLayer3.h"

#include "RingBuffer.h"
#include "UDPSocket.h"

#include <cstdint>
#include <string>

class CNXDNNetwork : public INetwork {
public:
	CNXDNNetwork(NETWORK network, const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug);
	virtual ~CNXDNNetwork();

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
	CRingBuffer<uint8_t> m_buffer;
	uint16_t         m_seqNo;
	uint8_t*         m_audio;
	uint8_t          m_audioCount;
	uint8_t          m_maxAudio;
	CNXDNLayer3      m_layer3;

	bool writeHeader(CData& data);
	bool writeBody(CData& data);
	bool writeTrailer(CData& data);
};

#endif
