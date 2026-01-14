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

#include "P25Network.h"

#include "TranscoderDefines.h"
#include "P25Defines.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

const unsigned int BUFFER_LENGTH = 1500U;

CP25Network::CP25Network(NETWORK network, const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug) :
m_network(network),
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_debug(debug),
m_buffer(1000U, "P25 Network"),
m_srcId(0U),
m_dstId(0U),
m_n(0x62U)
{
	assert(localPort > 0U);
	assert(!remoteAddress.empty());
	assert(remotePort > 0U);

	if (CUDPSocket::lookup(remoteAddress, remotePort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;
}

CP25Network::~CP25Network()
{
}

bool CP25Network::open()
{
	if (m_addrLen == 0U) {
		LogError("Unable to resolve the address of the P25 Gateway");
		return false;
	}

	LogMessage("Opening P25 network connection");

	return m_socket.open(m_addr);
}

bool CP25Network::writeRaw(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	uint8_t buffer[100U];
	uint16_t length = data.getRaw(buffer);

	if (length == 0U)
		return true;

	if (m_debug)
		CUtils::dump(1U, "P25 Network Raw Sent", buffer, length);

	return m_socket.write(buffer, length, m_addr, m_addrLen);
}

bool CP25Network::writeData(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	uint8_t buffer[50U];
	uint16_t len = 0U;

	if (data.hasData()) {
		switch (m_n) {
		case 0x62U:
			::memcpy(buffer, REC62, 22U);
			data.getData(buffer + 10U);
			len = 22U;
			m_n = 0x63U;
			break;

		case 0x63U:
			::memcpy(buffer, REC63, 14U);
			data.getData(buffer + 1U);
			len = 14U;
			m_n = 0x64U;
			break;

		case 0x64U:
			::memcpy(buffer, REC64, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x65U;
			break;

		case 0x65U: {
				NETWORK network = NETWORK::TO;
				uint32_t srcId = 0U, dstId = 0U;
				bool grp = true;
				data.getP25(network, srcId, dstId, grp);

				::memcpy(buffer, REC65, 17U);

				buffer[1U] = (dstId >> 16) & 0xFFU;
				buffer[2U] = (dstId >> 8)  & 0xFFU;
				buffer[3U] = (dstId >> 0)  & 0xFFU;

				data.getData(buffer + 5U);

				len = 17U;
				m_n = 0x66U;
			}
			break;

		case 0x66U: {
				NETWORK network = NETWORK::TO;
				uint32_t srcId = 0U, dstId = 0U;
				bool grp = true;
				data.getP25(network, srcId, dstId, grp);

				::memcpy(buffer, REC66, 17U);

				buffer[1U] = (srcId >> 16) & 0xFFU;
				buffer[2U] = (srcId >> 8)  & 0xFFU;
				buffer[3U] = (srcId >> 0)  & 0xFFU;

				data.getData(buffer + 5U);

				len = 17U;
				m_n = 0x67U;
			}
			break;

		case 0x67U:
			::memcpy(buffer, REC67, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x68U;
			break;

		case 0x68U:
			::memcpy(buffer, REC68, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x69U;
			break;

		case 0x69U:
			::memcpy(buffer, REC69, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x6AU;
			break;

		case 0x6AU:
			::memcpy(buffer, REC6A, 16U);
			data.getData(buffer + 4U);
			len = 16U;
			m_n = 0x6BU;
			break;

		case 0x6BU:
			::memcpy(buffer, REC6B, 22U);
			data.getData(buffer + 10U);
			len = 22U;
			m_n = 0x6CU;
			break;

		case 0x6CU:
			::memcpy(buffer, REC6C, 14U);
			data.getData(buffer + 1U);
			len = 14U;
			m_n = 0x6DU;
			break;

		case 0x6DU:
			::memcpy(buffer, REC6D, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x6EU;
			break;

		case 0x6EU:
			::memcpy(buffer, REC6E, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x6FU;
			break;

		case 0x6FU:
			::memcpy(buffer, REC6F, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x70U;
			break;

		case 0x70U:
			::memcpy(buffer, REC70, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x71U;
			break;

		case 0x71U:
			::memcpy(buffer, REC71, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x72U;
			break;

		case 0x72U:
			::memcpy(buffer, REC72, 17U);
			data.getData(buffer + 5U);
			len = 17U;
			m_n = 0x73U;
			break;

		case 0x73U:
			::memcpy(buffer, REC73, 16U);
			data.getData(buffer + 4U);
			len = 16U;
			m_n = 0x62U;
			break;

		default:
			break;
		}

		if (m_debug)
			CUtils::dump(1U, "P25 Network Data Sent", buffer, len);

		bool ret = m_socket.write(buffer, len, m_addr, m_addrLen);
		if (!ret)
			return false;
	}

	if (!data.hasData() && data.isEnd()) {
		::memcpy(buffer, REC80, 17U);

		if (m_debug)
			CUtils::dump(1U, "P25 Network Data Sent", buffer, 17U);

		bool ret = m_socket.write(buffer, 17U, m_addr, m_addrLen);
		if (!ret)
			return false;
	}

	return true;
}

void CP25Network::clock(unsigned int ms)
{
	uint8_t buffer[BUFFER_LENGTH];

	sockaddr_storage address;
	size_t addrLen;
	int length = m_socket.read(buffer, BUFFER_LENGTH, address, addrLen);
	if (length <= 0)
		return;

	if (!CUDPSocket::match(m_addr, address, IMT_ADDRESS_AND_PORT)) {
		LogMessage("P25, packet received from an invalid source");
		return;
	}

	if (m_debug)
		CUtils::dump(1U, "P25 Network Data Received", buffer, length);

	uint8_t c = length;
	m_buffer.add(&c, 1U);

	m_buffer.add(buffer, length);
}

bool CP25Network::read(CData& data)
{
	if (m_buffer.empty())
		return false;

	uint8_t length = 0U;
	m_buffer.get(&length, 1U);

	uint8_t buffer[BUFFER_LENGTH];
	m_buffer.get(buffer, length);

	data.setRaw(buffer, length);

	switch (buffer[0U]) {
	case 0x62U:
		data.setData(buffer + 10U);
		break;
	case 0x63U:
		data.setData(buffer + 1U);
		break;
	case 0x64U:
		data.setData(buffer + 5U);
		break;
	case 0x65U:
		if (m_dstId == 0U) {
			m_dstId |= buffer[1U] << 16;
			m_dstId |= buffer[2U] << 8;
			m_dstId |= buffer[3U] << 0;
			if (m_srcId > 0U)
				data.setP25(m_network, m_srcId, m_dstId, true);
		}
		data.setData(buffer + 5U);
		break;
	case 0x66U:
		if (m_srcId == 0U) {
			m_srcId |= buffer[1U] << 16;
			m_srcId |= buffer[2U] << 8;
			m_srcId |= buffer[3U] << 0;
			if (m_dstId > 0U)
				data.setP25(m_network, m_srcId, m_dstId, true);
		}
		data.setData(buffer + 5U);
		break;
	case 0x67U:
		data.setData(buffer + 5U);
		break;
	case 0x68U:
		data.setData(buffer + 5U);
		break;
	case 0x69U:
		data.setData(buffer + 5U);
		break;
	case 0x6AU:
		data.setData(buffer + 4U);
		break;
	case 0x6BU:
		data.setData(buffer + 10U);
		break;
	case 0x6CU:
		data.setData(buffer + 1U);
		break;
	case 0x6DU:
		data.setData(buffer + 5U);
		break;
	case 0x6EU:
		data.setData(buffer + 5U);
		break;
	case 0x6FU:
		data.setData(buffer + 5U);
		break;
	case 0x70U:
		data.setData(buffer + 5U);
		break;
	case 0x71U:
		data.setData(buffer + 5U);
		break;
	case 0x72U:
		data.setData(buffer + 5U);
		break;
	case 0x73U:
		data.setData(buffer + 4U);
		break;
	case 0x80U:
		data.setEnd();
		break;
	default:
		break;
	}

	return true;
}

bool CP25Network::read()
{
	if (m_buffer.empty())
		return false;

	uint8_t length = 0U;
	m_buffer.get(&length, 1U);

	uint8_t buffer[BUFFER_LENGTH];
	m_buffer.get(buffer, length);

	return true;
}

void CP25Network::reset()
{
	m_srcId = 0U;
	m_dstId = 0U;
	m_buffer.clear();
	m_n = 0x62U;
}

bool CP25Network::hasData()
{
	return m_buffer.hasData();
}

void CP25Network::close()
{
	m_socket.close();

	LogMessage("Closing P25 network connection");
}
