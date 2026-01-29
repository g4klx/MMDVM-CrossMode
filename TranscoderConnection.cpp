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

#include "TranscoderConnection.h"

#include "Log.h"

#include <cassert>

// For network transfers
const size_t BUFFER_LENGTH = 500;

CTranscoderConnection::CTranscoderConnection(bool debug) :
m_serial(nullptr),
m_socket(nullptr),
m_debug(debug),
m_address(),
m_addressLength(0),
m_buffer(2000U, "Transcoder UDP Buffer")
{
}

CTranscoderConnection::~CTranscoderConnection()
{
	delete m_serial;
	delete m_socket;
}

void CTranscoderConnection::setUARTConnection(const std::string& port, uint32_t speed)
{
	assert(!port.empty());
	assert(speed > 0U);

	m_serial = new CUARTController(port, speed);
}

void CTranscoderConnection::setUDPConnection(const std::string& remoteAddress, uint16_t remotePort, const std::string& localAddress, uint16_t localPort)
{
	assert(!remoteAddress.empty());
	assert(!localAddress.empty());
	assert(remotePort > 0U);
	assert(localPort > 0U);

	int ret = CUDPSocket::lookup(remoteAddress, remotePort, m_address, m_addressLength);
	if (ret != 0)
		return;

	m_socket = new CUDPSocket(localAddress, localPort);
}

bool CTranscoderConnection::open()
{
	if (m_serial != nullptr)
		return m_serial->open();

	if (m_socket != nullptr)
		return m_socket->open();

	LogError("No connection type specified for the transcoder - open");

	return false;
}

uint16_t CTranscoderConnection::read(uint8_t* buffer, uint16_t length)
{
	assert(buffer != nullptr);
	assert(length > 0U);

	if (m_serial != nullptr)
		return m_serial->read(buffer, length);

	if (m_socket != nullptr) {
		uint8_t data[BUFFER_LENGTH];
		sockaddr_storage address;
		size_t addressLength;

		// Get any network data that is waiting, and store it
		int ret = m_socket->read(data, BUFFER_LENGTH, address, addressLength);
		if (ret > 0)
			m_buffer.add(data, ret);

		uint16_t size = m_buffer.size();

		// Is there any socket data?
		if (size == 0U)
			return 0U;

		// Is there enough socket data?
		if (size < length)
			length = size;

		m_buffer.get(buffer, length);

		return length;
	}

	LogError("No connection type specified for the transcoder - read");

	return 0U;
}

int16_t CTranscoderConnection::write(const uint8_t* buffer, uint16_t length)
{
	assert(buffer != nullptr);
	assert(length > 0U);

	if (m_serial != nullptr)
		return m_serial->write(buffer, length);

	if (m_socket != nullptr) {
		bool ret = m_socket->write(buffer, length, m_address, m_addressLength);
		return ret ? length : -1;
	}

	LogError("No connection type specified for the transcoder - write");

	return -1;
}

void CTranscoderConnection::close()
{
	m_buffer.clear();

	if (m_serial != nullptr) {
		m_serial->close();
		return;
	}

	if (m_socket != nullptr) {
		m_socket->close();
		return;
	}

	LogError("No connection type specified for the transcoder - close");
}
