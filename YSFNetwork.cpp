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

#include "TranscoderDefines.h"
#include "YSFPayload.h"
#include "YSFDefines.h"
#include "YSFNetwork.h"
#include "YSFFICH.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

const unsigned int BUFFER_LENGTH = 200U;

CYSFNetwork::CYSFNetwork(const std::string& callsign, const std::string& localAddress, unsigned short localPort, const std::string& gatewayAddress, unsigned short gatewayPort, bool debug) :
m_socket(localAddress, localPort),
m_addr(),
m_addrLen(0U),
m_callsign(),
m_debug(debug),
m_buffer(1000U, "YSF Network"),
m_pollTimer(1000U, 5U),
m_tag(nullptr),
m_seqNo(0U),
m_audio(nullptr),
m_audioCount(0U),
m_fn(0U)
#if defined(DUMP_M17)
, m_fpIn(nullptr),
m_fpOut(nullptr)
#endif
{
	m_callsign = callsign;
	m_callsign.resize(YSF_CALLSIGN_LENGTH, ' ');

	if (CUDPSocket::lookup(gatewayAddress, gatewayPort, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;

	m_audio = new uint8_t[YSFDN_DATA_LENGTH * 5U];

	m_tag = new uint8_t[YSF_CALLSIGN_LENGTH];
	::memset(m_tag, ' ', YSF_CALLSIGN_LENGTH);
}

CYSFNetwork::~CYSFNetwork()
{
	delete[] m_audio;
	delete[] m_tag;
}

bool CYSFNetwork::open()
{
	if (m_addrLen == 0U) {
		LogError("Unable to resolve the address of the YSF Gateway");
		return false;
	}

	LogMessage("Opening YSF network connection");

	m_pollTimer.start();

#if defined(DUMP_YSF)
	m_fpIn  = ::fopen("dump_in.ysf", "wb");
	m_fpOut = ::fopen("dump_out.ysf", "wb");
#endif

	return m_socket.open(m_addr);
}

bool CYSFNetwork::write(CData& data)
{
	if (m_addrLen == 0U)
		return false;

	switch (m_audioCount) {
	case 0U:
		data.getData(m_audio + 0U);
		m_audioCount = 1U;
		if (!data.isEnd())
			return true;
		break;
	case 1U:
		data.getData(m_audio + YSFDN_DATA_LENGTH);
		m_audioCount = 2U;
		if (!data.isEnd())
			return true;
		break;
	case 2U:
		data.getData(m_audio + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH);
		m_audioCount = 3U;
		if (!data.isEnd())
			return true;
		break;
	case 3U:
		data.getData(m_audio + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH);
		m_audioCount = 4U;
		if (!data.isEnd())
			return true;
		break;
	default:
		data.getData(m_audio + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH);
		m_audioCount = 0U;
		break;
	}

	if (m_seqNo == 0U) {
		bool ret = writeHeader(data);
		if (!ret)
			return false;
		return writeCommunication(data);
	} else if (data.isEnd()) {
		return writeTerminator(data);
	} else {
		return writeCommunication(data);
	}
}

bool CYSFNetwork::writeHeader(CData& data)
{
	uint8_t buffer[200U];

	buffer[0] = 'Y';
	buffer[1] = 'S';
	buffer[2] = 'F';
	buffer[3] = 'D';

	for (unsigned int i = 0U; i < YSF_CALLSIGN_LENGTH; i++)
		buffer[i + 4U] = m_callsign.at(i);

	uint8_t source[YSF_CALLSIGN_LENGTH];
	uint8_t destination[YSF_CALLSIGN_LENGTH];
	uint8_t dgId = 0U;
	data.getYSF(source, destination, dgId);

	::memcpy(buffer + 14U, source,      YSF_CALLSIGN_LENGTH);
	::memcpy(buffer + 24U, destination, YSF_CALLSIGN_LENGTH);

	buffer[34U] = 0x00U;

	::memcpy(buffer + 35U, YSF_SYNC_BYTES, YSF_SYNC_LENGTH_BYTES);

	CYSFFICH fich;
	fich.setFI(YSF_FI_HEADER);
	fich.setCS(YSF_CS_ASSIGN);
	fich.setCM(YSF_CM_GROUP_CQ);
	fich.setBN(0U);
	fich.setBT(0U);
	fich.setFN(0U);
	fich.setFT(6U);
	fich.setDev(false);
	fich.setMR(YSF_MR_DIRECT);
	fich.setVoIP(false);
	fich.setDT(YSF_DT_VD_MODE2);
	fich.setDGId(dgId);
	fich.encode(buffer + 35U);

	CYSFPayload payload;
	payload.createHeaderData(buffer + 35U, source, destination, YSF_NULL_CALLSIGN1, YSF_NULL_CALLSIGN1);

	m_seqNo++;
	m_fn = 0U;

	if (m_debug)
		CUtils::dump(1U, "YSF Network Data Sent", buffer, 155U);

	return m_socket.write(buffer, 155U, m_addr, m_addrLen);
}

bool CYSFNetwork::writeCommunication(CData& data)
{
	uint8_t buffer[200U];

	buffer[0] = 'Y';
	buffer[1] = 'S';
	buffer[2] = 'F';
	buffer[3] = 'D';

	for (unsigned int i = 0U; i < YSF_CALLSIGN_LENGTH; i++)
		buffer[i + 4U] = m_callsign.at(i);

	uint8_t source[YSF_CALLSIGN_LENGTH];
	uint8_t destination[YSF_CALLSIGN_LENGTH];
	uint8_t dgId = 0U;
	data.getYSF(source, destination, dgId);

	::memcpy(buffer + 14U, source,      YSF_CALLSIGN_LENGTH);
	::memcpy(buffer + 24U, destination, YSF_CALLSIGN_LENGTH);

	buffer[34U] = (m_seqNo & 0x7FU) << 1;

	::memcpy(buffer + 35U, YSF_SYNC_BYTES, YSF_SYNC_LENGTH_BYTES);

	CYSFFICH fich;
	fich.setFI(YSF_FI_COMMUNICATIONS);
	fich.setCS(YSF_CS_ASSIGN);
	fich.setCM(YSF_CM_GROUP_CQ);
	fich.setBN(0U);
	fich.setBT(0U);
	fich.setFN(m_fn);
	fich.setFT(6U);
	fich.setDev(false);
	fich.setMR(YSF_MR_DIRECT);
	fich.setVoIP(false);
	fich.setDT(YSF_DT_VD_MODE2);
	fich.setDGId(dgId);
	fich.encode(buffer + 35U);

	CYSFPayload payload;
	payload.createVDMode2Audio(buffer + 35U, m_audio);

	switch (m_fn) {
	case 2U:	// Downlink
	case 3U:	// Uplink
	case 4U:	// Rem1+2
	case 5U:	// Rem3+4
		payload.createVDMode2Data(buffer + 35U, YSF_NULL_CALLSIGN1);
		break;
	case 0U:	// Destination
		payload.createVDMode2Data(buffer + 35U, destination);
		break;
	case 1U:	// Source
		payload.createVDMode2Data(buffer + 35U, source);
		break;
	default:	// DT1
		payload.createVDMode2Data(buffer + 35U, YSF_NULL_DT);
		break;
	}

	m_seqNo++;

	m_fn++;
	if (m_fn > 6U)
		m_fn = 0U;

	if (m_debug)
		CUtils::dump(1U, "YSF Network Data Sent", buffer, 155U);

	return m_socket.write(buffer, 155U, m_addr, m_addrLen);
}

bool CYSFNetwork::writeTerminator(CData& data)
{
	uint8_t buffer[200U];

	buffer[0] = 'Y';
	buffer[1] = 'S';
	buffer[2] = 'F';
	buffer[3] = 'D';

	for (unsigned int i = 0U; i < YSF_CALLSIGN_LENGTH; i++)
		buffer[i + 4U] = m_callsign.at(i);

	uint8_t source[YSF_CALLSIGN_LENGTH];
	uint8_t destination[YSF_CALLSIGN_LENGTH];
	uint8_t dgId = 0U;
	data.getYSF(source, destination, dgId);

	::memcpy(buffer + 14U, source,      YSF_CALLSIGN_LENGTH);
	::memcpy(buffer + 24U, destination, YSF_CALLSIGN_LENGTH);

	buffer[34U]  = (m_seqNo & 0x7FU) << 1;
	buffer[34U] |= 0x01U;

	::memcpy(buffer + 35U, YSF_SYNC_BYTES, YSF_SYNC_LENGTH_BYTES);

	CYSFFICH fich;
	fich.setFI(YSF_FI_TERMINATOR);
	fich.setCS(YSF_CS_ASSIGN);
	fich.setCM(YSF_CM_GROUP_CQ);
	fich.setBN(0U);
	fich.setBT(0U);
	fich.setFN(0U);
	fich.setFT(6U);
	fich.setDev(false);
	fich.setMR(YSF_MR_DIRECT);
	fich.setVoIP(false);
	fich.setDT(YSF_DT_VD_MODE2);
	fich.setDGId(dgId);
	fich.encode(buffer + 35U);

	CYSFPayload payload;
	payload.createHeaderData(buffer + 35U, source, destination, YSF_NULL_CALLSIGN1, YSF_NULL_CALLSIGN1);

	if (m_debug)
		CUtils::dump(1U, "YSF Network Data Sent", buffer, 155U);

	return m_socket.write(buffer, 155U, m_addr, m_addrLen);
}

bool CYSFNetwork::writePoll()
{
	uint8_t buffer[20U];

	buffer[0] = 'Y';
	buffer[1] = 'S';
	buffer[2] = 'F';
	buffer[3] = 'P';

	for (unsigned int i = 0U; i < YSF_CALLSIGN_LENGTH; i++)
		buffer[i + 4U] = m_callsign.at(i);

	// if (m_debug)
	//	CUtils::dump(1U, "YSF Network Poll Sent", buffer, 14U);

	return m_socket.write(buffer, 14U, m_addr, m_addrLen);
}

void CYSFNetwork::clock(unsigned int ms)
{
	m_pollTimer.clock(ms);
	if (m_pollTimer.hasExpired()) {
		writePoll();
		m_pollTimer.start();
	}

	uint8_t buffer[BUFFER_LENGTH];

	sockaddr_storage address;
	size_t addrLen;
	int length = m_socket.read(buffer, BUFFER_LENGTH, address, addrLen);
	if (length <= 0)
		return;

	if (!CUDPSocket::match(m_addr, address)) {
		LogMessage("YSF, packet received from an invalid source");
		return;
	}

	// Invalid packet type?
	if (::memcmp(buffer, "YSFD", 4U) != 0)
		return;

	if (m_debug)
		CUtils::dump(1U, "YSF Network Data Received", buffer, length);

	if (::memcmp(m_tag, "          ", YSF_CALLSIGN_LENGTH) == 0) {
		::memcpy(m_tag, buffer + 4U, YSF_CALLSIGN_LENGTH);
	} else {
		if (::memcmp(m_tag, buffer + 4U, YSF_CALLSIGN_LENGTH) != 0)
			return;
	}

	m_buffer.add(buffer, 155U);
}

bool CYSFNetwork::read(CData& data)
{
	switch (m_audioCount) {
	case 1U:
		data.setData(m_audio + YSFDN_DATA_LENGTH);
		m_audioCount = 2U;
		return true;
	case 2U:
		data.setData(m_audio + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH);
		m_audioCount = 3U;
		return true;
	case 3U:
		data.setData(m_audio + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH);
		m_audioCount = 4U;
		return true;
	case 4U:
		data.setData(m_audio + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH + YSFDN_DATA_LENGTH);
		m_audioCount = 0U;
		return true;
	default:
		break;
	}

	if (m_buffer.empty())
		return false;

	uint8_t buffer[155U];
	m_buffer.get(buffer, 155U);

	CYSFFICH fich;
	fich.decode(buffer + 35U);

	switch (fich.getFI()) {
	case YSF_FI_HEADER:
		processHeader(buffer, data, fich.getDGId());
		return true;
	case YSF_FI_COMMUNICATIONS:
		break;
	case YSF_FI_TERMINATOR:
		data.setEnd();
		return true;
	default:
		return false;
	}

	if (fich.getDT() != YSF_DT_VD_MODE2)
		return false;

	CYSFPayload payload;
	payload.processVDMode2Audio(buffer + 35U, m_audio);

#if defined(DUMP_YSF)
	if (m_fpIn != nullptr) {
		::fwrite(m_audio, 1U, YSFDN_DATA_LENGTH * 5U, m_fpIn);
		::fflush(m_fpIn);
	}
#endif

	data.setData(m_audio + 0U);

	m_audioCount = 1U;

	return true;
}

void CYSFNetwork::processHeader(const uint8_t* buffer, CData& data, uint8_t dgId)
{
	assert(buffer != nullptr);

	uint8_t source[YSF_CALLSIGN_LENGTH];
	uint8_t destination[YSF_CALLSIGN_LENGTH];
	uint8_t uplink[YSF_CALLSIGN_LENGTH];
	uint8_t downlink[YSF_CALLSIGN_LENGTH];

	CYSFPayload payload;
	payload.processHeaderData(buffer + 35U, source, destination, uplink, downlink);

	data.setYSF(source, dgId);
}

void CYSFNetwork::reset()
{
	::memset(m_tag, ' ', YSF_CALLSIGN_LENGTH);
	m_buffer.clear();
	m_audioCount = 0U;
	m_seqNo      = 0U;
	m_fn         = 0U;
}

bool CYSFNetwork::hasData()
{
	return m_buffer.hasData() || (m_audioCount > 0U);
}

void CYSFNetwork::close()
{
	m_socket.close();

#if defined(DUMP_YSF)
	if (m_fpIn != nullptr) {
		::fclose(m_fpIn);
		m_fpIn = nullptr;
	}

	if (m_fpOut != nullptr) {
		::fclose(m_fpOut);
		m_fpOut = nullptr;
	}
#endif

	LogMessage("Closing YSF network connection");
}
