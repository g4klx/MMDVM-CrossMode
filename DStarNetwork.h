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

#ifndef	DStarNetwork_H
#define	DStarNetwork_H

#include "DStarDefines.h"
#include "RingBuffer.h"
#include "UDPSocket.h"
#include "Timer.h"

#include <cstdint>
#include <string>
#include <random>

class CDStarNetwork {
public:
	CDStarNetwork(const std::string& gatewayAddress, unsigned short gatewayPort, const std::string& localAddress, unsigned short localPort, bool debug);
	~CDStarNetwork();

	bool open();

	bool writeHeader(const uint8_t* header, unsigned int length, bool busy);
	bool writeData(const uint8_t* data, unsigned int length, unsigned int errors, bool end, bool busy);

	unsigned int read(uint8_t* data, unsigned int length);

	void reset();

	void close();

	void clock(unsigned int ms);

private:
	CUDPSocket       m_socket;
	sockaddr_storage m_addr;
	unsigned int     m_addrLen;
	bool             m_debug;
	uint16_t         m_outId;
	uint8_t          m_outSeq;
	uint16_t         m_inId;
	CRingBuffer<uint8_t> m_buffer;
	CTimer           m_pollTimer;
	std::mt19937     m_random;

	bool writePoll(const char* text);
};

#endif
