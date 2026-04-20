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
	std::map<DATA_MODE, INetwork*> m_fromNetworks;
	std::map<DATA_MODE, INetwork*> m_toNetworks;

	bool createFromNetworks();
	bool createToNetworks();
	void setThroughModes(CMetaData& data);
	
	DATA_MODE hasToNetworkGotData() const;
	DATA_MODE hasFromNetworkGotData() const;
	bool readFromNetwork(DATA_MODE mode, CMetaData& data);
	bool readToNetwork(DATA_MODE mode, CMetaData& data);
	bool writeFromNetworkData(DATA_MODE mode, CMetaData& data);
	bool writeToNetworkData(DATA_MODE mode, CMetaData& data);
	bool writeFromNetworkRaw(DATA_MODE mode, CMetaData& data);
	bool writeToNetworkRaw(DATA_MODE mode, CMetaData& data);
	void drainFromNetworks();
	void drainToNetworks();
	void resetFromNetworks();
	void resetToNetworks();
	void clockFromNetworks(unsigned int ms);
	void clockToNetworks(unsigned int ms);
	void closeFromNetworks();
	void closeToNetworks();

	bool loadIdLookupTables(CMetaData& data);
	void loadModeTranslationTables(CMetaData& data);

	void writeJSONMessage(const std::string& message);
};

#endif
