/*
 *   Copyright (C) 2009-2014,2016,2019,2020,2021,2024 by Jonathan Naylor G4KLX
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

#include "DStarDefines.h"
#include "DStarNetwork.h"
#include "StopWatch.h"
#include "Defines.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

const unsigned int BUFFER_LENGTH = 100U;

CDStarNetwork::CDStarNetwork(const std::string& localAddress, unsigned short localPort, const std::string& remoteAddress, unsigned short remotePort, bool debug) :
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_debug(debug),
m_outId(0U),
m_outSeq(0U),
m_inId(0U),
m_buffer(1000U, "D-Star Network"),
m_pollTimer(1000U, 60U),
m_random()
{
	if (CUDPSocket::lookup(remoteAddress, remotePort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;

	std::random_device rd;
	std::mt19937 mt(rd());
	m_random = mt;
}

CDStarNetwork::~CDStarNetwork()
{
}

bool CDStarNetwork::open()
{
	if (m_addrLen == 0U) {
		LogError("Unable to resolve the address of the remote");
		return false;
	}

	LogMessage("Opening D-Star network connection");

	m_pollTimer.start();

	return m_socket.open(m_addr);
}

bool CDStarNetwork::writeHeader(const uint8_t* header, unsigned int length, bool busy)
{
	assert(header != NULL);

	uint8_t buffer[50U];

	buffer[0] = 'D';
	buffer[1] = 'S';
	buffer[2] = 'R';
	buffer[3] = 'P';

	buffer[4] = busy ? 0x22U : 0x20U;

	// Create a random id for this transmission
	std::uniform_int_distribution<uint16_t> dist(0x0001, 0xfffe);
	m_outId = dist(m_random);

	buffer[5] = m_outId / 256U;	// Unique session id
	buffer[6] = m_outId % 256U;

	buffer[7] = 0U;

	::memcpy(buffer + 8U, header, length);

	m_outSeq = 0U;

	if (m_debug)
		CUtils::dump(1U, "D-Star Network Header Sent", buffer, 49U);

	for (unsigned int i = 0U; i < 2U; i++) {
		bool ret = m_socket.write(buffer, 49U, m_addr, m_addrLen);
		if (!ret)
			return false;
	}

	return true;
}

bool CDStarNetwork::writeData(const uint8_t* data, unsigned int length, unsigned int errors, bool end, bool busy)
{
	assert(data != NULL);

	uint8_t buffer[30U];

	buffer[0] = 'D';
	buffer[1] = 'S';
	buffer[2] = 'R';
	buffer[3] = 'P';

	buffer[4] = busy ? 0x23U :  0x21U;

	buffer[5] = m_outId / 256U;	// Unique session id
	buffer[6] = m_outId % 256U;

	// If this is a data sync, reset the sequence to zero
	if (data[9] == 0x55 && data[10] == 0x2D && data[11] == 0x16)
		m_outSeq = 0U;

	buffer[7] = m_outSeq;
	if (end)
		buffer[7] |= 0x40U;			// End of data marker

	buffer[8] = errors;

	m_outSeq++;
	if (m_outSeq > 0x14U)
		m_outSeq = 0U;

	::memcpy(buffer + 9U, data, length);

	if (m_debug)
		CUtils::dump(1U, "D-Star Network Data Sent", buffer, length + 9U);

	return m_socket.write(buffer, length + 9U, m_addr, m_addrLen);
}

bool CDStarNetwork::writePoll(const char* text)
{
	assert(text != NULL);

	uint8_t buffer[40U];

	buffer[0] = 'D';
	buffer[1] = 'S';
	buffer[2] = 'R';
	buffer[3] = 'P';

	buffer[4] = 0x0A;				// Poll with text

	unsigned int length = ::strlen(text);

	// Include the nul at the end also
	::memcpy(buffer + 5U, text, length + 1U);

	// if (m_debug)
	//	CUtils::dump(1U, "D-Star Network Poll Sent", buffer, 6U + length);

	return m_socket.write(buffer, 6U + length, m_addr, m_addrLen);
}

void CDStarNetwork::clock(unsigned int ms)
{
	m_pollTimer.clock(ms);
	if (m_pollTimer.hasExpired()) {
		writePoll("cross-mode");
		m_pollTimer.start();
	}

	uint8_t buffer[BUFFER_LENGTH];

	sockaddr_storage address;
	unsigned int addrLen;
	int length = m_socket.read(buffer, BUFFER_LENGTH, address, addrLen);
	if (length <= 0)
		return;

	if (!CUDPSocket::match(m_addr, address)) {
		LogMessage("D-Star, packet received from an invalid source");
		return;
	}

	// Invalid packet type?
	if (::memcmp(buffer, "DSRP", 4U) != 0)
		return;

	switch (buffer[4]) {
	case 0x00U:			// NETWORK_TEXT;
	case 0x01U:			// NETWORK_TEMPTEXT;
	case 0x04U:			// NETWORK_STATUS1..5
	case 0x24U:			// NETWORK_DD_DATA
		return;

	case 0x20U:			// NETWORK_HEADER
		if (m_inId == 0U) {
			if (m_debug)
				CUtils::dump(1U, "D-Star Network Header Received", buffer, length);

			m_inId = buffer[5] * 256U + buffer[6];

			uint8_t c = length - 7U;
			m_buffer.addData(&c, 1U);

			c = TAG_HEADER;
			m_buffer.addData(&c, 1U);

			m_buffer.addData(buffer + 8U, length - 8U);
		}
		break;

	case 0x21U: {			// NETWORK_DATA
		if (m_debug)
			CUtils::dump(1U, "D-Star Network Data Received", buffer, length);

		uint16_t id = buffer[5] * 256U + buffer[6];

		// Check that the stream id matches the valid header, reject otherwise
		if (id == m_inId) {
			uint8_t ctrl[3U];

			ctrl[0U] = length - 7U;

			// Is this the last packet in the stream?
			if ((buffer[7] & 0x40U) == 0x40U) {
				m_inId = 0U;
				ctrl[1U] = TAG_EOT;
			} else {
				ctrl[1U] = TAG_DATA;
			}

			ctrl[2U] = buffer[7] & 0x3FU;

			m_buffer.addData(ctrl, 3U);

			m_buffer.addData(buffer + 9U, length - 9U);
		}
		}
		break;

	default:
		CUtils::dump("Unknown D-Star packet from the remote", buffer, length);
		break;
	}
}

bool CDStarNetwork::read(CData& data)
{
	if (m_buffer.isEmpty())
		return false;

	uint8_t length = 0U;
	m_buffer.getData(&length, 1U);

	uint8_t type = 0U;
	m_buffer.getData(&type, 1U);

	uint8_t buffer[100U];
	m_buffer.getData(buffer, length - 1U);

	switch (type) {
	case TAG_HEADER:
		data.setDStar();
		break;
	case TAG_DATA:
		data.setData();
		break;
	case TAG_EOT:
		data.setEnd();
		break;
	default:
		return false;
	}

	return true;
}

void CDStarNetwork::reset()
{
	m_inId  = 0U;
	m_outId = 0U;
}

void CDStarNetwork::close()
{
	m_socket.close();

	LogMessage("Closing D-Star network connection");
}
