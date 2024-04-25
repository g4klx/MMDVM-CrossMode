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

CFMNetwork::CFMNetwork(const std::string& callsign, const std::string& localAddress, unsigned short localPort, const std::string& gatewayAddress, unsigned short gatewayPort, bool debug) :
m_callsign(callsign),
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_debug(debug),
m_buffer(2000U, "FM Network"),
m_seqNo(0U)
{
	assert(!callsign.empty());
	assert(gatewayPort > 0U);
	assert(!gatewayAddress.empty());

	if (CUDPSocket::lookup(gatewayAddress, gatewayPort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;

	// Remove any trailing letters in the callsign
	size_t pos = callsign.find_first_of(' ');
	if (pos != std::string::npos)
		m_callsign = callsign.substr(0U, pos);
}

CFMNetwork::~CFMNetwork()
{
}

bool CFMNetwork::open()
{
	if (m_addrLen == 0U) {
		LogError("Unable to resolve the address of the FM Gateway");
		return false;
	}

	LogMessage("Opening FM network connection");

	return m_socket.open(m_addr);
}

bool CFMNetwork::write(CData& data)
{
	if (data.isEnd())
		return writeEnd();

	if (m_seqNo == 0U) {
		bool ret = writeStart();
		if (!ret)
			return false;
	}

	uint8_t buffer[500U];
	::memset(buffer, 0x00U, 500U);

	buffer[0U] = 'U';
	buffer[1U] = 'S';
	buffer[2U] = 'R';
	buffer[3U] = 'P';

	// Sequence number
	buffer[4U] = (m_seqNo >> 24) & 0xFFU;
	buffer[5U] = (m_seqNo >> 16) & 0xFFU;
	buffer[6U] = (m_seqNo >> 8)  & 0xFFU;
	buffer[7U] = (m_seqNo >> 0)  & 0xFFU;

	// PTT on
	buffer[15U] = 0x01U;

	data.getData(buffer + 32U);

	if (m_debug)
		CUtils::dump(1U, "FM Network Data Sent", buffer, 32U + PCM_DATA_LENGTH);

	m_seqNo++;

	return m_socket.write(buffer, 32U + PCM_DATA_LENGTH, m_addr, m_addrLen);
}

void CFMNetwork::clock(unsigned int ms)
{
	uint8_t buffer[BUFFER_LENGTH];

	sockaddr_storage addr;
	unsigned int addrlen;
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
	if (::memcmp(buffer, "USRP", 4U) != 0)
		return;

	if (length < 32)
		return;

	// The type is a big-endian 4-byte integer
	unsigned int type = (buffer[20U] << 24) +
						(buffer[21U] << 16) +
						(buffer[22U] << 8)  +
						(buffer[23U] << 0);

	if (type == 0U) {
		uint8_t ptt = (buffer[15U] == 0x01U) ? TAG_DATA : TAG_EOT;
		m_buffer.addData(&ptt, 1U);
		m_buffer.addData(buffer + 32U, PCM_DATA_LENGTH);
	}
}

bool CFMNetwork::read(CData& data)
{
	unsigned int bytes = m_buffer.dataSize();
	if (bytes == 0U)
		return false;

	uint8_t ptt = 0U;
	m_buffer.getData(&ptt, 1U);

	uint8_t buffer[400U];
	m_buffer.getData(buffer, PCM_DATA_LENGTH);

	if (ptt)
		data.setData(buffer);
	else
		data.setEnd();

	return true;
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

bool CFMNetwork::writeStart()
{
	uint8_t buffer[70U];
	::memset(buffer, 0x00U, 70U);

	buffer[0U] = 'U';
	buffer[1U] = 'S';
	buffer[2U] = 'R';
	buffer[3U] = 'P';

	// Sequence number
	buffer[4U] = (m_seqNo >> 24) & 0xFFU;
	buffer[5U] = (m_seqNo >> 16) & 0xFFU;
	buffer[6U] = (m_seqNo >> 8) & 0xFFU;
	buffer[7U] = (m_seqNo >> 0) & 0xFFU;

	buffer[23U] = 0x02U;

	// TLV TAG for Metadata
	buffer[32U] = 0x08U;

	// TLV Length
	buffer[33U] = 3U + 4U + 3U + 1U + 1U + uint8_t(m_callsign.size()) + 1U;

	// Callsign
	unsigned int pos = 46U;
	for (std::string::const_iterator it = m_callsign.cbegin(); it != m_callsign.cend(); ++it)
		buffer[pos++] = *it;

	if (m_debug)
		CUtils::dump(1U, "FM Network Data Sent", buffer, 70U);

	return m_socket.write(buffer, 70U, m_addr, m_addrLen);
}

bool CFMNetwork::writeEnd()
{
	uint8_t buffer[500U];
	::memset(buffer, 0x00U, 500U);

	buffer[0U] = 'U';
	buffer[1U] = 'S';
	buffer[2U] = 'R';
	buffer[3U] = 'P';

	// Sequence number
	buffer[4U] = (m_seqNo >> 24) & 0xFFU;
	buffer[5U] = (m_seqNo >> 16) & 0xFFU;
	buffer[6U] = (m_seqNo >> 8) & 0xFFU;
	buffer[7U] = (m_seqNo >> 0) & 0xFFU;

	if (m_debug)
		CUtils::dump(1U, "FM Network Data Sent", buffer, 32U + PCM_DATA_LENGTH);

	return m_socket.write(buffer, 32U + PCM_DATA_LENGTH, m_addr, m_addrLen);
}
