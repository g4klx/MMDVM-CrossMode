/*
 *	Copyright (C) 2009,2014,2015,2024 by Jonathan Naylor, G4KLX
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 */

#ifndef	Utils_H
#define	Utils_H

#include <cstdint>
#include <string>

class CUtils {
public:
	static void dump(const std::string& title, const uint8_t* data, unsigned int length);
	static void dump(int level, const std::string& title, const uint8_t* data, unsigned int length);

	static void dump(const std::string& title, const bool* bits, unsigned int length);
	static void dump(int level, const std::string& title, const bool* bits, unsigned int length);

	static void byteToBitsBE(uint8_t byte, bool* bits);
	static void byteToBitsLE(uint8_t byte, bool* bits);

	static void bitsToByteBE(const bool* bits, uint8_t& byte);
	static void bitsToByteLE(const bool* bits, uint8_t& byte);

private:
};

#endif
