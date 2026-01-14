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

#include "FMNetwork.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const unsigned int BUFFER_LENGTH = 1500U;

CFMNetwork::CFMNetwork(NETWORK network, const std::string& callsign, const std::string& localAddress, unsigned short localPort, const std::string& gatewayAddress, unsigned short gatewayPort, bool debug) :
m_network(network),
m_callsign(callsign),
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_debug(debug),
m_buffer(3000U, "FM Network"),
m_seqNo(0U)
{
	assert(gatewayPort > 0U);
	assert(!gatewayAddress.empty());

	if (CUDPSocket::lookup(gatewayAddress, gatewayPort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;
}

CFMNetwork::~CFMNetwork()
{
}

bool CFMNetwork::open()
{
	if (m_addrLen == 0U) {
		LogError("Unable to resolve the address of the remote");
		return false;
	}

	LogMessage("Opening FM network connection");

	return m_socket.open(m_addr);
}

bool CFMNetwork::writeRaw(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	uint8_t buffer[500U];
	uint16_t length = data.getRaw(buffer);

	if (length == 0U)
		return true;

	if (m_debug)
		CUtils::dump(1U, "FM Network Raw Sent", buffer, length);

	return m_socket.write(buffer, length, m_addr, m_addrLen);
}

bool CFMNetwork::writeData(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	if (data.isEnd())
		return writeEnd();

	if (m_seqNo == 0U) {
		bool ret = writeStart(data);
		if (!ret)
			return false;
	}

	uint8_t buffer[500U];
	::memset(buffer, 0x00U, 500U);

	buffer[0U] = 'F';
	buffer[1U] = 'M';
	buffer[2U] = 'D';

	data.getData(buffer + 3U);

	if (m_debug)
		CUtils::dump(1U, "FM Network Data Sent", buffer, 3U + PCM_DATA_LENGTH);

	m_seqNo++;

	return m_socket.write(buffer, 3U + PCM_DATA_LENGTH, m_addr, m_addrLen);
}

void CFMNetwork::clock(unsigned int ms)
{
	uint8_t buffer[BUFFER_LENGTH];

	sockaddr_storage addr;
	size_t addrlen;
	int length = m_socket.read(buffer, BUFFER_LENGTH, addr, addrlen);
	if (length <= 0)
		return;

	// Check if the data is for us
	if (!CUDPSocket::match(addr, m_addr, IMT_ADDRESS_AND_PORT)) {
		LogMessage("FM packet received from an invalid source");
		return;
	}

	if (m_debug)
		CUtils::dump(1U, "FM Network Data Received", buffer, length);

	// Invalid packet type?
	if (::memcmp(buffer, "FM", 2U) != 0)
		return;

	uint16_t len = length;
	m_buffer.add((uint8_t*)&len, sizeof(uint16_t));
	m_buffer.add(buffer, len);
}

bool CFMNetwork::read(CData& data)
{
	if (m_buffer.empty())
		return false;

	uint16_t length = 0U;
	m_buffer.get((uint8_t*)&length, sizeof(uint16_t));

	uint8_t buffer[BUFFER_LENGTH];
	m_buffer.get(buffer, length);

	data.setFM(m_network);

	data.setRaw(buffer, length);

	if (::memcmp(buffer + 0U, "FMD", 3U) == 0)
		data.setData(buffer + 3U);
	else if (::memcmp(buffer + 0U, "FME", 3U) == 0)
		data.setEnd();
	else if (::memcmp(buffer + 0U, "FMS", 3U) == 0)
		data.setFM(buffer + 3U);

	return true;
}

bool CFMNetwork::read()
{
	if (m_buffer.empty())
		return false;

	uint16_t length = 0U;
	m_buffer.get((uint8_t*)&length, sizeof(uint16_t));

	uint8_t buffer[BUFFER_LENGTH];
	m_buffer.get(buffer, length);

	return true;
}

bool CFMNetwork::hasData()
{
	return m_buffer.hasData();
}

void CFMNetwork::reset()
{
	m_seqNo = 0U;
	m_buffer.clear();
}

void CFMNetwork::close()
{
	m_socket.close();

	LogMessage("Closing FM network connection");
}

bool CFMNetwork::writeStart(CData& data)
{
	uint8_t buffer[20U];
	::memset(buffer, 0x00U, 20U);

	buffer[0U] = 'F';
	buffer[1U] = 'M';
	buffer[2U] = 'S';

	data.getFM(buffer + 3U);

	uint16_t length = uint16_t(::strlen((char*)buffer));

	if (m_debug)
		CUtils::dump(1U, "FM Network Data Sent", buffer, length + 1U);

	return m_socket.write(buffer, length + 1U, m_addr, m_addrLen);
}

bool CFMNetwork::writeEnd()
{
	uint8_t buffer[10U];

	buffer[0U] = 'F';
	buffer[1U] = 'M';
	buffer[2U] = 'E';

	if (m_debug)
		CUtils::dump(1U, "FM Network Data Sent", buffer, 3U);

	return m_socket.write(buffer, 3U, m_addr, m_addrLen);
}
