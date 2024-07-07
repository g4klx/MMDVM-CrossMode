/*
 *   Copyright (C) 2018,2024 by Jonathan Naylor G4KLX
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

#if !defined(NXDNLICH_H)
#define  NXDNLICH_H

#include <cstdint>

class CNXDNLICH {
public:
	CNXDNLICH(const CNXDNLICH& lich);
	CNXDNLICH();
	~CNXDNLICH();

	bool decode(const uint8_t* bytes);

	void encode(uint8_t* bytes);

	uint8_t getRFCT() const;
	uint8_t getFCT() const;
	uint8_t getOption() const;
	uint8_t getDirection() const;
	uint8_t getRaw() const;
	
	void setRFCT(uint8_t rfct);
	void setFCT(uint8_t usc);
	void setOption(uint8_t option);
	void setDirection(uint8_t direction);
	void setRaw(uint8_t lich);

	CNXDNLICH& operator=(const CNXDNLICH& lich);

private:
	uint8_t* m_lich;

	bool getParity() const;
};

#endif
