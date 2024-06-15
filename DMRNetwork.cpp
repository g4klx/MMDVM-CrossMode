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

const uint16_t ENCODING_TABLE_1676[] = {
	0x0000U, 0x0273U, 0x04E5U, 0x0696U, 0x09C9U, 0x0BBAU, 0x0D2CU, 0x0F5FU, 0x11E2U, 0x1391U, 0x1507U, 0x1774U,
	0x182BU, 0x1A58U, 0x1CCEU, 0x1EBDU, 0x21B7U, 0x23C4U, 0x2552U, 0x2721U, 0x287EU, 0x2A0DU, 0x2C9BU, 0x2EE8U,
	0x3055U, 0x3226U, 0x34B0U, 0x36C3U, 0x399CU, 0x3BEFU, 0x3D79U, 0x3F0AU, 0x411EU, 0x436DU, 0x45FBU, 0x4788U,
	0x48D7U, 0x4AA4U, 0x4C32U, 0x4E41U, 0x50FCU, 0x528FU, 0x5419U, 0x566AU, 0x5935U, 0x5B46U, 0x5DD0U, 0x5FA3U,
	0x60A9U, 0x62DAU, 0x644CU, 0x663FU, 0x6960U, 0x6B13U, 0x6D85U, 0x6FF6U, 0x714BU, 0x7338U, 0x75AEU, 0x77DDU,
	0x7882U, 0x7AF1U, 0x7C67U, 0x7E14U, 0x804FU, 0x823CU, 0x84AAU, 0x86D9U, 0x8986U, 0x8BF5U, 0x8D63U, 0x8F10U,
	0x91ADU, 0x93DEU, 0x9548U, 0x973BU, 0x9864U, 0x9A17U, 0x9C81U, 0x9EF2U, 0xA1F8U, 0xA38BU, 0xA51DU, 0xA76EU,
	0xA831U, 0xAA42U, 0xACD4U, 0xAEA7U, 0xB01AU, 0xB269U, 0xB4FFU, 0xB68CU, 0xB9D3U, 0xBBA0U, 0xBD36U, 0xBF45U,
	0xC151U, 0xC322U, 0xC5B4U, 0xC7C7U, 0xC898U, 0xCAEBU, 0xCC7DU, 0xCE0EU, 0xD0B3U, 0xD2C0U, 0xD456U, 0xD625U,
	0xD97AU, 0xDB09U, 0xDD9FU, 0xDFECU, 0xE0E6U, 0xE295U, 0xE403U, 0xE670U, 0xE92FU, 0xEB5CU, 0xEDCAU, 0xEFB9U,
	0xF104U, 0xF377U, 0xF5E1U, 0xF792U, 0xF8CDU, 0xFABEU, 0xFC28U, 0xFE5BU};

const uint16_t ENCODING_TABLE_2087[] = {
	0x0000U, 0xB08EU, 0xE093U, 0x501DU, 0x70A9U, 0xC027U, 0x903AU, 0x20B4U, 0x60DCU, 0xD052U, 0x804FU, 0x30C1U,
	0x1075U, 0xA0FBU, 0xF0E6U, 0x4068U, 0x7036U, 0xC0B8U, 0x90A5U, 0x202BU, 0x009FU, 0xB011U, 0xE00CU, 0x5082U,
	0x10EAU, 0xA064U, 0xF079U, 0x40F7U, 0x6043U, 0xD0CDU, 0x80D0U, 0x305EU, 0xD06CU, 0x60E2U, 0x30FFU, 0x8071U,
	0xA0C5U, 0x104BU, 0x4056U, 0xF0D8U, 0xB0B0U, 0x003EU, 0x5023U, 0xE0ADU, 0xC019U, 0x7097U, 0x208AU, 0x9004U,
	0xA05AU, 0x10D4U, 0x40C9U, 0xF047U, 0xD0F3U, 0x607DU, 0x3060U, 0x80EEU, 0xC086U, 0x7008U, 0x2015U, 0x909BU,
	0xB02FU, 0x00A1U, 0x50BCU, 0xE032U, 0x90D9U, 0x2057U, 0x704AU, 0xC0C4U, 0xE070U, 0x50FEU, 0x00E3U, 0xB06DU,
	0xF005U, 0x408BU, 0x1096U, 0xA018U, 0x80ACU, 0x3022U, 0x603FU, 0xD0B1U, 0xE0EFU, 0x5061U, 0x007CU, 0xB0F2U,
	0x9046U, 0x20C8U, 0x70D5U, 0xC05BU, 0x8033U, 0x30BDU, 0x60A0U, 0xD02EU, 0xF09AU, 0x4014U, 0x1009U, 0xA087U,
	0x40B5U, 0xF03BU, 0xA026U, 0x10A8U, 0x301CU, 0x8092U, 0xD08FU, 0x6001U, 0x2069U, 0x90E7U, 0xC0FAU, 0x7074U,
	0x50C0U, 0xE04EU, 0xB053U, 0x00DDU, 0x3083U, 0x800DU, 0xD010U, 0x609EU, 0x402AU, 0xF0A4U, 0xA0B9U, 0x1037U,
	0x505FU, 0xE0D1U, 0xB0CCU, 0x0042U, 0x20F6U, 0x9078U, 0xC065U, 0x70EBU, 0xA03DU, 0x10B3U, 0x40AEU, 0xF020U,
	0xD094U, 0x601AU, 0x3007U, 0x8089U, 0xC0E1U, 0x706FU, 0x2072U, 0x90FCU, 0xB048U, 0x00C6U, 0x50DBU, 0xE055U,
	0xD00BU, 0x6085U, 0x3098U, 0x8016U, 0xA0A2U, 0x102CU, 0x4031U, 0xF0BFU, 0xB0D7U, 0x0059U, 0x5044U, 0xE0CAU,
	0xC07EU, 0x70F0U, 0x20EDU, 0x9063U, 0x7051U, 0xC0DFU, 0x90C2U, 0x204CU, 0x00F8U, 0xB076U, 0xE06BU, 0x50E5U,
	0x108DU, 0xA003U, 0xF01EU, 0x4090U, 0x6024U, 0xD0AAU, 0x80B7U, 0x3039U, 0x0067U, 0xB0E9U, 0xE0F4U, 0x507AU,
	0x70CEU, 0xC040U, 0x905DU, 0x20D3U, 0x60BBU, 0xD035U, 0x8028U, 0x30A6U, 0x1012U, 0xA09CU, 0xF081U, 0x400FU,
	0x30E4U, 0x806AU, 0xD077U, 0x60F9U, 0x404DU, 0xF0C3U, 0xA0DEU, 0x1050U, 0x5038U, 0xE0B6U, 0xB0ABU, 0x0025U,
	0x2091U, 0x901FU, 0xC002U, 0x708CU, 0x40D2U, 0xF05CU, 0xA041U, 0x10CFU, 0x307BU, 0x80F5U, 0xD0E8U, 0x6066U,
	0x200EU, 0x9080U, 0xC09DU, 0x7013U, 0x50A7U, 0xE029U, 0xB034U, 0x00BAU, 0xE088U, 0x5006U, 0x001BU, 0xB095U,
	0x9021U, 0x20AFU, 0x70B2U, 0xC03CU, 0x8054U, 0x30DAU, 0x60C7U, 0xD049U, 0xF0FDU, 0x4073U, 0x106EU, 0xA0E0U,
	0x90BEU, 0x2030U, 0x702DU, 0xC0A3U, 0xE017U, 0x5099U, 0x0084U, 0xB00AU, 0xF062U, 0x40ECU, 0x10F1U, 0xA07FU,
	0x80CBU, 0x3045U, 0x6058U, 0xD0D6U};

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
m_seqNo(0U),
m_N(0U)
#if defined(DUMP_DMR)
, m_fpIn(nullptr),
m_fpOut(nullptr)
#endif
{
	assert(remotePort > 0U);

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

#if defined(DUMP_DMR)
	m_fpIn = ::fopen("dump_in.dmr", "wb");
	m_fpOut = ::fopen("dump_out.dmr", "wb");
#endif

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

#if defined(DUMP_DMR)
	if (m_fpIn != nullptr) {
		::fwrite(m_audio, 1U, DMR_NXDN_DATA_LENGTH * 3U, m_fpIn);
		::fflush(m_fpIn);
	}
#endif

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
			::memcpy(m_audio + (i * DMR_NXDN_DATA_LENGTH), DMR_NXDN_SILENCE, DMR_NXDN_DATA_LENGTH);

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

	std::uniform_int_distribution<uint32_t> dist(0x00000001, 0xFFFFFFFE);
	m_streamId = dist(m_random);

	buffer[15U] |= (0x20U | DT_VOICE_LC_HEADER);

	buffer[4U] = uint8_t(m_seqNo & 0xFFU);

	::memcpy(buffer + 16U, &m_streamId, 4U);

	uint16_t offset = (20U * 8U) + 108U;
	for (unsigned int i = 0U; i < 48U; i++, offset++) {
		bool b = READ_BIT8(MS_SOURCED_DATA_SYNC, i) != 0U;
		WRITE_BIT8(buffer, offset, b);
	}

	encodeSlotType(buffer + 20U, 0U, DT_VOICE_LC_HEADER);

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

	buffer[4U] = uint8_t(m_seqNo & 0xFFU);

	::memcpy(buffer + 16U, &m_streamId, 4U);

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

	switch (m_N) {
	case 0U: {
			uint16_t offset = (20U * 8U) + 108U;
			for (unsigned int i = 0U; i < 48U; i++, offset++) {
				bool b = READ_BIT8(MS_SOURCED_AUDIO_SYNC, i) != 0U;
				WRITE_BIT8(buffer, offset, b);
			}
			buffer[15U] |= 0x10U;
			m_N++;
		}
		break;
	case 1U:
		encodeEMB(buffer + 20U, 0U, false, 0x01U);
		// Add LC
		buffer[15U] |= 1U;
		m_N++;
		break;
	case 2U:
		encodeEMB(buffer + 20U, 0U, false, 0x03U);
		// Add LC
		buffer[15U] |= 2U;
		m_N++;
		break;
	case 3U:
		encodeEMB(buffer + 20U, 0U, false, 0x03U);
		// Add LC
		buffer[15U] |= 3U;
		m_N++;
		break;
	case 4U:
		encodeEMB(buffer + 20U, 0U, false, 0x02U);
		// Add LC
		buffer[15U] |= 4U;
		m_N++;
		break;
	case 5U:
		// NULL LC
		encodeEMB(buffer + 20U, 0U, false, 0x00U);
		buffer[15U] |= 5U;
		m_N = 0U;
		break;
	default:
		assert(false);
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

	buffer[4U] = uint8_t(m_seqNo & 0xFFU);

	::memcpy(buffer + 16U, &m_streamId, 4U);

	uint16_t offset = (20U * 8U) + 108U;
	for (unsigned int i = 0U; i < 48U; i++, offset++) {
		bool b = READ_BIT8(MS_SOURCED_DATA_SYNC, i) != 0U;
		WRITE_BIT8(buffer, offset, b);
	}

	encodeSlotType(buffer + 20U, 0U, DT_TERMINATOR_WITH_LC);

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

#if defined(DUMP_DMR)
	if (m_fpIn != nullptr) {
		::fclose(m_fpIn);
		m_fpIn = nullptr;
	}

	if (m_fpOut != nullptr) {
		::fclose(m_fpOut);
		m_fpOut = nullptr;
	}
#endif

	m_socket.close();
}

void CDMRNetwork::clock(unsigned int ms)
{
	m_pingTimer.clock(ms);
	if (m_pingTimer.isRunning() && m_pingTimer.hasExpired()) {
		// writeConfig();
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

void CDMRNetwork::encodeEMB(uint8_t* data, uint8_t cc, bool pi, uint8_t lcss) const
{
	assert(data != nullptr);

	uint8_t DMREMB[2U];
	DMREMB[0U] = (cc << 4) & 0xF0U;
	DMREMB[0U] |= pi ? 0x08U : 0x00U;
	DMREMB[0U] |= (lcss << 1) & 0x06U;
	DMREMB[1U] = 0x00U;

	encodeQR1676(DMREMB);

	data[13U] = (data[13U] & 0xF0U) | ((DMREMB[0U] >> 4U) & 0x0FU);
	data[14U] = (data[14U] & 0x0FU) | ((DMREMB[0U] << 4U) & 0xF0U);
	data[18U] = (data[18U] & 0xF0U) | ((DMREMB[1U] >> 4U) & 0x0FU);
	data[19U] = (data[19U] & 0x0FU) | ((DMREMB[1U] << 4U) & 0xF0U);
}

void CDMRNetwork::encodeSlotType(uint8_t* data, uint8_t cc, uint8_t dataType) const
{
	assert(data != nullptr);

	uint8_t DMRSlotType[3U];
	DMRSlotType[0U] = (cc << 4) & 0xF0U;
	DMRSlotType[0U] |= (dataType << 0) & 0x0FU;
	DMRSlotType[1U] = 0x00U;
	DMRSlotType[2U] = 0x00U;

	encode2087(DMRSlotType);

	data[12U] = (data[12U] & 0xC0U) | ((DMRSlotType[0U] >> 2) & 0x3FU);
	data[13U] = (data[13U] & 0x0FU) | ((DMRSlotType[0U] << 6) & 0xC0U) | ((DMRSlotType[1U] >> 2) & 0x30U);
	data[19U] = (data[19U] & 0xF0U) | ((DMRSlotType[1U] >> 2) & 0x0FU);
	data[20U] = (data[20U] & 0x03U) | ((DMRSlotType[1U] << 6) & 0xC0U) | ((DMRSlotType[2U] >> 2) & 0x3CU);
}

void CDMRNetwork::encodeQR1676(uint8_t* data) const
{
	assert(data != nullptr);

	uint8_t  value = (data[0U] >> 1) & 0x7FU;
	uint16_t cksum = ENCODING_TABLE_1676[value];

	data[0U] = cksum >> 8;
	data[1U] = cksum & 0xFFU;
}

void CDMRNetwork::encode2087(uint8_t* data) const
{
	assert(data != nullptr);

	uint8_t value = data[0U];

	uint16_t cksum = ENCODING_TABLE_2087[value];

	data[1U] = cksum & 0xFFU;
	data[2U] = cksum >> 8;
}
