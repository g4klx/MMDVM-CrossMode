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

#include "CrossMode.h"

#include "DStarNetwork.h"
#include "YSFNetwork.h"
#include "DMRNetwork.h"
#include "M17Network.h"
#include "FMNetwork.h"
#include "StopWatch.h"
#include "Version.h"
#include "Defines.h"
#include "Thread.h"
#include "Timer.h"
#include "Log.h"
#include "GitVersion.h"

#include <cstring>
#include <cassert>

#include <vector>

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
const char* DEFAULT_INI_FILE = "CrossMode.ini";
#else
const char* DEFAULT_INI_FILE = "/etc/CrossMode.ini";
#endif

static bool m_killed = false;
static int  m_signal = 0;
static bool m_reload = false;

#if !defined(_WIN32) && !defined(_WIN64)
static void sigHandler1(int signum)
{
	m_killed = true;
	m_signal = signum;
}

static void sigHandler2(int signum)
{
	m_reload = true;
}
#endif

int main(int argc, char** argv)
{
	std::string iniFile = DEFAULT_INI_FILE;

	if ((argc == 2) && ((::strcmp(argv[1U], "-v") == 0) || (::strcmp(argv[1U], "--version") == 0))) {
		::fprintf(stdout, "CrossMode version %s git #%.7s\n", VERSION, gitversion);
		return 0;
	}

	if (argc == 2) {
		iniFile  = std::string(argv[1U]);
	} else {
		::fprintf(stderr, "Usage: CrossMode [-v|--version] [filename]\n");
		return 1;
	}

#if !defined(_WIN32) && !defined(_WIN64)
	::signal(SIGINT, sigHandler1);
	::signal(SIGTERM, sigHandler1);
	::signal(SIGHUP, sigHandler1);
	::signal(SIGUSR1, sigHandler2);
#endif

	int ret = 0;

	do {
		m_signal = 0;

		CCrossMode* host = new CCrossMode(iniFile);
		ret = host->run();

		delete host;

		switch (m_signal) {
		case 2:
			::LogInfo("CrossMode-%s exited on receipt of SIGINT", VERSION);
			break;
		case 15:
			::LogInfo("CrossMode-%s exited on receipt of SIGTERM", VERSION);
			break;
		case 1:
			::LogInfo("CrossMode-%s exited on receipt of SIGHUP", VERSION);
			break;
		case 10:
			::LogInfo("CrossMode-%s is restarting on receipt of SIGUSR1", VERSION);
			break;
		default:
			::LogInfo("CrossMode-%s exited on receipt of an unknown signal", VERSION);
			break;
		}
	} while (m_signal == 10);

	::LogFinalise();

	return ret;
}

CCrossMode::CCrossMode(const std::string& fileName) :
m_conf(fileName),
m_fromNetwork(nullptr),
m_toNetworks()
{
	assert(!fileName.empty());
}

CCrossMode::~CCrossMode()
{
}

int CCrossMode::run()
{
	bool ret = m_conf.read();
	if (!ret) {
		::fprintf(stderr, "CrossMode: cannot read the .ini file\n");
		return 1;
	}

#if !defined(_WIN32) && !defined(_WIN64)
	bool m_daemon = m_conf.getDaemon();
	if (m_daemon) {
		// Create new process
		pid_t pid = ::fork();
		if (pid == -1) {
			::fprintf(stderr, "Couldn't fork() , exiting\n");
			return -1;
		} else if (pid != 0) {
			exit(EXIT_SUCCESS);
		}

		// Create new session and process group
		if (::setsid() == -1) {
			::fprintf(stderr, "Couldn't setsid(), exiting\n");
			return -1;
		}

		// Set the working directory to the root directory
		if (::chdir("/") == -1) {
			::fprintf(stderr, "Couldn't cd /, exiting\n");
			return -1;
		}

		// If we are currently root...
		if (getuid() == 0) {
			struct passwd* user = ::getpwnam("mmdvm");
			if (user == nullptr) {
				::fprintf(stderr, "Could not get the mmdvm user, exiting\n");
				return -1;
			}

			uid_t mmdvm_uid = user->pw_uid;
			gid_t mmdvm_gid = user->pw_gid;

			// Set user and group ID's to mmdvm:mmdvm
			if (::setgid(mmdvm_gid) != 0) {
				::fprintf(stderr, "Could not set mmdvm GID, exiting\n");
				return -1;
			}

			if (::setuid(mmdvm_uid) != 0) {
				::fprintf(stderr, "Could not set mmdvm UID, exiting\n");
				return -1;
			}

			// Double check it worked (AKA Paranoia)
			if (::setuid(0) != -1) {
				::fprintf(stderr, "It's possible to regain root - something is wrong!, exiting\n");
				return -1;
			}
		}
	}
#endif

#if !defined(_WIN32) && !defined(_WIN64)
	ret = ::LogInitialise(m_daemon, m_conf.getLogFilePath(), m_conf.getLogFileRoot(), m_conf.getLogFileLevel(), m_conf.getLogDisplayLevel(), m_conf.getLogFileRotate());
#else
	ret = ::LogInitialise(false, m_conf.getLogFilePath(), m_conf.getLogFileRoot(), m_conf.getLogFileLevel(), m_conf.getLogDisplayLevel(), m_conf.getLogFileRotate());
#endif
	if (!ret) {
		::fprintf(stderr, "CrossMode: unable to open the log file\n");
		return 1;
	}

#if !defined(_WIN32) && !defined(_WIN64)
	if (m_daemon) {
		::close(STDIN_FILENO);
		::close(STDOUT_FILENO);
	}
#endif

	DATA_MODE fromMode = getFromMode();
	if (fromMode == DATA_MODE_NONE) {
		LogError("Unknown FromMode entered - \"%s\"", m_conf.getFromMode().c_str());
		return 1;
	}

	CUDPSocket::startup();

	ret = createFromNetwork(fromMode);
	if (!ret)
		return 1;

	CData data(m_conf.getTranscoderPort(), m_conf.getTranscoderSpeed(), m_conf.getTranscoderDebug(), m_conf.getCallsign(), m_conf.getDMRId(), m_conf.getNXDNId());
	ret = data.open();
	if (!ret) {
		m_fromNetwork->close();
		delete m_fromNetwork;
		closeToNetworks();
		return 1;
	}

	data.setFromMode(fromMode);
	data.setDirection(DIR_NONE);

	ret = createToNetworks(fromMode, data);
	if (!ret) {
		m_fromNetwork->close();
		delete m_fromNetwork;
		return 1;
	}

	loadModeTranslationTables(fromMode, data);

	ret = loadIdLookupTables(data);
	if (!ret) {
		closeToNetworks();
		m_fromNetwork->close();
		delete m_fromNetwork;
		return 1;
	}

	CStopWatch stopwatch;
	CTimer watchdog(1000U, 0U, 500U);

	DIRECTION direction = DIR_NONE;
	DATA_MODE toMode = DATA_MODE_NONE;

	CTimer rfTimer(1000U, m_conf.getRFModeHang());
	CTimer netTimer(1000U, m_conf.getNetModeHang());

	while (!m_killed) {
		stopwatch.start();

		switch (direction) {
		case DIR_FROM_TO:
			ret = m_fromNetwork->read(data);
			if (ret) {
				rfTimer.start();
				watchdog.start();
			}

			if (data.isTranscode()) {
				if (data.hasData() || data.isEnd())
					writeToNetworkData(toMode, data);
			} else {
				if (data.hasRaw())
					writeToNetworkRaw(toMode, data);
			}

			break;

		case DIR_TO_FROM:
			ret = readToNetwork(toMode, data);
			if (ret) {
				netTimer.start();
				watchdog.start();
			}

			if (data.isTranscode()) {
				if (data.hasData() || data.isEnd())
					m_fromNetwork->writeData(data);
			} else {
				if (data.hasRaw())
					m_fromNetwork->writeRaw(data);
			}

			break;

		default:
			ret = m_fromNetwork->hasData();
			if (ret) {
				::LogMessage("Swicthed by RF activity");
				rfTimer.start();
				watchdog.start();
				toMode = data.getToMode();
				direction = DIR_FROM_TO;
				data.setDirection(DIR_FROM_TO);
				break;
			}

			toMode = hasToNetworkGotData();
			if (toMode != DATA_MODE_NONE) {
				::LogMessage("Swicthed by Net activity");
				netTimer.start();
				watchdog.start();
				direction = DIR_TO_FROM;
				data.setToMode(toMode);
				data.setDirection(DIR_TO_FROM);
				break;
			}

			break;
		}

		if (data.isEnd()) {
			m_fromNetwork->reset();
			resetToNetworks();
			data.reset();
			direction = DIR_NONE;
			data.setDirection(DIR_NONE);
			watchdog.stop();
			netTimer.stop();
			rfTimer.stop();
		}

		CThread::sleep(5U);

		unsigned int elapsed = stopwatch.elapsed();

		m_fromNetwork->clock(elapsed);
		clockToNetworks(elapsed);
		data.clock(elapsed);

		rfTimer.clock(elapsed);
		if (rfTimer.isRunning() && rfTimer.hasExpired()) {
			::LogMessage("Swictched back to Idle");
			m_fromNetwork->reset();
			resetToNetworks();
			data.reset();
			direction = DIR_NONE;
			data.setDirection(DIR_NONE);
			rfTimer.stop();
		}

		netTimer.clock(elapsed);
		if (netTimer.isRunning() && netTimer.hasExpired()) {
			::LogMessage("Swictched back to Idle");
			m_fromNetwork->reset();
			resetToNetworks();
			data.reset();
			direction = DIR_NONE;
			data.setDirection(DIR_NONE);
			netTimer.stop();
		}

		watchdog.clock(elapsed);
		if (watchdog.isRunning() && watchdog.hasExpired()) {
			::LogMessage("The watchdog timer has exprired");
			m_fromNetwork->reset();
			resetToNetworks();
			data.reset();
			direction = DIR_NONE;
			data.setDirection(DIR_NONE);
			watchdog.stop();
		}
	}

	data.close();

	m_fromNetwork->close();
	delete m_fromNetwork;

	closeToNetworks();

	CUDPSocket::shutdown();

	return 0;
}

bool CCrossMode::createFromNetwork(DATA_MODE mode)
{
	std::string callsign = m_conf.getCallsign();

	if (mode == DATA_MODE_DSTAR) {
		std::string dstarCallsign = callsign + "        ";
		dstarCallsign = dstarCallsign.substr(0U, DSTAR_LONG_CALLSIGN_LENGTH - 1U) + m_conf.getDStarModule();
		dstarCallsign = dstarCallsign.substr(0U, DSTAR_LONG_CALLSIGN_LENGTH);

		std::string remoteAddress = m_conf.getDStarFromRemoteAddress();
		std::string localAddress  = m_conf.getDStarFromLocalAddress();
		uint16_t remotePort       = m_conf.getDStarFromRemotePort();
		uint16_t localPort        = m_conf.getDStarFromLocalPort();
		bool debug                = m_conf.getDStarFromDebug();
		m_fromNetwork = new CDStarNetwork(NET_FROM, dstarCallsign, localAddress, localPort, remoteAddress, remotePort, debug);
	} else if (mode == DATA_MODE_DMR) {
		uint32_t id = m_conf.getDMRId();

		std::string remoteAddress = m_conf.getDMRFromRemoteAddress();
		std::string localAddress  = m_conf.getDMRFromLocalAddress();
		uint16_t remotePort       = m_conf.getDMRFromRemotePort();
		uint16_t localPort        = m_conf.getDMRFromLocalPort();
		bool debug                = m_conf.getDMRFromDebug();
		m_fromNetwork = new CDMRNetwork(NET_FROM, id, localAddress, localPort, remoteAddress, remotePort, debug);
	} else if (mode == DATA_MODE_YSF) {
		std::string remoteAddress = m_conf.getYSFFromRemoteAddress();
		std::string localAddress  = m_conf.getYSFFromLocalAddress();
		uint16_t remotePort       = m_conf.getYSFFromRemotePort();
		uint16_t localPort        = m_conf.getYSFFromLocalPort();
		bool debug                = m_conf.getYSFFromDebug();
		m_fromNetwork = new CYSFNetwork(NET_FROM, callsign, localAddress, localPort, remoteAddress, remotePort, debug);
	} else if (mode == DATA_MODE_FM) {
		std::string remoteAddress = m_conf.getFMFromRemoteAddress();
		std::string localAddress  = m_conf.getFMFromLocalAddress();
		uint16_t remotePort       = m_conf.getFMFromRemotePort();
		uint16_t localPort        = m_conf.getFMFromLocalPort();
		bool debug                = m_conf.getFMFromDebug();
		m_fromNetwork = new CFMNetwork(NET_FROM, callsign, localAddress, localPort, remoteAddress, remotePort, debug);
	} else if (mode == DATA_MODE_M17) {
		std::string remoteAddress = m_conf.getM17FromRemoteAddress();
		std::string localAddress  = m_conf.getM17FromLocalAddress();
		uint16_t remotePort       = m_conf.getM17FromRemotePort();
		uint16_t localPort        = m_conf.getM17FromLocalPort();
		bool debug                = m_conf.getM17FromDebug();
		m_fromNetwork = new CM17Network(NET_FROM, localAddress, localPort, remoteAddress, remotePort, debug);
	} else {
		::LogError("Unknown from mode specified");
		return false;
	}

	bool ret = m_fromNetwork->open();
	if (!ret) {
		::LogError("Unable to open the From network interface");
		delete m_fromNetwork;
		return false;
	}

	return true;
}

bool CCrossMode::createToNetworks(DATA_MODE fromMode, CData& data)
{
	closeToNetworks();

	std::string callsign = m_conf.getCallsign();

	bool toDStar = false;
	bool toDMR   = false;
	bool toYSF   = false;
	bool toP25   = false;
	bool toNXDN  = false;
	bool toFM    = false;
	bool toM17   = false;

	if (((fromMode == DATA_MODE_DSTAR) && m_conf.getDStarDStarEnable()) ||
		((fromMode == DATA_MODE_DMR)   && m_conf.getDMRDStarEnable())   ||
		((fromMode == DATA_MODE_YSF)   && m_conf.getYSFDStarEnable())   ||
		((fromMode == DATA_MODE_P25)   && m_conf.getP25DStarEnable())   ||
		((fromMode == DATA_MODE_NXDN)  && m_conf.getNXDNDStarEnable())  ||
		((fromMode == DATA_MODE_FM)    && m_conf.getFMDStarEnable())    ||
		((fromMode == DATA_MODE_M17)   && m_conf.getM17DStarEnable())) {
		toDStar = true;
	}

	if (toDStar) {
		std::string dstarCallsign = callsign + "        ";
		dstarCallsign = dstarCallsign.substr(0U, DSTAR_LONG_CALLSIGN_LENGTH - 1U) + m_conf.getDStarModule();
		dstarCallsign = dstarCallsign.substr(0U, DSTAR_LONG_CALLSIGN_LENGTH);

		std::string remoteAddress = m_conf.getDStarToRemoteAddress();
		std::string localAddress  = m_conf.getDStarToLocalAddress();
		uint16_t remotePort       = m_conf.getDStarToRemotePort();
		uint16_t localPort        = m_conf.getDStarToLocalPort();
		bool debug                = m_conf.getDStarToDebug();

		CDStarNetwork* network = new CDStarNetwork(NET_TO, dstarCallsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the D-Star To network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE_DSTAR, network));
	}

	if (((fromMode == DATA_MODE_DSTAR) && m_conf.getDStarDMREnable()) ||
		((fromMode == DATA_MODE_DMR)   && m_conf.getDMRDMREnable1())  ||
		((fromMode == DATA_MODE_DMR)   && m_conf.getDMRDMREnable2())  ||
		((fromMode == DATA_MODE_YSF)   && m_conf.getYSFDMREnable())   ||
		((fromMode == DATA_MODE_P25)   && m_conf.getP25DMREnable())   ||
		((fromMode == DATA_MODE_NXDN)  && m_conf.getNXDNDMREnable())  ||
		((fromMode == DATA_MODE_FM)    && m_conf.getFMDMREnable())    ||
		((fromMode == DATA_MODE_M17)   && m_conf.getM17DMREnable())) {
		toDMR = true;
	}

	if (toDMR) {
		uint32_t id = m_conf.getDMRId();

		std::string remoteAddress = m_conf.getDMRToRemoteAddress();
		std::string localAddress  = m_conf.getDMRToLocalAddress();
		uint16_t remotePort       = m_conf.getDMRToRemotePort();
		uint16_t localPort        = m_conf.getDMRToLocalPort();
		bool debug                = m_conf.getDMRToDebug();

		CDMRNetwork* network = new CDMRNetwork(NET_TO, id, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the DMR To network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE_DMR, network));
	}

	if (((fromMode == DATA_MODE_DSTAR) && m_conf.getDStarYSFEnable()) ||
		((fromMode == DATA_MODE_DMR)   && m_conf.getDMRYSFEnable())   ||
		((fromMode == DATA_MODE_YSF)   && m_conf.getYSFYSFEnable())   ||
		((fromMode == DATA_MODE_P25)   && m_conf.getP25YSFEnable())   ||
		((fromMode == DATA_MODE_NXDN)  && m_conf.getNXDNYSFEnable())  ||
		((fromMode == DATA_MODE_FM)    && m_conf.getFMYSFEnable())    ||
		((fromMode == DATA_MODE_M17)   && m_conf.getM17YSFEnable())) {
		toYSF = true;
	}

	if (toYSF) {
		std::string remoteAddress = m_conf.getYSFToRemoteAddress();
		std::string localAddress  = m_conf.getYSFToLocalAddress();
		uint16_t remotePort       = m_conf.getYSFToRemotePort();
		uint16_t localPort        = m_conf.getYSFToLocalPort();
		bool debug                = m_conf.getYSFToDebug();

		CYSFNetwork* network = new CYSFNetwork(NET_TO, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the System Fusion To network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE_YSF, network));
	}

	if (((fromMode == DATA_MODE_DSTAR) && m_conf.getDStarFMEnable()) ||
		((fromMode == DATA_MODE_DMR)   && m_conf.getDMRFMEnable())   ||
		((fromMode == DATA_MODE_YSF)   && m_conf.getYSFFMEnable())   ||
		((fromMode == DATA_MODE_P25)   && m_conf.getP25FMEnable())   ||
		((fromMode == DATA_MODE_NXDN)  && m_conf.getNXDNFMEnable())  ||
		((fromMode == DATA_MODE_FM)    && m_conf.getFMFMEnable())    ||
		((fromMode == DATA_MODE_M17)   && m_conf.getM17FMEnable())) {
		toFM = true;
	}

	if (toFM) {
		std::string remoteAddress = m_conf.getFMToRemoteAddress();
		std::string localAddress  = m_conf.getFMToLocalAddress();
		uint16_t remotePort       = m_conf.getFMToRemotePort();
		uint16_t localPort        = m_conf.getFMToLocalPort();
		bool debug                = m_conf.getFMToDebug();

		CFMNetwork* network = new CFMNetwork(NET_TO, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the FM To network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE_FM, network));
	}

	if (((fromMode == DATA_MODE_DSTAR) && m_conf.getDStarM17Enable()) ||
		((fromMode == DATA_MODE_DMR)   && m_conf.getDMRM17Enable())   ||
		((fromMode == DATA_MODE_YSF)   && m_conf.getYSFM17Enable())   ||
		((fromMode == DATA_MODE_P25)   && m_conf.getP25M17Enable())   ||
		((fromMode == DATA_MODE_NXDN)  && m_conf.getNXDNM17Enable())  ||
		((fromMode == DATA_MODE_FM)    && m_conf.getFMM17Enable())    ||
		((fromMode == DATA_MODE_M17)   && m_conf.getM17M17Enable())) {
		toM17 = true;
	}

	if (toM17) {
		std::string remoteAddress = m_conf.getM17ToRemoteAddress();
		std::string localAddress  = m_conf.getM17ToLocalAddress();
		uint16_t remotePort       = m_conf.getM17ToRemotePort();
		uint16_t localPort        = m_conf.getM17ToLocalPort();
		bool debug                = m_conf.getM17ToDebug();

		CM17Network* network = new CM17Network(NET_TO, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the M17 To network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE_M17, network));
	}

	data.setToModes(m_conf.getDStarDStarEnable(),
					m_conf.getDMRDMREnable1(),
					m_conf.getDMRDMREnable2(),
					m_conf.getYSFYSFEnable(),
					m_conf.getP25P25Enable(),
					m_conf.getNXDNNXDNEnable(),
					m_conf.getFMFMEnable(),
					m_conf.getM17M17Enable());

	return true;
}

DATA_MODE CCrossMode::hasToNetworkGotData() const
{
	for (const auto& it : m_toNetworks) {
		bool ret = it.second->hasData();
		if (ret)
			return it.first;
	}

	return DATA_MODE_NONE;
}

bool CCrossMode::readToNetwork(DATA_MODE mode, CData& data)
{
	bool ret = false;

	for (const auto& it : m_toNetworks) {
		if (mode == it.first)
			ret = it.second->read(data);
		else
			it.second->read();
	}

	return ret;
}

bool CCrossMode::writeToNetworkData(DATA_MODE mode, CData& data)
{
	const auto& it = m_toNetworks.find(mode);
	if (it == m_toNetworks.end())
		return false;

	return it->second->writeData(data);
}

bool CCrossMode::writeToNetworkRaw(DATA_MODE mode, CData& data)
{
	const auto& it = m_toNetworks.find(mode);
	if (it == m_toNetworks.end())
		return false;

	return it->second->writeRaw(data);
}

void CCrossMode::resetToNetworks()
{
	for (auto& it : m_toNetworks)
		it.second->reset();
}

void CCrossMode::clockToNetworks(unsigned int ms)
{
	for (auto& it : m_toNetworks)
		it.second->clock(ms);
}

void CCrossMode::closeToNetworks()
{
	for (auto& it : m_toNetworks) {
		it.second->close();
		delete it.second;
	}

	m_toNetworks.clear();
}

DATA_MODE CCrossMode::getFromMode() const
{
	std::string mode = m_conf.getFromMode();

	if (mode == "D-Star")
		return DATA_MODE_DSTAR;
	else if (mode == "DMR")
		return DATA_MODE_DMR;
	else if (mode == "System Fusion")
		return DATA_MODE_YSF;
	else if (mode == "P25")
		return DATA_MODE_P25;
	else if (mode == "NXDN")
		return DATA_MODE_NXDN;
	else if (mode == "FM")
		return DATA_MODE_FM;
	else if (mode == "M17")
		return DATA_MODE_M17;
	else
		return DATA_MODE_NONE;
}

void CCrossMode::loadModeTranslationTables(DATA_MODE fromMode, CData& data)
{
	if (fromMode == DATA_MODE_DSTAR) {
		if (m_conf.getDStarDMREnable())
			data.setDStarDMRDests(m_conf.getDStarDMRDests());
		if (m_conf.getDStarYSFEnable())
			data.setDStarYSFDests(m_conf.getDStarYSFDests());
		if (m_conf.getDStarP25Enable())
			data.setDStarP25Dests(m_conf.getDStarP25Dests());
		if (m_conf.getDStarNXDNEnable())
			data.setDStarNXDNDests(m_conf.getDStarNXDNDests());
		if (m_conf.getDStarFMEnable())
			data.setDStarFMDest(m_conf.getDStarFMDest());
		if (m_conf.getDStarM17Enable())
			data.setDStarM17Dests(m_conf.getDStarM17Dests());
	}

	if (fromMode == DATA_MODE_DMR) {
		if (m_conf.getDMRDStarEnable())
			data.setDMRDStarTGs(m_conf.getDMRDStarTGs());
		if (m_conf.getDMRYSFEnable())
			data.setDMRYSFTGs(m_conf.getDMRYSFTGs());
		if (m_conf.getDMRP25Enable())
			data.setDMRP25TGs(m_conf.getDMRP25TGs());
		if (m_conf.getDMRNXDNEnable())
			data.setDMRNXDNTGs(m_conf.getDMRNXDNTGs());
		if (m_conf.getDMRFMEnable())
			data.setDMRFMTG(m_conf.getDMRFMTG());
		if (m_conf.getDMRM17Enable())
			data.setDMRM17TGs(m_conf.getDMRM17TGs());
	}

	if (fromMode == DATA_MODE_YSF) {
		if (m_conf.getYSFDStarEnable())
			data.setYSFDStarDGIds(m_conf.getYSFDStarDGIds());
		if (m_conf.getYSFDMREnable())
			data.setYSFDMRDGIds(m_conf.getYSFDMRDGIds());
		if (m_conf.getYSFP25Enable())
			data.setYSFP25DGIds(m_conf.getYSFP25DGIds());
		if (m_conf.getYSFNXDNEnable())
			data.setYSFNXDNDGIds(m_conf.getYSFNXDNDGIds());
		if (m_conf.getYSFFMEnable())
			data.setYSFFMDGId(m_conf.getYSFFMDGId());
		if (m_conf.getYSFM17Enable())
			data.setYSFM17DGIds(m_conf.getYSFM17DGIds());
	}

	if (fromMode == DATA_MODE_M17) {
		if (m_conf.getM17DStarEnable())
			data.setM17DStarDests(m_conf.getM17DStarDests());
		if (m_conf.getM17FMEnable())
			data.setM17FMDest(m_conf.getM17FMDest());
	}
}

bool CCrossMode::loadIdLookupTables(CData& data)
{
	std::string dmrFileName  = m_conf.getDMRLookupFile();
	std::string nxdnFileName = m_conf.getNXDNLookupFile();
	unsigned int reloadTime  = m_conf.getReloadTime();

	bool ret = data.setDMRLookup(dmrFileName, reloadTime);
	if (!ret)
		return false;

	return data.setNXDNLookup(nxdnFileName, reloadTime);
}
