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

#if !defined(NXDNSACCH_H)
#define	NXDNSACCH_H

#include <cstdint>

class CNXDNSACCH {
public:
	CNXDNSACCH(const CNXDNSACCH& sacch);
	CNXDNSACCH();
	~CNXDNSACCH();

	bool decode(const uint8_t* data);

	void encode(uint8_t* data) const;

	uint8_t getRAN() const;
	uint8_t getStructure() const;

	void getData(uint8_t* data) const;
	void getRaw(uint8_t* data) const;

	void setRAN(uint8_t ran);
	void setStructure(uint8_t structure);

	void setData(const uint8_t* data);
	void setRaw(const uint8_t* data);

	CNXDNSACCH& operator=(const CNXDNSACCH& sacch);

private:
	uint8_t* m_data;
};

#endif
