/*
 *   Copyright (C) 2024 by Jonathan Naylor G4KLX
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

#include "Transcoder.h"

#include "TranscoderDefines.h"
#include "StopWatch.h"
#include "Utils.h"

#include <cstring>
#include <cassert>


CTranscoder::CTranscoder(const std::string& port, uint32_t speed, bool debug) :
m_serial(port, speed),
m_debug(debug),
m_inMode(MODE_PCM),
m_outMode(MODE_PCM),
m_hasAMBE(NO_AMBE_CHIP)
{
	assert(!port.empty());
	assert(speed > 0U);
}

CTranscoder::~CTranscoder()
{
}

bool CTranscoder::open()
{
	bool ret1 = m_serial.open();
	if (!ret1) {
		::fprintf(stderr, "Transcoder: cannot open the transcoder port\n");
		return false;
	}

	int16_t ret2 = write(GET_VERSION, GET_VERSION_LEN);
	if (ret2 <= 0) {
		::fprintf(stderr, "Error writing the data to the transcoder\n");
		return false;
	}

	uint8_t buffer[400U];
	uint16_t len = read(buffer, 200U);
	if (len == 0U) {
		::fprintf(stderr, "Transcoder version read timeout (200 me)\n");
		m_serial.close();
		return false;
	}

	if (m_debug)
		CUtils::dump("Transcoder read", buffer, len);

	switch (buffer[TYPE_POS]) {
	case TYPE_NAK:
		::fprintf(stderr, "NAK returned for get version - %u\n", buffer[NAK_ERROR_POS]);
		m_serial.close();
		return false;

	case TYPE_GET_VERSION:
		if (buffer[GET_VERSION_PROTOCOL_POS] != PROTOCOL_VERSION) {
			::fprintf(stderr, "Unknown protocol version - %u\n", buffer[GET_VERSION_PROTOCOL_POS]);
			m_serial.close();
			return false;
		}

		::fprintf(stdout, "Transcoder version - %.*s\n", len - 5U, buffer + 5U);
		break;

	default:
		::fprintf(stderr, "Unknown response from the transcoder to get version - 0x%02X\n", buffer[TYPE_POS]);
		m_serial.close();
		return false;
	}

	ret2 = write(GET_CAPABILITIES, GET_CAPABILITIES_LEN);
	if (ret2 <= 0) {
		::fprintf(stderr, "Error writing data to the transcoder\n");
		return false;
	}

	len = read(buffer, 50U);
	if (len == 0U) {
		::fprintf(stderr, "Transcoder capabilities read timeout (200 me)\n");
		m_serial.close();
		return false;
	}

	if (m_debug)
		CUtils::dump("Transcoder read", buffer, len);

	switch (buffer[TYPE_POS]) {
	case TYPE_NAK:
		::fprintf(stderr, "NAK returned for get capabilities - %u\n", buffer[NAK_ERROR_POS]);
		m_serial.close();
		return false;

	case TYPE_GET_CAPABILITIES:
		break;

	default:
		::fprintf(stderr, "Unknown response from the transcoder to get capabilities - 0x%02X\n", buffer[TYPE_POS]);
		m_serial.close();
		return false;
	}

	m_hasAMBE = buffer[GET_CAPABILITIES_AMBE_TYPE_POS];
	switch (m_hasAMBE) {
	case HAS_1AMBE_CHIP:
		::fprintf(stdout, "Transcoder has 1 AMBE chip\n");
		break;
	case HAS_2AMBE_CHIPS:
		::fprintf(stdout, "Transcoder has 2 AMBE chips\n");
		break;
	default:
		::fprintf(stdout, "Transcoder has no AMBE chips\n");
		break;
	}

	return true;
}

bool CTranscoder::setConversion(uint8_t inMode, uint8_t outMode)
{
	uint8_t command[10U];
	::memcpy(command + 0U, SET_MODE_HEADER, SET_MODE_HEADER_LEN);
	command[INPUT_MODE_POS]  = inMode;
	command[OUTPUT_MODE_POS] = outMode;

	int16_t ret = write(command, SET_MODE_LEN);
	if (ret <= 0) {
		::fprintf(stderr, "Error writing data to the transcoder\n");
		return false;
	}

	uint8_t buffer[50U];
	uint16_t len = read(buffer, 200U);
	if (len == 0U) {
		::fprintf(stderr, "Set mode read timeout (200 me)\n");
		return false;
	}

	if (m_debug)
		CUtils::dump("Transcoder read", buffer, len);

	switch (buffer[TYPE_POS]) {
	case TYPE_NAK:
		::fprintf(stderr, "NAK returned for set mode - %u\n", buffer[NAK_ERROR_POS]);
		return false;

	case TYPE_ACK:
		::fprintf(stdout, "Conversion modes - set\n");
		m_inMode  = inMode;
		m_outMode = outMode;
		return true;

	default:
		::fprintf(stderr, "Unknown response from the transcoder to set mode - 0x%02X\n", buffer[TYPE_POS]);
		return false;
	}
}

void CTranscoder::close()
{
	m_serial.close();
}

uint16_t CTranscoder::read(uint8_t* buffer, uint16_t timeout)
{
	assert(buffer != NULL);

	uint16_t len = 0U;
	uint16_t ptr = 0U;

	CStopWatch stopwatch;
	stopwatch.start();

	for (;;) {
		uint8_t c = 0U;
		if (m_serial.read(&c, 1U) == 1) {
			if (ptr == MARKER_POS) {
				if (c == MARKER) {
					// Handle the frame start correctly
					buffer[0U] = c;
					ptr = 1U;
					len = 0U;
				} else {
					ptr = 0U;
					len = 0U;
				}
			} else if (ptr == LENGTH_LSB_POS) {
				// Handle the frame length LSB
				uint8_t val = buffer[ptr] = c;
				len = (val << 0) & 0x00FFU;
				ptr = 2U;
			} else if (ptr == LENGTH_MSB_POS) {
				// Handle the frame length MSB
				uint8_t val = buffer[ptr] = c;
				len |= (val << 8) & 0xFF00U;
				ptr = 3U;
			} else {
				// Any other bytes are added to the buffer
				buffer[ptr] = c;
				ptr++;

				// The full packet has been received, process it
				if (ptr == len)
					return len;
			}
		} else {
			unsigned long elapsed = stopwatch.elapsed() / 1000U;
			if (elapsed > timeout) {
				::fprintf(stderr, "Read has timed out after %u ms\n", timeout);
				return len;
			}
		}

		if ((ptr == 0U) && (timeout == 0U))
			return 0U;
	}
}

int16_t CTranscoder::write(const uint8_t* buffer, uint16_t length)
{
	assert(buffer != nullptr);
	assert(length > 0U);

	if (m_debug)
		CUtils::dump("Transcoder write", buffer, length);

	return m_serial.write(buffer, length);
}

uint16_t CTranscoder::read(uint8_t* data)
{
	assert(data != nullptr);

	uint16_t length = getBlockLength(m_outMode);

	uint8_t buffer[400U];
	uint16_t len = read(buffer, 0U);
	if (len == 0U)
		return 0U;

	if (m_debug)
		CUtils::dump("Transcoder read", buffer, len);

	switch (buffer[TYPE_POS]) {
	case TYPE_NAK:
		::fprintf(stderr, "NAK returned for transcoding - %u\n", buffer[NAK_ERROR_POS]);
		return false;

	case TYPE_DATA:
		break;

	default:
		::fprintf(stderr, "Unknown response from the transcoder to transcoding - 0x%02X\n", buffer[TYPE_POS]);
		return false;
	}

	::memcpy(data, buffer + DATA_START_POS, length);

	return length;
}

bool CTranscoder::write(const uint8_t* data)
{
	assert(data != nullptr);

	uint16_t length       = getBlockLength(m_inMode);
	const uint8_t* header = getDataHeader(m_inMode);

	uint8_t buffer[400U];
	::memcpy(buffer + 0U, header, DATA_HEADER_LEN);
	::memcpy(buffer + DATA_START_POS, data, length);

	int16_t ret = write(buffer, length + DATA_HEADER_LEN);
	if (ret <= 0) {
		::fprintf(stderr, "Error writing the data to the transcoder\n");
		return false;
	}

	return true;
}

uint16_t CTranscoder::getBlockLength(uint8_t mode) const
{
	switch (mode) {
	case MODE_DSTAR:
		return DSTAR_DATA_LENGTH;
	case MODE_DMR_NXDN:
		return DMR_NXDN_DATA_LENGTH;
	case MODE_YSFDN:
		return YSFDN_DATA_LENGTH;
	case MODE_IMBE:
		return IMBE_DATA_LENGTH;
	case MODE_IMBE_FEC:
		return IMBE_FEC_DATA_LENGTH;
	case MODE_CODEC2_3200:
		return CODEC2_3200_DATA_LENGTH;
	default:
		return PCM_DATA_LENGTH;
	}
}

const uint8_t* CTranscoder::getDataHeader(uint8_t mode) const
{
	switch (mode) {
	case MODE_DSTAR:
		return DSTAR_DATA_HEADER;
	case MODE_DMR_NXDN:
		return DMR_NXDN_DATA_HEADER;
	case MODE_YSFDN:
		return YSFDN_DATA_HEADER;
	case MODE_IMBE:
		return IMBE_DATA_HEADER;
	case MODE_IMBE_FEC:
		return IMBE_FEC_DATA_HEADER;
	case MODE_CODEC2_3200:
		return CODEC2_3200_DATA_HEADER;
	default:
		return PCM_DATA_HEADER;
	}
}

bool CTranscoder::validateOptions() const
{
	switch (m_hasAMBE) {
	case HAS_1AMBE_CHIP:
		if ((m_inMode == MODE_DSTAR) && (m_outMode == MODE_DMR_NXDN)) {
			::fprintf(stderr, "Transcoding isn't possible without an AMBE chip\n");
			return false;
		}
		if ((m_inMode == MODE_DMR_NXDN) && (m_outMode == MODE_DSTAR)) {
			::fprintf(stderr, "Transcoding isn't possible without an AMBE chip\n");
			return false;
		}
		if ((m_inMode == MODE_DSTAR) && (m_outMode == MODE_YSFDN)) {
			::fprintf(stderr, "Transcoding isn't possible without an AMBE chip\n");
			return false;
		}
		if ((m_inMode == MODE_YSFDN) && (m_outMode == MODE_DSTAR)) {
			::fprintf(stderr, "Transcoding isn't possible without an AMBE chip\n");
			return false;
		}
		break;

	case HAS_2AMBE_CHIPS:
		break;

	default:
		if ((m_inMode == MODE_DSTAR) && (m_outMode != MODE_DSTAR)) {
			::fprintf(stderr, "Transcoding isn't possible without an AMBE chip\n");
			return false;
		}
		if ((m_inMode != MODE_DSTAR) && (m_outMode == MODE_DSTAR)) {
			::fprintf(stderr, "Transcoding isn't possible without an AMBE chip\n");
			return false;
		}
		if ((m_inMode == MODE_DMR_NXDN) && ((m_outMode != MODE_DMR_NXDN) && (m_outMode != MODE_YSFDN))) {
			::fprintf(stderr, "Transcoding isn't possinle without an AMBE chip\n");
			return false;
		}
		if ((m_outMode == MODE_DMR_NXDN) && ((m_inMode != MODE_DMR_NXDN) && (m_inMode != MODE_YSFDN))) {
			::fprintf(stderr, "Transcoding isn't possinle without an AMBE chip\n");
			return false;
		}
		if ((m_inMode == MODE_YSFDN) && ((m_outMode != MODE_DMR_NXDN) && (m_outMode != MODE_YSFDN))) {
			::fprintf(stderr, "Transcoding isn't possinle without an AMBE chip\n");
			return false;
		}
		if ((m_outMode == MODE_YSFDN) && ((m_inMode != MODE_DMR_NXDN) && (m_inMode != MODE_YSFDN))) {
			::fprintf(stderr, "Transcoding isn't possinle without an AMBE chip\n");
			return false;
		}
		break;
	}

	return true;
}
