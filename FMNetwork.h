/*
 *   Copyright (C) 2020,2021,2023,2024 by Jonathan Naylor G4KLX
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

#if !defined(FMNetwork_H)
#define	FMNetwork_H

#include "RingBuffer.h"
#include "UDPSocket.h"
#include "Network.h"

#include <cstdint>
#include <string>


class CFMNetwork : public INetwork {
public:
	CFMNetwork(NETWORK network, const std::string& callsign, const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug);
	virtual ~CFMNetwork();

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
	std::string      m_callsign;
	CUDPSocket       m_socket;
	sockaddr_storage m_addr;
	size_t           m_addrLen;
	bool             m_debug;
	CRingBuffer<uint8_t> m_buffer;
	unsigned int     m_seqNo;

	bool writeStart(CData& data);
	bool writeEnd();
};

#endif

