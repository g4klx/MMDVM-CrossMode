/*
 *   Copyright (C) 2016,2017,2019,2020,2024 by Jonathan Naylor G4KLX
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

#if !defined(YSFFICH_H)
#define  YSFFICH_H

#include <cstdint>

class CYSFFICH {
public:
	CYSFFICH(const CYSFFICH& fich);
	CYSFFICH();
	~CYSFFICH();

	bool decode(const uint8_t* bytes);

	void encode(uint8_t* bytes);

	uint8_t getFI() const;
	uint8_t getCS() const;
	uint8_t getCM() const;
	uint8_t getBN() const;
	uint8_t getBT() const;
	uint8_t getFN() const;
	uint8_t getFT() const;
	bool    getDev() const;
	uint8_t getMR() const;
	bool    getVoIP() const;
	uint8_t getDT() const;
	uint8_t getDGId() const;

	void setFI(uint8_t fi);
	void setCS(uint8_t cs);
	void setCM(uint8_t cm);
	void setBN(uint8_t bn);
	void setBT(uint8_t bt);
	void setFN(uint8_t fn);
	void setFT(uint8_t ft);
	void setDev(bool set);
	void setMR(uint8_t mr);
	void setVoIP(bool set);
	void setDT(uint8_t dt);
	void setDGId(uint8_t id);

	CYSFFICH& operator=(const CYSFFICH& fich);

private:
	uint8_t* m_fich;
};

#endif
