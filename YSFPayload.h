/*
*   Copyright (C) 2016,2017,2020,2024 by Jonathan Naylor G4KLX
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

#if !defined(YSFPayload_H)
#define	YSFPayload_H

#include <cstdint>

#include <string>

class CYSFPayload {
public:
	CYSFPayload();
	~CYSFPayload();

	bool processHeaderData(const uint8_t* bytes, uint8_t* source, uint8_t* destination, uint8_t* downlink, uint8_t* uplink) const;
	void createHeaderData(uint8_t* bytes, const uint8_t* source, const uint8_t* destination, const uint8_t* downlink, const uint8_t* uplink) const;

	bool processVDMode2Data(const uint8_t* bytes, uint8_t* data) const;
	void processVDMode2Audio(const uint8_t* bytes, uint8_t* audio) const;

	void createVDMode2Data(uint8_t* bytes, const uint8_t* data) const;
	void createVDMode2Audio(uint8_t* bytes, const uint8_t* audio) const;

/*
	bool processVoiceFRModeData(uint8_t* bytes);

	unsigned int processVoiceFRModeAudio2(uint8_t* bytes);
	unsigned int processVoiceFRModeAudio5(uint8_t* bytes);
*/
private:
};

#endif
