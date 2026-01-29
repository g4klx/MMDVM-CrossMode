/*
 *   Copyright (C) 2026 by Jonathan Naylor G4KLX
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

#if !defined(TranscoderConnection_H)
#define TranscoderConnection_H

#include "UARTController.h"
#include "RingBuffer.h"
#include "UDPSocket.h"

#include <string>

#include <cstdint>

class CTranscoderConnection {
public:
	CTranscoderConnection(bool debug);
	~CTranscoderConnection();

	void setUARTConnection(const std::string& port, uint32_t speed);
	void setUDPConnection(const std::string& remoteAddress, uint16_t remotePort, const std::string& localAddress, uint16_t localPort);

	bool open();

	uint16_t read(uint8_t* buffer, uint16_t length);

	int16_t  write(const uint8_t* buffer, uint16_t length);

	void close();

private:
	CUARTController*     m_serial;
	CUDPSocket*          m_socket;
	bool                 m_debug;
	sockaddr_storage     m_address;
	size_t               m_addressLength;
	CRingBuffer<uint8_t> m_buffer;
};

#endif
