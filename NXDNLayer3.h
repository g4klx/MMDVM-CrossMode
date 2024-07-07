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

#if !defined(NXDNLayer3_H)
#define  NXDNLayer3_H

#include <cstdint>

class CNXDNLayer3 {
public:
	CNXDNLayer3(const CNXDNLayer3& layer3);
	CNXDNLayer3();
	~CNXDNLayer3();

	void decode(const uint8_t* bytes, unsigned int length, unsigned int offset = 0U);

	void encode(uint8_t* bytes, unsigned int length, unsigned int offset = 0U);

	uint8_t  getMessageType() const;
	uint16_t getSourceUnitId() const;
	uint16_t getDestinationGroupId() const;
	bool     getIsGroup() const;
	uint8_t  getDataBlocks() const;

	void     setMessageType(uint8_t type);
	void     setSourceUnitId(uint16_t id);
	void     setDestinationGroupId(uint16_t id);
	void     setIsGroup(bool grp);
	void     setDataBlocks(uint8_t n);

	void     setData(const uint8_t* data, unsigned int length);
	void     getData(uint8_t* data) const;

	void     reset();

	CNXDNLayer3& operator=(const CNXDNLayer3& layer3);

private:
	uint8_t* m_data;
};

#endif
