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

const uint16_t CCITT16_TABLE[] = {
	0x0000U, 0x1189U, 0x2312U, 0x329bU, 0x4624U, 0x57adU, 0x6536U, 0x74bfU,
	0x8c48U, 0x9dc1U, 0xaf5aU, 0xbed3U, 0xca6cU, 0xdbe5U, 0xe97eU, 0xf8f7U,
	0x1081U, 0x0108U, 0x3393U, 0x221aU, 0x56a5U, 0x472cU, 0x75b7U, 0x643eU,
	0x9cc9U, 0x8d40U, 0xbfdbU, 0xae52U, 0xdaedU, 0xcb64U, 0xf9ffU, 0xe876U,
	0x2102U, 0x308bU, 0x0210U, 0x1399U, 0x6726U, 0x76afU, 0x4434U, 0x55bdU,
	0xad4aU, 0xbcc3U, 0x8e58U, 0x9fd1U, 0xeb6eU, 0xfae7U, 0xc87cU, 0xd9f5U,
	0x3183U, 0x200aU, 0x1291U, 0x0318U, 0x77a7U, 0x662eU, 0x54b5U, 0x453cU,
	0xbdcbU, 0xac42U, 0x9ed9U, 0x8f50U, 0xfbefU, 0xea66U, 0xd8fdU, 0xc974U,
	0x4204U, 0x538dU, 0x6116U, 0x709fU, 0x0420U, 0x15a9U, 0x2732U, 0x36bbU,
	0xce4cU, 0xdfc5U, 0xed5eU, 0xfcd7U, 0x8868U, 0x99e1U, 0xab7aU, 0xbaf3U,
	0x5285U, 0x430cU, 0x7197U, 0x601eU, 0x14a1U, 0x0528U, 0x37b3U, 0x263aU,
	0xdecdU, 0xcf44U, 0xfddfU, 0xec56U, 0x98e9U, 0x8960U, 0xbbfbU, 0xaa72U,
	0x6306U, 0x728fU, 0x4014U, 0x519dU, 0x2522U, 0x34abU, 0x0630U, 0x17b9U,
	0xef4eU, 0xfec7U, 0xcc5cU, 0xddd5U, 0xa96aU, 0xb8e3U, 0x8a78U, 0x9bf1U,
	0x7387U, 0x620eU, 0x5095U, 0x411cU, 0x35a3U, 0x242aU, 0x16b1U, 0x0738U,
	0xffcfU, 0xee46U, 0xdcddU, 0xcd54U, 0xb9ebU, 0xa862U, 0x9af9U, 0x8b70U,
	0x8408U, 0x9581U, 0xa71aU, 0xb693U, 0xc22cU, 0xd3a5U, 0xe13eU, 0xf0b7U,
	0x0840U, 0x19c9U, 0x2b52U, 0x3adbU, 0x4e64U, 0x5fedU, 0x6d76U, 0x7cffU,
	0x9489U, 0x8500U, 0xb79bU, 0xa612U, 0xd2adU, 0xc324U, 0xf1bfU, 0xe036U,
	0x18c1U, 0x0948U, 0x3bd3U, 0x2a5aU, 0x5ee5U, 0x4f6cU, 0x7df7U, 0x6c7eU,
	0xa50aU, 0xb483U, 0x8618U, 0x9791U, 0xe32eU, 0xf2a7U, 0xc03cU, 0xd1b5U,
	0x2942U, 0x38cbU, 0x0a50U, 0x1bd9U, 0x6f66U, 0x7eefU, 0x4c74U, 0x5dfdU,
	0xb58bU, 0xa402U, 0x9699U, 0x8710U, 0xf3afU, 0xe226U, 0xd0bdU, 0xc134U,
	0x39c3U, 0x284aU, 0x1ad1U, 0x0b58U, 0x7fe7U, 0x6e6eU, 0x5cf5U, 0x4d7cU,
	0xc60cU, 0xd785U, 0xe51eU, 0xf497U, 0x8028U, 0x91a1U, 0xa33aU, 0xb2b3U,
	0x4a44U, 0x5bcdU, 0x6956U, 0x78dfU, 0x0c60U, 0x1de9U, 0x2f72U, 0x3efbU,
	0xd68dU, 0xc704U, 0xf59fU, 0xe416U, 0x90a9U, 0x8120U, 0xb3bbU, 0xa232U,
	0x5ac5U, 0x4b4cU, 0x79d7U, 0x685eU, 0x1ce1U, 0x0d68U, 0x3ff3U, 0x2e7aU,
	0xe70eU, 0xf687U, 0xc41cU, 0xd595U, 0xa12aU, 0xb0a3U, 0x8238U, 0x93b1U,
	0x6b46U, 0x7acfU, 0x4854U, 0x59ddU, 0x2d62U, 0x3cebU, 0x0e70U, 0x1ff9U,
	0xf78fU, 0xe606U, 0xd49dU, 0xc514U, 0xb1abU, 0xa022U, 0x92b9U, 0x8330U,
	0x7bc7U, 0x6a4eU, 0x58d5U, 0x495cU, 0x3de3U, 0x2c6aU, 0x1ef1U, 0x0f78U };

const unsigned int BUFFER_LENGTH = 100U;

CDStarNetwork::CDStarNetwork(const std::string& callsign, const std::string& localAddress, unsigned short localPort, const std::string& remoteAddress, unsigned short remotePort, bool debug) :
m_callsign(callsign),
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_debug(debug),
m_outId(0U),
m_outSeq(0U),
m_inId(0U),
m_buffer(1000U, "D-Star Network"),
m_pollTimer(1000U, 60U),
m_random(),
m_header(nullptr)
{
	assert(!callsign.empty());
	assert(remotePort > 0U);

	if (CUDPSocket::lookup(remoteAddress, remotePort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;

	m_header = new uint8_t[DSTAR_HEADER_LENGTH_BYTES];

	std::random_device rd;
	std::mt19937 mt(rd());
	m_random = mt;
}

CDStarNetwork::~CDStarNetwork()
{
	delete[] m_header;
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

bool CDStarNetwork::write(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	if (m_outId == 0U)
		return writeHeader(data);
	else
		return writeData(data);
}

bool CDStarNetwork::writeHeader(const CData& data)
{
	createHeader(data);

	uint8_t buffer[50U];

	buffer[0] = 'D';
	buffer[1] = 'S';
	buffer[2] = 'R';
	buffer[3] = 'P';

	buffer[4] = 0x20U;

	// Create a random id for this transmission
	std::uniform_int_distribution<uint16_t> dist(0x0001, 0xfffe);
	m_outId = dist(m_random);

	buffer[5] = m_outId / 256U;	// Unique session id
	buffer[6] = m_outId % 256U;

	buffer[7] = 0U;

	::memcpy(buffer + 8U, m_header, DSTAR_HEADER_LENGTH_BYTES);

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

bool CDStarNetwork::writeData(CData& data)
{
	uint8_t buffer[30U];

	buffer[0] = 'D';
	buffer[1] = 'S';
	buffer[2] = 'R';
	buffer[3] = 'P';

	buffer[4] = 0x21U;

	buffer[5] = m_outId / 256U;	// Unique session id
	buffer[6] = m_outId % 256U;

	unsigned int length = 9U;

	buffer[7] = m_outSeq;
	if (data.isEnd()) {
		buffer[7] |= 0x40U;			// End of data marker
		::memcpy(buffer + 9U, DSTAR_END_PATTERN_BYTES, DSTAR_END_PATTERN_LENGTH_BYTES);
		length += DSTAR_END_PATTERN_LENGTH_BYTES;
	} else {
		data.getData(buffer + 9U);
		addSlowData(buffer + 18U);
		length += DSTAR_VOICE_FRAME_LENGTH_BYTES + DSTAR_DATA_FRAME_LENGTH_BYTES;
	}

	buffer[8] = 0U;

	if (m_debug)
		CUtils::dump(1U, "D-Star Network Data Sent", buffer, length);

	return m_socket.write(buffer, length, m_addr, m_addrLen);
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
		data.setDStar(buffer + 27U, buffer + 19U);
		return false;
	case TAG_DATA:
		data.setData(buffer + 1U);
		return true;
	case TAG_EOT:
		data.setEnd();
		return true;
	default:
		return false;
	}
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

void CDStarNetwork::createHeader(const CData& data)
{
	::memset(m_header, ' ', DSTAR_HEADER_LENGTH_BYTES);

	m_header[0U] = 0x00U;
	m_header[1U] = 0x00U;
	m_header[2U] = 0x00U;

	data.getDStar(m_header + 27U, m_header + 19U);

	uint8_t rpt[DSTAR_LONG_CALLSIGN_LENGTH];

	stringToBytes(rpt, m_callsign);
	::memcpy(m_header + 3U, rpt, DSTAR_LONG_CALLSIGN_LENGTH);

	rpt[DSTAR_LONG_CALLSIGN_LENGTH - 1U] = 'G';
	::memcpy(m_header + 11U, rpt, DSTAR_LONG_CALLSIGN_LENGTH);

	addHeaderCRC();
}

void CDStarNetwork::addSlowData(uint8_t* buffer)
{
	assert(buffer != nullptr);

	switch (m_outSeq) {
	case 0U:
		::memcpy(buffer, DSTAR_SYNC_BYTES, DSTAR_DATA_FRAME_LENGTH_BYTES);
		m_outSeq++;
		return;

	case 1U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 5U;
		buffer[1U] = m_header[0U];
		buffer[2U] = m_header[1U];
		break;

	case 2U:
		buffer[0U] = m_header[2U];
		buffer[1U] = m_header[3U];
		buffer[2U] = m_header[4U];
		break;

	case 3U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 5U;
		buffer[1U] = m_header[5U];
		buffer[2U] = m_header[6U];
		break;

	case 4U:
		buffer[0U] = m_header[7U];
		buffer[1U] = m_header[8U];
		buffer[2U] = m_header[9U];
		break;

	case 5U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 5U;
		buffer[1U] = m_header[10U];
		buffer[2U] = m_header[11U];
		break;

	case 6U:
		buffer[0U] = m_header[12U];
		buffer[1U] = m_header[13U];
		buffer[2U] = m_header[14U];
		break;

	case 7U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 5U;
		buffer[1U] = m_header[15U];
		buffer[2U] = m_header[16U];
		break;

	case 8U:
		buffer[0U] = m_header[17U];
		buffer[1U] = m_header[18U];
		buffer[2U] = m_header[19U];
		break;

	case 9U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 5U;
		buffer[1U] = m_header[20U];
		buffer[2U] = m_header[21U];
		break;

	case 10U:
		buffer[0U] = m_header[22U];
		buffer[1U] = m_header[23U];
		buffer[2U] = m_header[24U];
		break;

	case 11U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 5U;
		buffer[1U] = m_header[25U];
		buffer[2U] = m_header[26U];
		break;

	case 12U:
		buffer[0U] = m_header[27U];
		buffer[1U] = m_header[28U];
		buffer[2U] = m_header[29U];
		break;

	case 13U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 5U;
		buffer[1U] = m_header[30U];
		buffer[2U] = m_header[31U];
		break;

	case 14U:
		buffer[0U] = m_header[32U];
		buffer[1U] = m_header[33U];
		buffer[2U] = m_header[34U];
		break;

	case 15U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 5U;
		buffer[1U] = m_header[35U];
		buffer[2U] = m_header[36U];
		break;

	case 16U:
		buffer[0U] = m_header[37U];
		buffer[1U] = m_header[38U];
		buffer[2U] = m_header[39U];
		break;

	case 17U:
		buffer[0U] = DSTAR_SLOW_DATA_TYPE_HEADER | 1U;
		buffer[1U] = m_header[40U];
		buffer[2U] = 'f';
		break;

	default:
		buffer[0U] = 'f';
		buffer[1U] = 'f';
		buffer[2U] = 'f';
		break;
	}

	buffer[0U] ^= DSTAR_SCRAMBLER_BYTES[0U];
	buffer[1U] ^= DSTAR_SCRAMBLER_BYTES[1U];
	buffer[2U] ^= DSTAR_SCRAMBLER_BYTES[2U];

	m_outSeq++;
	if (m_outSeq > 0x14U)
		m_outSeq = 0U;
}

void CDStarNetwork::stringToBytes(uint8_t* str, const std::string& callsign) const
{
	assert(str != nullptr);

	::memset(str, ' ', DSTAR_LONG_CALLSIGN_LENGTH);

	unsigned int len = callsign.length();
	if (len > DSTAR_LONG_CALLSIGN_LENGTH)
		len = DSTAR_LONG_CALLSIGN_LENGTH;

	for (unsigned int i = 0U; i < len; i++)
		str[i] = callsign[i];
}

void CDStarNetwork::addHeaderCRC()
{
	union {
		uint16_t crc16;
		uint8_t  crc8[2U];
	};

	crc16 = 0xFFFFU;

	for (unsigned int i = 0U; i < (DSTAR_HEADER_LENGTH_BYTES - 2U); i++)
		crc16 = uint16_t(crc8[1U]) ^ CCITT16_TABLE[crc8[0U] ^ m_header[i]];

	crc16 = ~crc16;

	m_header[DSTAR_HEADER_LENGTH_BYTES - 2U] = crc8[0U];
	m_header[DSTAR_HEADER_LENGTH_BYTES - 1U] = crc8[1U];
}
