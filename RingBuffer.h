/*
 *   Copyright (C) 2006-2009,2012,2013,2015,2016,2024 by Jonathan Naylor G4KLX
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

#if !defined(RingBuffer_H)
#define RingBuffer_H

#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdint>

template<class T> class CRingBuffer {
public:
	CRingBuffer(uint16_t length, const char* name) :
	m_length(length),
	m_name(name),
	m_buffer(nullptr),
	m_iPtr(0U),
	m_oPtr(0U)
	{
		assert(length > 0U);
		assert(name != nullptr);

		m_buffer = new T[length];

		::memset(m_buffer, 0x00, m_length * sizeof(T));
	}

	~CRingBuffer()
	{
		delete[] m_buffer;
	}

	bool add(const T* buffer, uint16_t nSamples)
	{
		assert(buffer != nullptr);
		assert(nSamples > 0U);

		if (nSamples >= free()) {
			LogError("%s buffer overflow, clearing the buffer. (%u >= %u)", m_name, nSamples, free());
			clear();
			return false;
		}

		for (uint16_t i = 0U; i < nSamples; i++) {
			m_buffer[m_iPtr++] = buffer[i];

			if (m_iPtr == m_length)
				m_iPtr = 0U;
		}

		return true;
	}

	bool get(T* buffer, uint16_t nSamples)
	{
		assert(buffer != nullptr);
		assert(nSamples > 0U);

		if (size() < nSamples) {
			LogError("**** Underflow in %s ring buffer, %u < %u", m_name, size(), nSamples);
			return false;
		}

		for (uint16_t i = 0U; i < nSamples; i++) {
			buffer[i] = m_buffer[m_oPtr++];

			if (m_oPtr == m_length)
				m_oPtr = 0U;
		}

		return true;
	}

	bool remove(uint16_t nSamples)
	{
		assert(nSamples > 0U);

		if (size() < nSamples) {
			LogError("**** Underflow in %s ring buffer, %u < %u", m_name, size(), nSamples);
			return false;
		}

		for (uint16_t i = 0U; i < nSamples; i++) {
			m_oPtr++;
			if (m_oPtr == m_length)
				m_oPtr = 0U;
		}

		return true;
	}

	bool peek(T* buffer, uint16_t nSamples) const
	{
		assert(buffer != nullptr);
		assert(nSamples > 0U);

		if (size() < nSamples) {
			LogError("**** Underflow peek in %s ring buffer, %u < %u", m_name, size(), nSamples);
			return false;
		}

		uint16_t ptr = m_oPtr;
		for (uint16_t i = 0U; i < nSamples; i++) {
			buffer[i] = m_buffer[ptr++];

			if (ptr == m_length)
				ptr = 0U;
		}

		return true;
	}

	void clear()
	{
		m_iPtr = 0U;
		m_oPtr = 0U;

		::memset(m_buffer, 0x00, m_length * sizeof(T));
	}

	uint16_t free() const
	{
		uint16_t len = m_length;

		if (m_oPtr > m_iPtr)
			len = m_oPtr - m_iPtr;
		else if (m_iPtr > m_oPtr)
			len = m_length - (m_iPtr - m_oPtr);

		if (len > m_length)
			len = 0U;

		return len;
	}

	uint16_t size() const
	{
		return m_length - free();
	}

	bool hasSpace(uint16_t length) const
	{
		return free() > length;
	}

	bool hasData() const
	{
		return m_oPtr != m_iPtr;
	}

	bool empty() const
	{
		return m_oPtr == m_iPtr;
	}

private:
	uint16_t     m_length;
	const char*  m_name;
	T*           m_buffer;
	uint16_t     m_iPtr;
	uint16_t     m_oPtr;
};

#endif
