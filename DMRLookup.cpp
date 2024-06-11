/*
 *	 Copyright (C) 2024 by Jonathan Naylor G4KLX
 *
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the Free Software
 *	 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "DMRLookup.h"
#include "Log.h"

#include <cstdio>
#include <cstring>
#include <cassert>

const std::string NULL_CALLSIGN = "";
const uint32_t    NULL_ID       = 0xFFFFFFFFU;

const unsigned int BUFFER_SIZE = 200U;

CDMRLookup::CDMRLookup() :
m_filename(),
m_data(),
m_timer(1000U)
{
}

CDMRLookup::~CDMRLookup()
{
	m_data.clear();
}

bool CDMRLookup::load(const std::string& filename, unsigned int reloadTime)
{
	assert(!filename.empty());

	m_filename = filename;

	m_timer.setTimeout(reloadTime * 60U * 60U);

	return load();
}

bool CDMRLookup::load()
{
	m_data.clear();

	FILE* fp = ::fopen(m_filename.c_str(), "rt");
	if (fp == nullptr) {
		LogError("Unable to open the lookup file - %s", m_filename.c_str());
		return false;
	}

	char buffer[BUFFER_SIZE];
	while (::fgets(buffer, BUFFER_SIZE, fp) != nullptr) {
		char* p1 = ::strtok(buffer, ", \t\r\n");
		char* p2 = ::strtok(nullptr, ", \t\r\n");

		if ((p1 == nullptr) || (p2 == nullptr))
			continue;

		if (*p1 == '#')
			continue;

		int id = ::atoi(p1);
		if (id <= 0)
			continue;

		std::string callsign = std::string(p2);

		m_data.push_back(std::pair<uint32_t, std::string>(uint32_t(id), callsign));
	}

	m_timer.start();

	return true;
}

std::string CDMRLookup::lookup(uint32_t id) const
{
	for (const auto& it : m_data) {
		if (it.first == id)
			return it.second;
	}

	return NULL_CALLSIGN;
}

uint32_t CDMRLookup::lookup(const std::string& callsign) const
{
	for (const auto& it : m_data) {
		if (it.second == callsign)
			return it.first;
	}

	return NULL_ID;
}

void CDMRLookup::clock(unsigned int ms)
{
	m_timer.clock(ms);
	if (m_timer.isRunning() && m_timer.hasExpired()) {
		load();
		m_timer.start();
	}
}
