/*
 *   Copyright (C) 2015,2016,2019,2021,2022,2024 by Jonathan Naylor G4KLX
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

#if !defined(DMRLC_H)
#define DMRLC_H

#include "BPTC19696.h"
#include "DMRDefines.h"

#include <cstdint>

class CDMRLC
{
public:
	CDMRLC();
	~CDMRLC();

	void setParameters(FLCO flco, uint32_t srcId, uint32_t dstId);

	void encode(uint8_t* data, uint8_t type);

	void getData(uint8_t* data, uint8_t n) const;

private:
	bool       m_PF;
	bool       m_R;
	FLCO       m_FLCO;
	uint8_t    m_FID;
	uint8_t    m_options;
	uint32_t   m_srcId;
	uint32_t   m_dstId;
	CBPTC19696 m_bptc;
	bool*      m_raw;

	void getData(uint8_t* bytes) const;
	void getData(bool* bits) const;

	void encodeEmbeddedData();
};

#endif

