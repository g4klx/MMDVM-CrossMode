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

#if !defined(CrossMode_H)
#define	CrossMode_H

#include "Network.h"
#include "Defines.h"
#include "Data.h"
#include "Conf.h"

#include <string>
#include <map>

class CCrossMode {
public:
	CCrossMode(const std::string& fileName);
	~CCrossMode();

	int run();

private:
	CConf     m_conf;
	INetwork* m_fromNetwork;
	std::map<DATA_MODE, INetwork*> m_toNetworks;

	DATA_MODE getFromMode() const;

	bool createFromNetwork(DATA_MODE mode);

	bool createToNetworks(DATA_MODE fromMode, CData& data);
	DATA_MODE hasToNetworkGotData() const;
	bool readToNetwork(DATA_MODE mode, CData& data);
	bool writeToNetworkData(DATA_MODE mode, CData& data);
	bool writeToNetworkRaw(DATA_MODE mode, CData& data);
	void resetToNetworks();
	void clockToNetworks(unsigned int ms);
	void closeToNetworks();
};

#endif
