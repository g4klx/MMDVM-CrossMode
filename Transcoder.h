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

#ifndef Transcoder_H
#define Transcoder_H

#include "UARTController.h"

#include <string>

class CTranscoder {
public:
	CTranscoder(const std::string& port, uint32_t speed, bool debug);
	~CTranscoder();

	bool open();

	bool setConversion(uint8_t inMode, uint8_t outMode);

	uint16_t read(uint8_t* data);
	bool     write(const uint8_t* data);

	void close();

private:
	CUARTController m_serial;
	bool            m_debug;
	uint8_t         m_inMode;
	uint8_t         m_outMode;
	uint8_t         m_hasAMBE;

	bool           validateOptions() const;
	int16_t        write(const uint8_t* buffer, uint16_t length);
	uint16_t       read(uint8_t* buffer, uint16_t timeout);
	uint16_t       getBlockLength(uint8_t mode) const;
	const uint8_t* getDataHeader(uint8_t mode) const;
};

#endif
