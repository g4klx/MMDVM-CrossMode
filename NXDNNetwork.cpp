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

#include "NXDNNetwork.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

const unsigned int BUFFER_LENGTH = 200U;

CNXDNNetwork::CNXDNNetwork(const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug) :
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_debug(debug),
m_buffer(1000U, "NXDN Network")
{
	assert(localPort > 0U);
	assert(!remoteAddress.empty());
	assert(remotePort > 0U);

	if (CUDPSocket::lookup(remoteAddress, remotePort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;
}

CNXDNNetwork::~CNXDNNetwork()
{
}

bool CNXDNNetwork::open()
{
	if (m_addrLen == 0U) {
		LogError("Unable to resolve the address of the NXDN network");
		return false;
	}

	LogMessage("Opening NXDN connection");

	return m_socket.open(m_addr);
}

bool CNXDNNetwork::writeRaw(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	uint8_t buffer[100U];
	uint16_t length = data.getRaw(buffer);

	if (length == 0U)
		return true;

	if (m_debug)
		CUtils::dump(1U, "NXDN Network Raw Sent", buffer, length);

	return m_socket.write(buffer, length, m_addr, m_addrLen);
}

bool CNXDNNetwork::writeData(CData& data)
{
	unsigned char buffer[110U];
	::memset(buffer, 0x00U, 110U);

	buffer[0U] = 'I';
	buffer[1U] = 'C';
	buffer[2U] = 'O';
	buffer[3U] = 'M';
	buffer[4U] = 0x01U;
	buffer[5U] = 0x01U;
	buffer[6U] = 0x08U;
	buffer[7U] = 0xE0U;

	if ((data[0U] & 0xF0U) == 0x90U) {
		buffer[37U] = 0x23U;
		buffer[38U] = 0x02U;
		buffer[39U] = 0x18U;
	} else {
		buffer[37U] = 0x23U;
		buffer[38U] = (data[0U] == 0x81U || data[0U] == 0x83U) ? 0x1CU : 0x10U;
		buffer[39U] = 0x21U;
	}

	::memcpy(buffer + 40U, data, length);

	if (m_debug)
		CUtils::dump(1U, "NXDN Data Sent", buffer, 102U);

	return m_socket.write(buffer, 102U, m_addr, m_addrLen);
}

void CNXDNNetwork::clock(unsigned int ms)
{
	uint8_t buffer[BUFFER_LENGTH];
	sockaddr_storage addr;
	size_t addrlen;

	int length = m_socket.read(buffer, BUFFER_LENGTH, addr, addrlen);
	if (length <= 0)
		return;

	if (!CUDPSocket::match(m_addr, addr, IMT_ADDRESS_AND_PORT)) {
		LogWarning("NXDN Data received from an unknown address");
		return;
	}

	// Invalid packet type?
	if (::memcmp(buffer, "ICOM", 4U) != 0)
		return;

	if (m_debug)
		CUtils::dump(1U, "NXDN Data Received", buffer, length);

	uint8_t c = length;
	m_buffer.add(&c, 1U);

	m_buffer.add(buffer, length);
}

bool CNXDNNetwork::read(CData& data)
{
	if (m_buffer.empty())
		return 0U;

	uint8_t length = 0U;
	m_buffer.get(&length, 1U);

	uint8_t buffer[BUFFER_LENGTH];
	m_buffer.get(buffer, length);

	data.setRaw(buffer, length);

	// An NXDN repeater connect request
	if (buffer[4U] == 0x01U && buffer[5U] == 0x61U) {
		buffer[5U]  = 0x62U;
		buffer[37U] = 0x02U;
		buffer[38U] = 0x4FU;
		buffer[39U] = 0x4BU;
		m_socket.write(buffer, length, m_addr, m_addrLen);
		return false;
	}

	if (length != 102U)
		return 0U;

	::memcpy(data, buffer + 40U, 33U);

	return 33U;
}

bool CNXDNNetwork::read()
{
	if (m_buffer.empty())
		return false;

	uint8_t length = 0U;
	m_buffer.get(&length, 1U);

	uint8_t buffer[BUFFER_LENGTH];
	m_buffer.get(buffer, length);

	return true;
}

void CNXDNNetwork::reset()
{
	m_buffer.clear();
	m_audioCount = 0U;
}

bool CNXDNNetwork::hasData()
{
	return m_buffer.hasData() || (m_audioCount > 0U);
}

void CNXDNNetwork::close()
{
	m_socket.close();

	LogMessage("Closing NXDN connection");
}
