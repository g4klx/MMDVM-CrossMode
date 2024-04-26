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

#include "M17Network.h"
#include "M17Defines.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

const std::string M17_CHARS = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-/.";

const unsigned int BUFFER_LENGTH = 200U;

CM17Network::CM17Network(const std::string& localAddress, unsigned short localPort, const std::string& remoteAddress, unsigned short remotePort, bool debug) :
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_debug(debug),
m_outId(0U),
m_outSeq(0U),
m_inId(0U),
m_buffer(1000U, "M17 Network"),
m_random(),
m_timer(1000U, 5U),
m_lich(nullptr),
m_hasMeta(false)
{
	if (CUDPSocket::lookup(remoteAddress, remotePort, m_addr, m_addrLen) != 0) {
		m_addrLen = 0U;
		return;
	}

	m_lich = new uint8_t[M17_LICH_LENGTH_BYTES];

	std::random_device rd;
	std::mt19937 mt(rd());
	m_random = mt;
}

CM17Network::~CM17Network()
{
	delete[] m_lich;
}

bool CM17Network::open()
{
	if (m_addrLen == 0U) {
		LogError("M17, unable to resolve the remote address");
		return false;
	}

	LogMessage("Opening M17 network connection");

	bool ret = m_socket.open(m_addr);

	if (ret) {
		m_timer.start();
		return true;
	} else {
		return false;
	}
}

bool CM17Network::write(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	uint8_t buffer[100U];

	buffer[0U] = 'M';
	buffer[1U] = '1';
	buffer[2U] = '7';
	buffer[3U] = ' ';

	// Create a random id for this transmission if needed
	if (m_outId == 0U) {
		std::uniform_int_distribution<uint16_t> dist(0x0001, 0xFFFE);
		m_outId = dist(m_random);

		createLICH(data);
	}

	buffer[4U] = m_outId / 256U;	// Unique session id
	buffer[5U] = m_outId % 256U;

	::memcpy(buffer + 6U, m_lich, M17_LICH_LENGTH_BYTES);

	buffer[34U] = m_outSeq / 256U;
	buffer[35U] = m_outSeq % 256U;
	m_outSeq++;

	if (data.isEnd()) {
		buffer[34U] |= 0x80U;
		::memcpy(buffer + 36U, M17_3200_SILENCE, M17_PAYLOAD_LENGTH_BYTES);
	} else {
		data.getData(buffer + 36U);
	}

	// Dummy CRC
	buffer[52U] = 0x00U;
	buffer[53U] = 0x00U;

	if (m_debug)
		CUtils::dump(1U, "M17 Network Transmitted", buffer, 54U);

	return m_socket.write(buffer, 54U, m_addr, m_addrLen);
}

void CM17Network::clock(unsigned int ms)
{
	m_timer.clock(ms);
	if (m_timer.isRunning() && m_timer.hasExpired()) {
		sendPing();
		m_timer.start();
	}

	uint8_t buffer[BUFFER_LENGTH];

	sockaddr_storage address;
	unsigned int addrLen;
	int length = m_socket.read(buffer, BUFFER_LENGTH, address, addrLen);
	if (length <= 0)
		return;

	if (!CUDPSocket::match(m_addr, address)) {
		LogMessage("M17, packet received from an invalid source");
		return;
	}

	if (m_debug)
		CUtils::dump(1U, "M17 Network Received", buffer, length);

	if (::memcmp(buffer + 0U, "PING", 4U) == 0)
		return;

	if (::memcmp(buffer + 0U, "M17 ", 4U) != 0) {
		CUtils::dump(2U, "M17, received unknown packet", buffer, length);
		return;
	}

	uint16_t id = (buffer[4U] << 8) + (buffer[5U] << 0);
	if (m_inId == 0U) {
		m_inId = id;
	} else {
		if (id != m_inId)
			return;
	}

	m_buffer.addData(buffer + 6U, 46U);
}

bool CM17Network::read(CData& data)
{
	if (m_buffer.isEmpty())
		return false;

	uint8_t buffer[80U];
	m_buffer.getData(buffer, 46U);

	if (!m_hasMeta) {
		std::string src, dst;
		decodeCallsign(buffer + 6U, src);
		decodeCallsign(buffer + 0U, dst);
		data.setM17(src, dst);

		m_hasMeta = true;
	}

	if ((buffer[28U] & 0x80U) == 0x80U)
		data.setEnd();
	else
		data.setData(buffer + 30U);

	return true;
}

void CM17Network::close()
{
	m_socket.close();

	LogMessage("Closing M17 network connection");
}

void CM17Network::reset()
{
	m_outId   = 0U;
	m_outSeq  = 0U;
	m_inId    = 0U;
	m_hasMeta = false;
}

void CM17Network::sendPing()
{
	uint8_t buffer[5U];

	buffer[0U] = 'P';
	buffer[1U] = 'I';
	buffer[2U] = 'N';
	buffer[3U] = 'G';

	if (m_debug)
		CUtils::dump(1U, "M17 Network Transmitted", buffer, 4U);

	m_socket.write(buffer, 4U, m_addr, m_addrLen);
}

void CM17Network::createLICH(const CData& data)
{
	std::string src, dst;
	data.getM17(src, dst);

	encodeCallsign(src, m_lich + 6U);
	encodeCallsign(dst, m_lich + 0U);

	m_lich[13U] = M17_STREAM_TYPE | (M17_DATA_TYPE_VOICE << 1) | (M17_ENCRYPTION_TYPE_NONE << 3);
}

void CM17Network::encodeCallsign(const std::string& callsign, uint8_t* encoded) const
{
	assert(encoded != NULL);

	if (callsign == "ALL" || callsign == "ALL      ") {
		encoded[0U] = 0xFFU;
		encoded[1U] = 0xFFU;
		encoded[2U] = 0xFFU;
		encoded[3U] = 0xFFU;
		encoded[4U] = 0xFFU;
		encoded[5U] = 0xFFU;
		return;
	}

	unsigned int len = callsign.size();
	if (len > 9U)
		len = 9U;

	uint64_t enc = 0ULL;
	for (int i = len - 1; i >= 0; i--) {
		size_t pos = M17_CHARS.find(callsign[i]);
		if (pos == std::string::npos)
			pos = 0ULL;

		enc *= 40ULL;
		enc += pos;
	}

	encoded[0U] = (enc >> 40) & 0xFFU;
	encoded[1U] = (enc >> 32) & 0xFFU;
	encoded[2U] = (enc >> 24) & 0xFFU;
	encoded[3U] = (enc >> 16) & 0xFFU;
	encoded[4U] = (enc >> 8) & 0xFFU;
	encoded[5U] = (enc >> 0) & 0xFFU;
}

void CM17Network::decodeCallsign(const uint8_t* encoded, std::string& callsign) const
{
	assert(encoded != NULL);

	callsign.clear();

	if (encoded[0U] == 0xFFU && encoded[1U] == 0xFFU && encoded[2U] == 0xFFU &&
		encoded[3U] == 0xFFU && encoded[4U] == 0xFFU && encoded[5U] == 0xFFU) {
		callsign = "ALL";
		return;
	}

	uint64_t enc =
		(uint64_t(encoded[0U]) << 40) +
		(uint64_t(encoded[1U]) << 32) +
		(uint64_t(encoded[2U]) << 24) +
		(uint64_t(encoded[3U]) << 16) +
		(uint64_t(encoded[4U]) << 8) +
		(uint64_t(encoded[5U]) << 0);

	if (enc >= 262144000000000ULL)	// 40^9
		return;

	while (enc > 0ULL) {
		callsign += " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-/."[enc % 40ULL];
		enc /= 40ULL;
	}
}
