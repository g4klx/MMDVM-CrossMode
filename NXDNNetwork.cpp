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
#include "NXDNDefines.h"
#include "NXDNLayer3.h"
#include "NXDNFACCH1.h"
#include "NXDNSACCH.h"
#include "NXDNLICH.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

const unsigned int BUFFER_LENGTH = 200U;

CNXDNNetwork::CNXDNNetwork(NETWORK network, const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug) :
m_network(network),
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_debug(debug),
m_buffer(1000U, "NXDN Network"),
m_seqNo(0U),
m_audio(nullptr),
m_audioCount(0U),
m_maxAudio(0U),
m_layer3()
{
	assert(localPort > 0U);
	assert(!remoteAddress.empty());
	assert(remotePort > 0U);

	if (CUDPSocket::lookup(remoteAddress, remotePort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;

	m_audio = new uint8_t[DMR_NXDN_DATA_LENGTH * 2U];
}

CNXDNNetwork::~CNXDNNetwork()
{
	delete[] m_audio;
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

	uint8_t buffer[500U];
	uint16_t length = data.getRaw(buffer);

	if (length == 0U)
		return true;

	if (m_debug)
		CUtils::dump(1U, "NXDN Network Raw Sent", buffer, length);

	return m_socket.write(buffer, length, m_addr, m_addrLen);
}

bool CNXDNNetwork::writeData(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	switch (m_audioCount) {
	case 0U:
		::memcpy(m_audio + 0U, NXDN_SILENCE, DMR_NXDN_DATA_LENGTH);
		::memcpy(m_audio + DMR_NXDN_DATA_LENGTH, NXDN_SILENCE, DMR_NXDN_DATA_LENGTH);

		if (data.hasData()) {
			data.getData(m_audio + 0U);
			m_audioCount = 1U;
		}

		if (!data.isEnd())
			return true;
		break;

	case 1U:
		if (data.hasData()) {
			data.getData(m_audio + DMR_NXDN_DATA_LENGTH + DMR_NXDN_DATA_LENGTH + DMR_NXDN_DATA_LENGTH);
			m_audioCount = 2U;
		}
		break;

	default:
		// This shouldn't happen
		assert(false);
	}

	if (m_seqNo == 0U) {
		bool ret = writeHeader(data);
		if (!ret)
			return false;
		return writeBody(data);
	} else if (data.isEnd()) {
		if (m_audioCount > 0U) {
			bool ret = writeBody(data);
			if (!ret)
				return false;
		}
		return writeTrailer(data);
	} else {
		return writeBody(data);
	}
}

bool CNXDNNetwork::writeHeader(CData& data)
{
	uint8_t buffer[110U];
	::memset(buffer, 0x00U, 110U);

	buffer[0U] = 'I';
	buffer[1U] = 'C';
	buffer[2U] = 'O';
	buffer[3U] = 'M';
	buffer[4U] = 0x01U;
	buffer[5U] = 0x01U;
	buffer[6U] = 0x08U;
	buffer[7U] = 0xE0U;

	buffer[37U] = 0x23U;
	buffer[38U] = 0x1CU;
	buffer[39U] = 0x21U;

	CNXDNLICH lich;
	lich.setRFCT(NXDN_LICH_RFCT_RDCH);
	lich.setFCT(NXDN_LICH_USC_SACCH_NS);
	lich.setOption(NXDN_LICH_STEAL_FACCH);
	lich.setDirection(NXDN_LICH_DIRECTION_OUTBOUND);
	buffer[40U] = lich.getRaw();

	CNXDNSACCH sacch;
	sacch.setRAN(0U);
	sacch.setStructure(NXDN_SR_SINGLE);
	sacch.setData(SACCH_IDLE);
	sacch.getRaw(buffer + 41U);

	NETWORK network = NET_TO;
	uint16_t srcId = 0U, dstId = 0U;
	bool grp = true;
	data.getNXDN(network, srcId, dstId, grp);

	uint8_t message[22U];
	m_layer3.setMessageType(NXDN_MESSAGE_TYPE_VCALL);
	m_layer3.setSourceUnitId(srcId);
	m_layer3.setDestinationGroupId(srcId);
	m_layer3.setIsGroup(grp);
	m_layer3.getData(message);

	CNXDNFACCH1 facch;
	facch.setData(message);
	facch.getRaw(buffer + 45U + 0U);
	facch.getRaw(buffer + 45U + 14U);

	m_seqNo++;

	if (m_debug)
		CUtils::dump(1U, "NXDN Network Header Sent", buffer, 102U);

	return m_socket.write(buffer, 102U, m_addr, m_addrLen);
}

bool CNXDNNetwork::writeBody(CData& data)
{
	uint8_t buffer[110U];
	::memset(buffer, 0x00U, 110U);

	buffer[0U] = 'I';
	buffer[1U] = 'C';
	buffer[2U] = 'O';
	buffer[3U] = 'M';
	buffer[4U] = 0x01U;
	buffer[5U] = 0x01U;
	buffer[6U] = 0x08U;
	buffer[7U] = 0xE0U;

	buffer[37U] = 0x23U;
	buffer[38U] = 0x10U;
	buffer[39U] = 0x21U;

	CNXDNLICH lich;
	lich.setRFCT(NXDN_LICH_RFCT_RDCH);
	lich.setFCT(NXDN_LICH_USC_SACCH_SS);
	lich.setOption(NXDN_LICH_STEAL_FACCH1_1);
	lich.setDirection(NXDN_LICH_DIRECTION_OUTBOUND);
	buffer[40U] = lich.getRaw();

	CNXDNSACCH sacch;
	sacch.setRAN(0U);

	uint8_t message[3U];

	switch ((m_seqNo - 1U) % 4U) {
	case 0U:
		sacch.setStructure(NXDN_SR_1_4);
		m_layer3.encode(message, 18U, 0U);
		break;
	case 1U:
		sacch.setStructure(NXDN_SR_2_4);
		m_layer3.encode(message, 18U, 18U);
		break;
	case 2U:
		sacch.setStructure(NXDN_SR_3_4);
		m_layer3.encode(message, 18U, 36U);
		break;
	default:
		sacch.setStructure(NXDN_SR_4_4);
		m_layer3.encode(message, 18U, 54U);
		break;
	}

	sacch.setData(SACCH_IDLE);
	sacch.getRaw(buffer + 41U);

	CNXDNFACCH1 facch;
	facch.setData(message);
	facch.getRaw(buffer + 45U + 0U);

	::memcpy(buffer + 45U + 14U, m_audio + 0U, 2U * DMR_NXDN_DATA_LENGTH);

	m_audioCount = 0U;
	m_seqNo++;

	if (m_debug)
		CUtils::dump(1U, "NXDN Network Audio Sent", buffer, 102U);

	return m_socket.write(buffer, 102U, m_addr, m_addrLen);
}

bool CNXDNNetwork::writeTrailer(CData& data)
{
	uint8_t buffer[110U];
	::memset(buffer, 0x00U, 110U);

	buffer[0U] = 'I';
	buffer[1U] = 'C';
	buffer[2U] = 'O';
	buffer[3U] = 'M';
	buffer[4U] = 0x01U;
	buffer[5U] = 0x01U;
	buffer[6U] = 0x08U;
	buffer[7U] = 0xE0U;

	buffer[37U] = 0x23U;
	buffer[38U] = 0x1CU;
	buffer[39U] = 0x21U;

	CNXDNLICH lich;
	lich.setRFCT(NXDN_LICH_RFCT_RDCH);
	lich.setFCT(NXDN_LICH_USC_SACCH_NS);
	lich.setOption(NXDN_LICH_STEAL_FACCH);
	lich.setDirection(NXDN_LICH_DIRECTION_OUTBOUND);
	buffer[40U] = lich.getRaw();

	CNXDNSACCH sacch;
	sacch.setRAN(0U);
	sacch.setStructure(NXDN_SR_SINGLE);
	sacch.setData(SACCH_IDLE);
	sacch.getRaw(buffer + 41U);

	uint8_t message[22U];
	m_layer3.setMessageType(NXDN_MESSAGE_TYPE_TX_REL);
	m_layer3.getData(message);

	CNXDNFACCH1 facch;
	facch.setData(message);
	facch.getRaw(buffer + 45U + 0U);
	facch.getRaw(buffer + 45U + 14U);

	m_seqNo = 0U;

	if (m_debug)
		CUtils::dump(1U, "NXDN Network Trailer Sent", buffer, 102U);

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
	switch (m_audioCount) {
	case 1U:
		data.setData(m_audio + DMR_NXDN_DATA_LENGTH);
		m_audioCount = 2U;
		if (m_maxAudio == 2U)
			m_audioCount = 0U;
		return true;
	case 2U:
		data.setData(m_audio + DMR_NXDN_DATA_LENGTH + DMR_NXDN_DATA_LENGTH);
		m_audioCount = 3U;
		return true;
	case 3U:
		data.setData(m_audio + DMR_NXDN_DATA_LENGTH + DMR_NXDN_DATA_LENGTH + DMR_NXDN_DATA_LENGTH);
		m_audioCount = 0U;
		return true;
	default:
		break;
	}

	if (m_buffer.empty())
		return false;

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

	CNXDNLICH lich;
	lich.decode(buffer + 42U);

	uint8_t usc = lich.getFCT();
	if (usc == NXDN_LICH_USC_UDCH)
		return false;

	if (usc == NXDN_LICH_USC_SACCH_NS) {
		CNXDNFACCH1 facch;
		bool valid = facch.decode(buffer + 42U, NXDN_FSW_LENGTH_BITS + NXDN_LICH_LENGTH_BITS + NXDN_SACCH_LENGTH_BITS);
		if (!valid)
			valid = facch.decode(buffer + 42U, NXDN_FSW_LENGTH_BITS + NXDN_LICH_LENGTH_BITS + NXDN_SACCH_LENGTH_BITS + NXDN_FACCH1_LENGTH_BITS);
		if (!valid)
			return false;

		uint8_t buffer[10U];
		facch.getData(buffer);

		CNXDNLayer3 layer3;
		layer3.decode(buffer, NXDN_FACCH1_LENGTH_BITS);

		uint8_t type = layer3.getMessageType();
		switch (type) {
		case NXDN_MESSAGE_TYPE_TX_REL:
			data.setEnd();
			return true;

		case NXDN_MESSAGE_TYPE_VCALL: {
				uint16_t srcId = layer3.getSourceUnitId();
				uint16_t dstId = layer3.getDestinationGroupId();
				bool       grp = layer3.getIsGroup();
				data.setNXDN(m_network, srcId, dstId, grp);
			}
			return true;

		default:
			return false;
		}
	} else {
		uint8_t option = lich.getOption();

		switch (option) {
		case NXDN_LICH_STEAL_NONE:
			::memcpy(m_audio, buffer + 42U + NXDN_FSW_LICH_SACCH_LENGTH_BYTES + 0U, 4U * DMR_NXDN_DATA_LENGTH);
			data.setData(m_audio + 0U);
			m_audioCount = 1U;
			m_maxAudio   = 4U;
			break;
		case NXDN_LICH_STEAL_FACCH1_1:
			::memcpy(m_audio, buffer + 42U + NXDN_FSW_LICH_SACCH_LENGTH_BYTES + 18U, 2U * DMR_NXDN_DATA_LENGTH);
			data.setData(m_audio + 0U);
			m_audioCount = 1U;
			m_maxAudio   = 2U;
			break;
		case NXDN_LICH_STEAL_FACCH1_2:
			::memcpy(m_audio, buffer + 42U + NXDN_FSW_LICH_SACCH_LENGTH_BYTES + 0U, 2U * DMR_NXDN_DATA_LENGTH);
			data.setData(m_audio + 0U);
			m_audioCount = 1U;
			m_maxAudio   = 2U;
			break;
		default:
			return false;
		}
	}

	return true;
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
	m_maxAudio   = 0U;
	m_seqNo      = 0U;
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
