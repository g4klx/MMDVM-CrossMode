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

#if !defined(NXDNLOOKUP_H)
#define	NXDNLOOKUP_H

#include "Timer.h"

#include <string>
#include <vector>

#include <cstdint>

class CNXDNLookup {
public:
	CNXDNLookup();
	~CNXDNLookup();

	bool load(const std::string& filename, unsigned int reloadTime);

	std::string lookup(uint16_t id) const;
	uint16_t lookup(const std::string& callsign) const;

	void clock(unsigned int ms);

private:
	std::string                                   m_filename;
	std::vector<std::pair<uint16_t, std::string>> m_data;
	CTimer                                        m_timer;

	bool load();
};

#endif
