/*
 *   Copyright (C) 2024,2026 by Jonathan Naylor G4KLX
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

#if !defined(MMDVM_CrossMode_H)
#define	MMDVM_CrossMode_H

#include "MetaData.h"
#include "Network.h"
#include "Defines.h"
#include "Conf.h"

#include <string>
#include <map>

class CMMDVMCrossMode {
public:
	CMMDVMCrossMode(const std::string& fileName);
	~CMMDVMCrossMode();

	int run();

private:
	CConf                          m_conf;
	std::map<DATA_MODE, INetwork*> m_rfNetworks;
	std::map<DATA_MODE, INetwork*> m_netNetworks;

	bool createRFNetworks();
	bool createNetNetworks();
	void setThroughModes(CMetaData& data);
	
	DATA_MODE hasRFNetworkGotData() const;
	DATA_MODE hasNetNetworkGotData() const;
	bool readRFNetwork(DATA_MODE mode, CMetaData& data);
	bool readNetNetwork(DATA_MODE mode, CMetaData& data);
	bool writeRFNetworkData(DATA_MODE mode, CMetaData& data);
	bool writeNetNetworkData(DATA_MODE mode, CMetaData& data);
	bool writeRFNetworkRaw(DATA_MODE mode, CMetaData& data);
	bool writeNetNetworkRaw(DATA_MODE mode, CMetaData& data);
	void drainRFNetworks();
	void drainNetNetworks();
	void resetRFNetworks();
	void resetNetNetworks();
	void clockRFNetworks(unsigned int ms);
	void clockNetNetworks(unsigned int ms);
	void closeRFNetworks();
	void closeNetNetworks();

	bool loadIdLookupTables(CMetaData& data);
	void loadModeTranslationTables(CMetaData& data);

	void writeJSONMessage(const std::string& message);
};

#endif
