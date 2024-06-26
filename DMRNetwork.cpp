/*
 *   Copyright (C) 2015-2021,2024 by Jonathan Naylor G4KLX
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

#include "DMRNetwork.h"

#include "DMRDefines.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

const unsigned int BUFFER_LENGTH = 500U;

const unsigned int HOMEBREW_DATA_PACKET_LENGTH = 55U;

const uint8_t BIT_MASK_TABLE[] = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };

#define WRITE_BIT8(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])
#define READ_BIT8(p,i)    (p[(i)>>3] & BIT_MASK_TABLE[(i)&7])


CDMRNetwork::CDMRNetwork(NETWORK network, uint32_t id, const std::string& localAddress, uint16_t localPort, const std::string& remoteAddress, uint16_t remotePort, bool debug) :
m_network(network),
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_id(nullptr),
m_debug(debug),
m_buffer(nullptr),
m_streamId(0U),
m_rxData(1000U, "DMR Network"),
m_random(),
m_pingTimer(1000U, 10U),
m_audio(nullptr),
m_audioCount(0U),
m_lc(),
m_seqNo(0U),
m_N(0U)
{
	assert(remotePort > 0U);
	assert(id > 0U);

	if (CUDPSocket::lookup(remoteAddress, remotePort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;

	m_buffer   = new uint8_t[BUFFER_LENGTH];
	m_id       = new uint8_t[sizeof(uint32_t)];
	m_audio    = new uint8_t[DMR_NXDN_DATA_LENGTH * 3U];

	m_id[0U] = id >> 24;
	m_id[1U] = id >> 16;
	m_id[2U] = id >> 8;
	m_id[3U] = id >> 0;

	std::random_device rd;
	std::mt19937 mt(rd());
	m_random = mt;
}

CDMRNetwork::~CDMRNetwork()
{
	delete[] m_buffer;
	delete[] m_id;
	delete[] m_audio;
}

bool CDMRNetwork::open()
{
	if (m_addrLen == 0U) {
		LogError("DMR, Unable to resolve the address of the DMR Network");
		return false;
	}

	LogMessage("Opening DMR Network");

	bool ret = m_socket.open(m_addr);
	if (!ret)
		return false;

	m_pingTimer.start();

	return true;
}

bool CDMRNetwork::writeRaw(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	uint8_t buffer[100U];
	uint16_t length = data.getRaw(buffer);

	if (length == 0U)
		return true;

	if (m_debug)
		CUtils::dump(1U, "DMR Network Raw Sent", buffer, length);

	return m_socket.write(buffer, length, m_addr, m_addrLen);
}

bool CDMRNetwork::read(CData& data)
{
	switch (m_audioCount) {
	case 1U:
		data.setData(m_audio + DMR_NXDN_DATA_LENGTH);
		m_audioCount = 2U;
		return true;
	case 2U:
		data.setData(m_audio + DMR_NXDN_DATA_LENGTH + DMR_NXDN_DATA_LENGTH);
		m_audioCount = 0U;
		return true;
	default:
		break;
	}

	if (m_rxData.empty())
		return false;

	uint8_t length = 0U;
	m_rxData.get(&length, 1U);
	m_rxData.get(m_buffer, length);

	data.setRaw(m_buffer, length);

	// Is this a data packet?
	if (::memcmp(m_buffer, "DMRD", 4U) != 0)
		return true;

	// Remove headers and trailers, and other non-audio stuff
	bool dataSync = (m_buffer[15U] & 0x20U) == 0x20U;
	if (dataSync) {
		switch (m_buffer[15U] & DT_MASK) {
		case DT_VOICE_LC_HEADER: {
				uint32_t srcId = (m_buffer[5U] << 16) | (m_buffer[6U] << 8) | (m_buffer[7U] << 0);
				uint32_t dstId = (m_buffer[8U] << 16) | (m_buffer[9U] << 8) | (m_buffer[10U] << 0);
				uint8_t slot   = (m_buffer[15U] & 0x80U) == 0x80U ? 2U : 1U;
				bool grp       = (m_buffer[15U] & 0x40U) == 0x40U;
				data.setDMR(m_network, slot, srcId, dstId, grp);
			}
			break;

		case DT_TERMINATOR_WITH_LC:
			data.setEnd();
			break;

		default:
			break;
		}

		return true;
	}

	uint16_t inOffset = (20U * 8U) + 0U;
	uint16_t outOffset = 0U;
	for (unsigned int i = 0U; i < 108U; i++, inOffset++, outOffset++) {
		bool b = READ_BIT8(m_buffer, inOffset) != 0U;
		WRITE_BIT8(m_audio, outOffset, b);
	}

	inOffset += 48U;
	for (unsigned int i = 0U; i < 108U; i++, inOffset++, outOffset++) {
		bool b = READ_BIT8(m_buffer, inOffset) != 0U;
		WRITE_BIT8(m_audio, outOffset, b);
	}

	data.setData(m_audio + 0U);

	m_audioCount = 1U;

	return true;
}

bool CDMRNetwork::read()
{
	if (m_rxData.empty())
		return false;

	uint8_t length = 0U;
	m_rxData.get(&length, 1U);
	m_rxData.get(m_buffer, length);

	return true;
}

bool CDMRNetwork::writeData(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	switch (m_audioCount) {
	case 0U:
		for (uint16_t i = 0U; i < 3U; i++)
			::memcpy(m_audio + (i * DMR_NXDN_DATA_LENGTH), DMR_SILENCE, DMR_NXDN_DATA_LENGTH);

		if (data.hasData()) {
			data.getData(m_audio + 0U);
			m_audioCount = 1U;
		}

		if (!data.isEnd())
			return true;
		break;

	case 1U:
		if (data.hasData()) {
			data.getData(m_audio + DMR_NXDN_DATA_LENGTH);
			m_audioCount = 2U;
		}

		if (!data.isEnd())
			return true;
		break;

	case 2U:
		if (data.hasData()) {
			data.getData(m_audio + DMR_NXDN_DATA_LENGTH + DMR_NXDN_DATA_LENGTH);
			m_audioCount = 3U;
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
		return writeAudio(data);
	} else if (data.isEnd()) {
		if (m_audioCount > 0U) {
			bool ret = writeAudio(data);
			if (!ret)
				return false;
		}
		return writeTrailer(data);
	} else {
		return writeAudio(data);
	}
}

bool CDMRNetwork::writeHeader(CData& data)
{
	NETWORK network = NET_FROM;
	uint8_t slot = 0U;
	uint32_t srcId = 0U, dstId = 0U;
	bool grp = true;
	data.getDMR(network, slot, srcId, dstId, grp);

	FLCO flco = grp ? FLCO_GROUP : FLCO_USER_USER;

	m_lc.setParameters(flco, srcId, dstId);

	uint8_t buffer[HOMEBREW_DATA_PACKET_LENGTH];
	::memset(buffer, 0x00U, HOMEBREW_DATA_PACKET_LENGTH);

	buffer[0U] = 'D';
	buffer[1U] = 'M';
	buffer[2U] = 'R';
	buffer[3U] = 'D';

	buffer[5U] = srcId >> 16;
	buffer[6U] = srcId >> 8;
	buffer[7U] = srcId >> 0;

	buffer[8U]  = dstId >> 16;
	buffer[9U]  = dstId >> 8;
	buffer[10U] = dstId >> 0;

	::memcpy(buffer + 11U, m_id, 4U);

	buffer[15U] = slot == 1U ? 0x00U : 0x80U;

	buffer[15U] |= grp ? 0x00U : 0x40U;

	std::uniform_int_distribution<uint32_t> dist(0x00000001U, 0xFFFFFFFEU);
	m_streamId = dist(m_random);

	buffer[15U] |= (0x20U | DT_VOICE_LC_HEADER);

	buffer[4U] = uint8_t(m_seqNo & 0x00FFU);

	::memcpy(buffer + 16U, &m_streamId, 4U);

	::memcpy(buffer + 20U, HEADER_SYNC_SLOT_TYPE, DMR_FRAME_LENGTH_BYTES);

	m_lc.encode(buffer + 20U, DT_VOICE_LC_HEADER);

	buffer[53U] = 0U;
	buffer[54U] = 0U;

	m_seqNo++;
	m_N = 0U;

	if (m_debug)
		CUtils::dump(1U, "DMR Network Header Sent", buffer, HOMEBREW_DATA_PACKET_LENGTH);

	return m_socket.write(buffer, HOMEBREW_DATA_PACKET_LENGTH, m_addr, m_addrLen);
}

bool CDMRNetwork::writeAudio(CData& data)
{
	NETWORK network = NET_FROM;
	uint8_t slot = 0U;
	uint32_t srcId = 0U, dstId = 0U;
	bool grp = true;
	data.getDMR(network, slot, srcId, dstId, grp);

	FLCO flco = grp ? FLCO_GROUP : FLCO_USER_USER;

	m_lc.setParameters(flco, srcId, dstId);

	uint8_t buffer[HOMEBREW_DATA_PACKET_LENGTH];
	::memset(buffer, 0x00U, HOMEBREW_DATA_PACKET_LENGTH);

	buffer[0U]  = 'D';
	buffer[1U]  = 'M';
	buffer[2U]  = 'R';
	buffer[3U]  = 'D';

	buffer[5U]  = srcId >> 16;
	buffer[6U]  = srcId >> 8;
	buffer[7U]  = srcId >> 0;

	buffer[8U]  = dstId >> 16;
	buffer[9U]  = dstId >> 8;
	buffer[10U] = dstId >> 0;

	::memcpy(buffer + 11U, m_id, 4U);

	buffer[15U] = slot == 1U ? 0x00U : 0x80U;

	buffer[15U] |= grp ? 0x00U : 0x40U;

	buffer[4U] = uint8_t(m_seqNo & 0x00FFU);

	::memcpy(buffer + 16U, &m_streamId, 4U);

	switch (m_N) {
	case 0U:
		::memcpy(buffer + 20U, VOICE_SYNC, DMR_FRAME_LENGTH_BYTES);
		buffer[15U] |= 0x10U;
		m_N++;
		break;
	case 1U:
		::memcpy(buffer + 20U, FIRST_LC_EMB, DMR_FRAME_LENGTH_BYTES);
		m_lc.getData(buffer + 20U, 0U);
		buffer[15U] |= 1U;
		m_N++;
		break;
	case 2U:
		::memcpy(buffer + 20U, CONT_LC_EMB, DMR_FRAME_LENGTH_BYTES);
		m_lc.getData(buffer + 20U, 1U);
		buffer[15U] |= 2U;
		m_N++;
		break;
	case 3U:
		::memcpy(buffer + 20U, CONT_LC_EMB, DMR_FRAME_LENGTH_BYTES);
		m_lc.getData(buffer + 20U, 2U);
		buffer[15U] |= 3U;
		m_N++;
		break;
	case 4U:
		::memcpy(buffer + 20U, LAST_LC_EMB, DMR_FRAME_LENGTH_BYTES);
		m_lc.getData(buffer + 20U, 3U);
		buffer[15U] |= 4U;
		m_N++;
		break;
	case 5U:
		::memcpy(buffer + 20U, NULL_LC_EMB, DMR_FRAME_LENGTH_BYTES);
		buffer[15U] |= 5U;
		m_N = 0U;
		break;
	default:
		assert(false);
	}

	// Add the audio
	uint16_t inOffset = 0U;
	uint16_t outOffset = (20U * 8U) + 0U;
	for (unsigned int i = 0U; i < 108U; i++, inOffset++, outOffset++) {
		bool b = READ_BIT8(m_audio, inOffset) != 0U;
		WRITE_BIT8(buffer, outOffset, b);
	}

	outOffset += 48U;
	for (unsigned int i = 0U; i < 108U; i++, inOffset++, outOffset++) {
		bool b = READ_BIT8(m_audio, inOffset) != 0U;
		WRITE_BIT8(buffer, outOffset, b);
	}

	buffer[53U] = 0U;
	buffer[54U] = 0U;

	m_seqNo++;

	if (m_debug)
		CUtils::dump(1U, "DMR Network Audio Sent", buffer, HOMEBREW_DATA_PACKET_LENGTH);

	return m_socket.write(buffer, HOMEBREW_DATA_PACKET_LENGTH, m_addr, m_addrLen);
}

bool CDMRNetwork::writeTrailer(CData& data)
{
	NETWORK network = NET_FROM;
	uint8_t slot = 0U;
	uint32_t srcId = 0U, dstId = 0U;
	bool grp = true;
	data.getDMR(network, slot, srcId, dstId, grp);

	FLCO flco = grp ? FLCO_GROUP : FLCO_USER_USER;

	m_lc.setParameters(flco, srcId, dstId);

	uint8_t buffer[HOMEBREW_DATA_PACKET_LENGTH];
	::memset(buffer, 0x00U, HOMEBREW_DATA_PACKET_LENGTH);

	buffer[0U] = 'D';
	buffer[1U] = 'M';
	buffer[2U] = 'R';
	buffer[3U] = 'D';

	buffer[5U] = srcId >> 16;
	buffer[6U] = srcId >> 8;
	buffer[7U] = srcId >> 0;

	buffer[8U]  = dstId >> 16;
	buffer[9U]  = dstId >> 8;
	buffer[10U] = dstId >> 0;

	::memcpy(buffer + 11U, m_id, 4U);

	buffer[15U] = slot == 1U ? 0x00U : 0x80U;

	buffer[15U] |= grp ? 0x00U : 0x40U;

	buffer[15U] |= (0x20U | DT_TERMINATOR_WITH_LC);

	buffer[4U] = uint8_t(m_seqNo & 0x00FFU);

	::memcpy(buffer + 16U, &m_streamId, 4U);

	::memcpy(buffer + 20U, TRAILER_SYNC_SLOT_TYPE, DMR_FRAME_LENGTH_BYTES);

	m_lc.encode(buffer + 20U, DT_TERMINATOR_WITH_LC);

	buffer[53U] = 0U;
	buffer[54U] = 0U;

	if (m_debug)
		CUtils::dump(1U, "DMR Network Trailer Sent", buffer, HOMEBREW_DATA_PACKET_LENGTH);

	return m_socket.write(buffer, HOMEBREW_DATA_PACKET_LENGTH, m_addr, m_addrLen);
}

void CDMRNetwork::reset()
{
	m_rxData.clear();
	m_audioCount = 0U;
	m_seqNo      = 0U;
	m_N          = 0U;
}

bool CDMRNetwork::hasData()
{
	return m_rxData.hasData() || (m_audioCount > 0U);
}

void CDMRNetwork::close()
{
	LogMessage("Closing DMR Network");

	m_socket.close();
}

void CDMRNetwork::clock(unsigned int ms)
{
	m_pingTimer.clock(ms);
	if (m_pingTimer.isRunning() && m_pingTimer.hasExpired()) {
		writePing();
		m_pingTimer.start();
	}

	sockaddr_storage address;
	size_t addrLen;
	int length = m_socket.read(m_buffer, BUFFER_LENGTH, address, addrLen);
	if (length <= 0)
		return;

	if (!CUDPSocket::match(m_addr, address)) {
		LogMessage("DMR, packet received from an invalid source");
		return;
	}

	if (m_debug)
		CUtils::dump(1U, "DMR Network Received", m_buffer, length);

	uint8_t len = length;
	m_rxData.add(&len, 1U);
	m_rxData.add(m_buffer, len);
}

bool CDMRNetwork::writePing()
{
	uint8_t buffer[10U];

	buffer[0U] = 'D';
	buffer[1U] = 'M';
	buffer[2U] = 'R';
	buffer[3U] = 'P';

	// if (m_debug)
	//	CUtils::dump(1U, "DMR Network Ping Sent", buffer, 4U);

	return m_socket.write(buffer, 4U, m_addr, m_addrLen);
}
