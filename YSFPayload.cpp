/*
*	Copyright (C) 2016,2017,2020,2024 Jonathan Naylor, G4KLX
*	Copyright (C) 2016 Mathias Weyland, HB9FRV
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

#include "TranscoderDefines.h"
#include "YSFConvolution.h"
#include "YSFPayload.h"
#include "YSFDefines.h"
#include "Utils.h"
#include "CRC.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdint>

const unsigned int INTERLEAVE_TABLE_9_20[] = {
        0U, 40U,  80U, 120U, 160U, 200U, 240U, 280U, 320U, 
        2U, 42U,  82U, 122U, 162U, 202U, 242U, 282U, 322U,
        4U, 44U,  84U, 124U, 164U, 204U, 244U, 284U, 324U,
        6U, 46U,  86U, 126U, 166U, 206U, 246U, 286U, 326U,
        8U, 48U,  88U, 128U, 168U, 208U, 248U, 288U, 328U,
       10U, 50U,  90U, 130U, 170U, 210U, 250U, 290U, 330U,
       12U, 52U,  92U, 132U, 172U, 212U, 252U, 292U, 332U,
       14U, 54U,  94U, 134U, 174U, 214U, 254U, 294U, 334U,
       16U, 56U,  96U, 136U, 176U, 216U, 256U, 296U, 336U,
       18U, 58U,  98U, 138U, 178U, 218U, 258U, 298U, 338U,
       20U, 60U, 100U, 140U, 180U, 220U, 260U, 300U, 340U,
       22U, 62U, 102U, 142U, 182U, 222U, 262U, 302U, 342U,
       24U, 64U, 104U, 144U, 184U, 224U, 264U, 304U, 344U,
       26U, 66U, 106U, 146U, 186U, 226U, 266U, 306U, 346U,
       28U, 68U, 108U, 148U, 188U, 228U, 268U, 308U, 348U,
       30U, 70U, 110U, 150U, 190U, 230U, 270U, 310U, 350U,
       32U, 72U, 112U, 152U, 192U, 232U, 272U, 312U, 352U,
       34U, 74U, 114U, 154U, 194U, 234U, 274U, 314U, 354U,
       36U, 76U, 116U, 156U, 196U, 236U, 276U, 316U, 356U,
       38U, 78U, 118U, 158U, 198U, 238U, 278U, 318U, 358U};

const unsigned int INTERLEAVE_TABLE_5_20[] = {
	0U, 40U,  80U, 120U, 160U,
	2U, 42U,  82U, 122U, 162U,
	4U, 44U,  84U, 124U, 164U,
	6U, 46U,  86U, 126U, 166U,
	8U, 48U,  88U, 128U, 168U,
	10U, 50U,  90U, 130U, 170U,
	12U, 52U,  92U, 132U, 172U,
	14U, 54U,  94U, 134U, 174U,
	16U, 56U,  96U, 136U, 176U,
	18U, 58U,  98U, 138U, 178U,
	20U, 60U, 100U, 140U, 180U,
	22U, 62U, 102U, 142U, 182U,
	24U, 64U, 104U, 144U, 184U,
	26U, 66U, 106U, 146U, 186U,
	28U, 68U, 108U, 148U, 188U,
	30U, 70U, 110U, 150U, 190U,
	32U, 72U, 112U, 152U, 192U,
	34U, 74U, 114U, 154U, 194U,
	36U, 76U, 116U, 156U, 196U,
	38U, 78U, 118U, 158U, 198U};

// This one differs from the others in that it interleaves bits and not dibits
const unsigned int INTERLEAVE_TABLE_26_4[] = {
	0U, 4U,  8U, 12U, 16U, 20U, 24U, 28U, 32U, 36U, 40U, 44U, 48U, 52U, 56U, 60U, 64U, 68U, 72U, 76U, 80U, 84U, 88U, 92U, 96U, 100U,
	1U, 5U,  9U, 13U, 17U, 21U, 25U, 29U, 33U, 37U, 41U, 45U, 49U, 53U, 57U, 61U, 65U, 69U, 73U, 77U, 81U, 85U, 89U, 93U, 97U, 101U,
	2U, 6U, 10U, 14U, 18U, 22U, 26U, 30U, 34U, 38U, 42U, 46U, 50U, 54U, 58U, 62U, 66U, 70U, 74U, 78U, 82U, 86U, 90U, 94U, 98U, 102U,
	3U, 7U, 11U, 15U, 19U, 23U, 27U, 31U, 35U, 39U, 43U, 47U, 51U, 55U, 59U, 63U, 67U, 71U, 75U, 79U, 83U, 87U, 91U, 95U, 99U, 103U};

const uint8_t WHITENING_DATA[] = {0x93U, 0xD7U, 0x51U, 0x21U, 0x9CU, 0x2FU, 0x6CU, 0xD0U, 0xEFU, 0x0FU,
										0xF8U, 0x3DU, 0xF1U, 0x73U, 0x20U, 0x94U, 0xEDU, 0x1EU, 0x7CU, 0xD8U};

const uint8_t BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])
#define READ_BIT1(p,i)    (p[(i)>>3] & BIT_MASK_TABLE[(i)&7])

CYSFPayload::CYSFPayload()
{
}

CYSFPayload::~CYSFPayload()
{
}

bool CYSFPayload::processHeaderData(const uint8_t* data, uint8_t* source, uint8_t* destination, uint8_t* downlink, uint8_t* uplink) const
{
	assert(data != nullptr);
	assert(source != nullptr);
	assert(destination != nullptr);
	assert(downlink != nullptr);
	assert(uplink != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	uint8_t dch[45U];

	const uint8_t* p1 = data;
	uint8_t*       p2 = dch;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p2, p1, 9U);
		p1 += 18U; p2 += 9U;
	}

	CYSFConvolution conv;
	conv.start();

	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];
		uint8_t s0 = READ_BIT1(dch, n) ? 1U : 0U;

		n++;
		uint8_t s1 = READ_BIT1(dch, n) ? 1U : 0U;

		conv.decode(s0, s1);
	}

	uint8_t output[23U];
	conv.chainback(output, 176U);

	bool valid = CCRC::checkCCITT162(output, 22U);
	if (!valid)
		return false;

	for (unsigned int i = 0U; i < 20U; i++)
		output[i] ^= WHITENING_DATA[i];

	::memcpy(destination, output + 0U,                  YSF_CALLSIGN_LENGTH);
	::memcpy(source,      output + YSF_CALLSIGN_LENGTH, YSF_CALLSIGN_LENGTH);

	p1 = data + 9U;
	p2 = dch;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p2, p1, 9U);
		p1 += 18U; p2 += 9U;
	}

	conv.start();

	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];
		uint8_t s0 = READ_BIT1(dch, n) ? 1U : 0U;

		n++;
		uint8_t s1 = READ_BIT1(dch, n) ? 1U : 0U;

		conv.decode(s0, s1);
	}

	conv.chainback(output, 176U);

	valid = CCRC::checkCCITT162(output, 22U);
	if (!valid)
		return false;

	for (unsigned int i = 0U; i < 20U; i++)
		output[i] ^= WHITENING_DATA[i];

	::memcpy(downlink, output + 0U,                  YSF_CALLSIGN_LENGTH);
	::memcpy(uplink,   output + YSF_CALLSIGN_LENGTH, YSF_CALLSIGN_LENGTH);

	return true;
}

void CYSFPayload::createHeaderData(uint8_t* data, const uint8_t* source, const uint8_t* destination, const uint8_t* downlink, const uint8_t* uplink) const
{
	assert(data != nullptr);
	assert(source != nullptr);
	assert(destination != nullptr);
	assert(downlink != nullptr);
	assert(uplink != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	uint8_t output[23U];
	::memcpy(output + 0U,                  destination, YSF_CALLSIGN_LENGTH);
	::memcpy(output + YSF_CALLSIGN_LENGTH, source,      YSF_CALLSIGN_LENGTH);

	for (unsigned int i = 0U; i < 20U; i++)
		output[i] ^= WHITENING_DATA[i];

	CCRC::addCCITT162(output, 22U);
	output[22U] = 0x00U;

	uint8_t convolved[45U];

	CYSFConvolution conv;
	conv.encode(output, convolved, 180U);

	uint8_t bytes[45U];
	unsigned int j = 0U;
	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];

		bool s0 = READ_BIT1(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT1(convolved, j) != 0U;
		j++;

		WRITE_BIT1(bytes, n, s0);

		n++;
		WRITE_BIT1(bytes, n, s1);
	}

	uint8_t* p1 = data;
	uint8_t* p2 = bytes;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 9U);
		p1 += 18U; p2 += 9U;
	}

	::memcpy(output + 0U,                  downlink, YSF_CALLSIGN_LENGTH);
	::memcpy(output + YSF_CALLSIGN_LENGTH, uplink,   YSF_CALLSIGN_LENGTH);

	for (unsigned int i = 0U; i < 20U; i++)
		output[i] ^= WHITENING_DATA[i];

	CCRC::addCCITT162(output, 22U);
	output[22U] = 0x00U;

	conv.encode(output, convolved, 180U);

	j = 0U;
	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];

		bool s0 = READ_BIT1(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT1(convolved, j) != 0U;
		j++;

		WRITE_BIT1(bytes, n, s0);

		n++;
		WRITE_BIT1(bytes, n, s1);
	}

	p1 = data + 9U;
	p2 = bytes;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 9U);
		p1 += 18U; p2 += 9U;
	}
}

void CYSFPayload::processVDMode2Audio(const uint8_t* data, uint8_t* audio) const
{
	assert(data != nullptr);
	assert(audio != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	unsigned int offset1 = 40U; // DCH(0)
	unsigned int offset2 = 0U;

	// We have a total of 5 VCH sections, iterate through each
	for (unsigned int j = 0U; j < 5U; j++, offset1 += 144U) {
		uint8_t vch[YSFDN_DATA_LENGTH];

		// Deinterleave
		for (unsigned int i = 0U; i < 104U; i++) {
			unsigned int n = INTERLEAVE_TABLE_26_4[i];
			bool s = READ_BIT1(data, offset1 + n);
			WRITE_BIT1(vch, i, s);
		}

		// "Un-whiten" (descramble)
		for (unsigned int i = 0U; i < YSFDN_DATA_LENGTH; i++, offset2++)
			audio[offset2] = vch[i] ^ WHITENING_DATA[i];
	}
}

void CYSFPayload::createVDMode2Audio(uint8_t* data, const uint8_t* audio) const
{
	assert(data != nullptr);
	assert(audio != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	unsigned int offset1 = 40U; // DCH(0)
	unsigned int offset2 = 0U;

	// We have a total of 5 VCH sections, iterate through each
	for (unsigned int j = 0U; j < 5U; j++, offset1 += 144U) {
		uint8_t vch[13U];

		// Scramble
		for (unsigned int i = 0U; i < YSFDN_DATA_LENGTH; i++, offset2++)
			vch[i] = audio[offset2] ^ WHITENING_DATA[i];

		// Interleave
		for (unsigned int i = 0U; i < 104U; i++) {
			unsigned int n = INTERLEAVE_TABLE_26_4[i];
			bool s = READ_BIT1(vch, i);
			WRITE_BIT1(data, offset1 + n, s);
		}
	}
}

bool CYSFPayload::processVDMode2Data(const uint8_t* data, uint8_t* dt) const
{
	assert(data != nullptr);
	assert(dt != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	uint8_t dch[25U];

	const uint8_t* p1 = data;
	uint8_t* p2 = dch;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p2, p1, 5U);
		p1 += 18U; p2 += 5U;
	}

	CYSFConvolution conv;
	conv.start();

	for (unsigned int i = 0U; i < 100U; i++) {
		unsigned int n = INTERLEAVE_TABLE_5_20[i];
		uint8_t s0 = READ_BIT1(dch, n) ? 1U : 0U;

		n++;
		uint8_t s1 = READ_BIT1(dch, n) ? 1U : 0U;

		conv.decode(s0, s1);
	}

	uint8_t output[13U];
	conv.chainback(output, 96U);

	bool ret = CCRC::checkCCITT162(output, 12U);
	if (!ret)
		return false;

	for (unsigned int i = 0U; i < YSF_CALLSIGN_LENGTH; i++)
		dt[i] = output[i] ^ WHITENING_DATA[i];

	return true;
}

void CYSFPayload::createVDMode2Data(uint8_t* data, const uint8_t* dt) const
{
	assert(data != nullptr);
	assert(dt != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	uint8_t output[13U];
	for (unsigned int i = 0U; i < YSF_CALLSIGN_LENGTH; i++)
		output[i] = dt[i] ^ WHITENING_DATA[i];

	CCRC::addCCITT162(output, 12U);
	output[12U] = 0x00U;

	uint8_t convolved[25U];

	CYSFConvolution conv;
	conv.encode(output, convolved, 100U);

	uint8_t bytes[25U];
	unsigned int j = 0U;
	for (unsigned int i = 0U; i < 100U; i++) {
		unsigned int n = INTERLEAVE_TABLE_5_20[i];

		bool s0 = READ_BIT1(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT1(convolved, j) != 0U;
		j++;

		WRITE_BIT1(bytes, n, s0);

		n++;
		WRITE_BIT1(bytes, n, s1);
	}

	uint8_t* p1 = data;
	uint8_t* p2 = bytes;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 5U);
		p1 += 18U; p2 += 5U;
	}
}

#ifdef notdef
unsigned int CYSFPayload::processVoiceFRModeAudio2(uint8_t* data)
{
	assert(data != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	// Regenerate the IMBE FEC
	unsigned int errors = 0U;
	errors += m_fec.regenerateIMBE(data + 54U);
	errors += m_fec.regenerateIMBE(data + 72U);

	return errors;
}

unsigned int CYSFPayload::processVoiceFRModeAudio5(uint8_t* data)
{
	assert(data != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	// Regenerate the IMBE FEC
	unsigned int errors = 0U;
	errors += m_fec.regenerateIMBE(data + 0U);
	errors += m_fec.regenerateIMBE(data + 18U);
	errors += m_fec.regenerateIMBE(data + 36U);
	errors += m_fec.regenerateIMBE(data + 54U);
	errors += m_fec.regenerateIMBE(data + 72U);

	return errors;
}

bool CYSFPayload::processVoiceFRModeData(uint8_t* data)
{
	assert(data != nullptr);

	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	uint8_t dch[45U];
	::memcpy(dch, data, 45U);

	CYSFConvolution conv;
	conv.start();

	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];
		uint8_t s0 = READ_BIT1(dch, n) ? 1U : 0U;

		n++;
		uint8_t s1 = READ_BIT1(dch, n) ? 1U : 0U;

		conv.decode(s0, s1);
	}

	uint8_t output[23U];
	conv.chainback(output, 176U);

	bool ret = CCRC::checkCCITT162(output, 22U);
	if (ret) {
		CCRC::addCCITT162(output, 22U);
		output[22U] = 0x00U;

		uint8_t convolved[45U];
		conv.encode(output, convolved, 180U);

		uint8_t bytes[45U];
		unsigned int j = 0U;
		for (unsigned int i = 0U; i < 180U; i++) {
			unsigned int n = INTERLEAVE_TABLE_9_20[i];

			bool s0 = READ_BIT1(convolved, j) != 0U;
			j++;

			bool s1 = READ_BIT1(convolved, j) != 0U;
			j++;

			WRITE_BIT1(bytes, n, s0);

			n++;
			WRITE_BIT1(bytes, n, s1);
		}

		::memcpy(data, bytes, 45U);
	}

	return ret;
}

#endif
