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

#include "MMDVM-CrossMode.h"

#include "MQTTConnection.h"
#include "DStarNetwork.h"
#include "NXDNNetwork.h"
#include "P25Network.h"
#include "YSFNetwork.h"
#include "DMRNetwork.h"
#include "FMNetwork.h"
#include "StopWatch.h"
#include "Version.h"
#include "Defines.h"
#include "Thread.h"
#include "Timer.h"
#include "Utils.h"
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
const char* DEFAULT_INI_FILE = "MMDVM-CrossMode.ini";
#else
const char* DEFAULT_INI_FILE = "/etc/MMDVM-CrossMode.ini";
#endif

static bool m_killed = false;
static int  m_signal = 0;
static bool m_reload = false;

// In Log.cpp
extern CMQTTConnection* m_mqtt;

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

		CMMDVMCrossMode* host = new CMMDVMCrossMode(iniFile);
		ret = host->run();

		delete host;

		switch (m_signal) {
		case 2:
			::LogInfo("MMDVM-CrossMode-%s exited on receipt of SIGINT", VERSION);
			break;
		case 15:
			::LogInfo("MMDVM-CrossMode-%s exited on receipt of SIGTERM", VERSION);
			break;
		case 1:
			::LogInfo("MMDVM-CrossMode-%s exited on receipt of SIGHUP", VERSION);
			break;
		case 10:
			::LogInfo("MMDVM-CrossMode-%s is restarting on receipt of SIGUSR1", VERSION);
			break;
		default:
			::LogInfo("MMDVM-CrossMode-%s exited on receipt of an unknown signal", VERSION);
			break;
		}
	} while (m_signal == 10);

	::LogFinalise();

	return ret;
}

CMMDVMCrossMode::CMMDVMCrossMode(const std::string& fileName) :
m_conf(fileName),
m_fromNetworks(),
m_toNetworks()
{
	assert(!fileName.empty());
}

CMMDVMCrossMode::~CMMDVMCrossMode()
{
}

int CMMDVMCrossMode::run()
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

	::LogInitialise(m_conf.getLogDisplayLevel(), m_conf.getLogMQTTLevel());

	std::vector<std::pair<std::string, void (*)(const unsigned char*, unsigned int)>> subscriptions;

	m_mqtt = new CMQTTConnection(m_conf.getMQTTAddress(), m_conf.getMQTTPort(), m_conf.getMQTTName(), m_conf.getMQTTAuthEnabled(), m_conf.getMQTTUsername(), m_conf.getMQTTPassword(), subscriptions, m_conf.getMQTTKeepalive());
	ret = m_mqtt->open();
	if (!ret)
		return 1; 

#if !defined(_WIN32) && !defined(_WIN64)
	if (m_daemon) {
		::close(STDIN_FILENO);
		::close(STDOUT_FILENO);
	}
#endif

	CUDPSocket::startup();

	CMetaData data(m_conf.getCallsign(), m_conf.getDMRId(), m_conf.getNXDNId(), m_conf.getTranscoderDebug());

	std::string protocol = m_conf.getTranscoderProtocol();
	if (protocol == "uart") {
		data.setUARTConnection(m_conf.getTranscoderUARTPort(), m_conf.getTranscoderUARTSpeed());
	} else if (protocol == "udp") {
		data.setUDPConnection(m_conf.getTranscoderRemoteAddress(), m_conf.getTranscoderRemotePort(),
			m_conf.getTranscoderLocalAddress(), m_conf.getTranscoderLocalPort());
	} else {
		LogError("Unknown transcoder connection protocol - %s", protocol.c_str());
		return 1;
	}

	ret = data.open();
	if (!ret)
		return 1;

	DIRECTION direction = DIRECTION::NONE;
	DATA_MODE fromMode  = DATA_MODE::NONE;
	DATA_MODE toMode    = DATA_MODE::NONE;

	data.setFromMode(fromMode);
	data.setToMode(toMode);
	data.setDirection(direction);

	ret = createFromNetworks();
	if (!ret)
		return 1;

	ret = createToNetworks();
	if (!ret) {
		closeFromNetworks();
		return 1;
	}

	setThroughModes(data);

	loadModeTranslationTables(data);

	ret = loadIdLookupTables(data);
	if (!ret) {
		closeFromNetworks();
		closeToNetworks();
		return 1;
	}

	CStopWatch stopwatch;
	CTimer watchdog(1000U, 1U);

	CTimer rfTimer(1000U, m_conf.getRFModeHang());
	CTimer netTimer(1000U, m_conf.getNetModeHang());

	LogMessage("MMDVM-CrossMode-%s is starting", VERSION);
	LogMessage("Built %s %s (GitID #%.7s)", __TIME__, __DATE__, gitversion);

	writeJSONMessage("MMDVM-CrossMode is starting");

	stopwatch.start();

	while (!m_killed) {
		switch (direction) {
		case DIRECTION::FROM_TO:
			ret = readFromNetwork(fromMode, data);
			if (ret) {
				netTimer.stop();
				rfTimer.start();
				watchdog.start();
			}

			drainToNetworks();

			if (data.isTranscode()) {
				if (data.hasData() || data.isEnd())
					writeToNetworkData(toMode, data);
			} else {
				if (data.hasRaw())
					writeToNetworkRaw(toMode, data);
			}

			break;

		case DIRECTION::TO_FROM:
			ret = readToNetwork(toMode, data);
			if (ret) {
				rfTimer.stop();
				netTimer.start();
				watchdog.start();
			}

			drainFromNetworks();

			if (data.isTranscode()) {
				if (data.hasData() || data.isEnd())
					writeFromNetworkData(fromMode, data);
			} else {
				if (data.hasRaw())
					writeFromNetworkRaw(fromMode, data);
			}

			break;

		default:
			fromMode = hasFromNetworkGotData();
			if (fromMode != DATA_MODE::NONE) {
				direction = DIRECTION::FROM_TO;
				data.setDirection(direction);
				data.setFromMode(fromMode);
				netTimer.stop();
				rfTimer.start();
				watchdog.start();
				::LogMessage("Switched by RF activity from mode %s", CUtils::getModeName(fromMode).c_str());
				break;
			}

			toMode = hasToNetworkGotData();
			if (toMode != DATA_MODE::NONE) {
				direction = DIRECTION::TO_FROM;
				data.setDirection(direction);
				data.setToMode(toMode);
				rfTimer.stop();
				netTimer.start();
				watchdog.start();
				::LogMessage("Switched by Net activity to mode %s", CUtils::getModeName(toMode).c_str());
				break;
			}

			break;
		}

		if (data.isEnd()) {
			resetFromNetworks();
			resetToNetworks();
			data.reset();
			direction = DIRECTION::NONE;
			data.setDirection(direction);
			watchdog.stop();
		}

		CThread::sleep(5U);

		unsigned int elapsed = stopwatch.elapsed();
		stopwatch.start();

		clockFromNetworks(elapsed);
		clockToNetworks(elapsed);
		data.clock(elapsed);

		rfTimer.clock(elapsed);
		if (rfTimer.isRunning() && rfTimer.hasExpired()) {
			resetFromNetworks();
			resetToNetworks();
			data.reset();
			direction = DIRECTION::NONE;
			toMode    = DATA_MODE::NONE;
			data.setDirection(direction);
			rfTimer.stop();
			::LogMessage("Switched back to Idle by the RF timer");
		}

		netTimer.clock(elapsed);
		if (netTimer.isRunning() && netTimer.hasExpired()) {
			resetFromNetworks();
			resetToNetworks();
			data.reset();
			direction = DIRECTION::NONE;
			toMode    = DATA_MODE::NONE;
			data.setDirection(direction);
			netTimer.stop();
			::LogMessage("Switched back to Idle by the Net timer");
		}

		watchdog.clock(elapsed);
		if (watchdog.isRunning() && watchdog.hasExpired()) {
			resetFromNetworks();
			resetToNetworks();
			data.reset();
			direction = DIRECTION::NONE;
			data.setDirection(direction);
			watchdog.stop();
			::LogMessage("The watchdog timer has expired");
		}
	}

	LogInfo("MMDVM-CrossMode is stopping");
	writeJSONMessage("MMDVM-CrossMode is stopping");

	data.close();

	closeFromNetworks();

	closeToNetworks();

	CUDPSocket::shutdown();

	return 0;
}

bool CMMDVMCrossMode::createFromNetworks()
{
	closeFromNetworks();
	
	std::string callsign = m_conf.getCallsign();

	bool fromDStar = false;
	bool fromDMR   = false;
	bool fromYSF   = false;
	bool fromP25   = false;
	bool fromNXDN  = false;
	bool fromFM    = false;

	if (m_conf.getDStarDStarEnable() ||
		m_conf.getDStarDMREnable()   ||
		m_conf.getDStarYSFEnable()   ||
		m_conf.getDStarP25Enable()   ||
		m_conf.getDStarNXDNEnable()  ||
		m_conf.getDStarFMEnable()) {
		fromDStar = true;
	}

	if (fromDStar) {
		std::string dstarCallsign = callsign + "        ";
		dstarCallsign = dstarCallsign.substr(0U, DSTAR_LONG_CALLSIGN_LENGTH - 1U) + m_conf.getDStarModule();
		dstarCallsign = dstarCallsign.substr(0U, DSTAR_LONG_CALLSIGN_LENGTH);

		std::string remoteAddress = m_conf.getDStarFromRemoteAddress();
		std::string localAddress  = m_conf.getDStarFromLocalAddress();
		uint16_t remotePort       = m_conf.getDStarFromRemotePort();
		uint16_t localPort        = m_conf.getDStarFromLocalPort();
		bool debug                = m_conf.getDStarFromDebug();

		CDStarNetwork* network = new CDStarNetwork(NETWORK::FROM, dstarCallsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the D-Star FROM network interface");
			closeFromNetworks();
			return false;
		}

		m_fromNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::DSTAR, network));
	}

	if (m_conf.getDMRDStarEnable() ||
		m_conf.getDMRDMREnable1()  ||
		m_conf.getDMRDMREnable2()  ||
		m_conf.getDMRYSFEnable()   ||
		m_conf.getDMRP25Enable()   ||
		m_conf.getDMRNXDNEnable()  ||
		m_conf.getDMRFMEnable()) {
		fromDMR = true;
	}

	if (fromDMR) {
		uint32_t id = m_conf.getDMRId();

		std::string remoteAddress = m_conf.getDMRFromRemoteAddress();
		std::string localAddress  = m_conf.getDMRFromLocalAddress();
		uint16_t remotePort       = m_conf.getDMRFromRemotePort();
		uint16_t localPort        = m_conf.getDMRFromLocalPort();
		bool debug                = m_conf.getDMRFromDebug();

		CDMRNetwork* network = new CDMRNetwork(NETWORK::FROM, id, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the DMR FROM network interface");
			closeFromNetworks();
			return false;
		}

		m_fromNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::DMR, network));
	}

	if (m_conf.getYSFDStarEnable() ||
		m_conf.getYSFDMREnable()   ||
		m_conf.getYSFYSFEnable()   ||
		m_conf.getYSFP25Enable()   ||
		m_conf.getYSFNXDNEnable()  ||
		m_conf.getYSFFMEnable()) {
		fromYSF = true;
	}

	if (fromYSF) {
		std::string remoteAddress = m_conf.getYSFFromRemoteAddress();
		std::string localAddress  = m_conf.getYSFFromLocalAddress();
		uint16_t remotePort       = m_conf.getYSFFromRemotePort();
		uint16_t localPort        = m_conf.getYSFFromLocalPort();
		bool debug                = m_conf.getYSFFromDebug();

		CYSFNetwork* network = new CYSFNetwork(NETWORK::FROM, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the System Fusion FROM network interface");
			closeFromNetworks();
			return false;
		}

		m_fromNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::YSF, network));
	}

	if (m_conf.getP25DStarEnable() ||
		m_conf.getP25DMREnable()   ||
		m_conf.getP25YSFEnable()   ||
		m_conf.getP25P25Enable()   ||
		m_conf.getP25NXDNEnable()  ||
		m_conf.getP25FMEnable()) {
		fromP25 = true;
	}

	if (fromP25) {
		std::string remoteAddress = m_conf.getP25FromRemoteAddress();
		std::string localAddress  = m_conf.getP25FromLocalAddress();
		uint16_t remotePort       = m_conf.getP25FromRemotePort();
		uint16_t localPort        = m_conf.getP25FromLocalPort();
		bool debug                = m_conf.getP25FromDebug();

		CP25Network* network = new CP25Network(NETWORK::FROM, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the P25 FROM network interface");
			closeFromNetworks();
			return false;
		}

		m_fromNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::P25, network));
	}

	if (m_conf.getNXDNDStarEnable() ||
		m_conf.getNXDNDMREnable()   ||
		m_conf.getNXDNYSFEnable()   ||
		m_conf.getNXDNP25Enable()   ||
		m_conf.getNXDNNXDNEnable()  ||
		m_conf.getNXDNFMEnable()) {
		fromNXDN = true;
	}

	if (fromNXDN) {
		std::string remoteAddress = m_conf.getNXDNFromRemoteAddress();
		std::string localAddress  = m_conf.getNXDNFromLocalAddress();
		uint16_t remotePort       = m_conf.getNXDNFromRemotePort();
		uint16_t localPort        = m_conf.getNXDNFromLocalPort();
		bool debug                = m_conf.getNXDNFromDebug();

		CNXDNNetwork* network = new CNXDNNetwork(NETWORK::FROM, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the NXDN FROM network interface");
			closeFromNetworks();
			return false;
		}

		m_fromNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::NXDN, network));
	}

	if (m_conf.getFMDStarEnable() ||
		m_conf.getFMDMREnable()   ||
		m_conf.getFMYSFEnable()   ||
		m_conf.getFMP25Enable()   ||
		m_conf.getFMNXDNEnable()  ||
		m_conf.getFMFMEnable()) {
		fromFM = true;
	}

	if (fromFM) {
		std::string remoteAddress = m_conf.getFMFromRemoteAddress();
		std::string localAddress  = m_conf.getFMFromLocalAddress();
		uint16_t remotePort       = m_conf.getFMFromRemotePort();
		uint16_t localPort        = m_conf.getFMFromLocalPort();
		bool debug                = m_conf.getFMFromDebug();

		CFMNetwork* network = new CFMNetwork(NETWORK::FROM, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the FM FROM network interface");
			closeFromNetworks();
			return false;
		}

		m_fromNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::FM, network));
	}

	return true;
}

bool CMMDVMCrossMode::createToNetworks()
{
	closeToNetworks();

	std::string callsign = m_conf.getCallsign();

	bool toDStar = false;
	bool toDMR   = false;
	bool toYSF   = false;
	bool toP25   = false;
	bool toNXDN  = false;
	bool toFM    = false;

	if (m_conf.getDStarDStarEnable() ||
		m_conf.getDMRDStarEnable()   ||
		m_conf.getYSFDStarEnable()   ||
		m_conf.getP25DStarEnable()   ||
		m_conf.getNXDNDStarEnable()  ||
		m_conf.getFMDStarEnable()) {
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

		CDStarNetwork* network = new CDStarNetwork(NETWORK::TO, dstarCallsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the D-Star TO network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::DSTAR, network));
	}

	if (m_conf.getDStarDMREnable() ||
		m_conf.getDMRDMREnable1()  ||
		m_conf.getDMRDMREnable2()  ||
		m_conf.getYSFDMREnable()   ||
		m_conf.getP25DMREnable()   ||
		m_conf.getNXDNDMREnable()  ||
		m_conf.getFMDMREnable()) {
		toDMR = true;
	}

	if (toDMR) {
		uint32_t id = m_conf.getDMRId();

		std::string remoteAddress = m_conf.getDMRToRemoteAddress();
		std::string localAddress  = m_conf.getDMRToLocalAddress();
		uint16_t remotePort       = m_conf.getDMRToRemotePort();
		uint16_t localPort        = m_conf.getDMRToLocalPort();
		bool debug                = m_conf.getDMRToDebug();

		CDMRNetwork* network = new CDMRNetwork(NETWORK::TO, id, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the DMR TO network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::DMR, network));
	}

	if (m_conf.getDStarYSFEnable() ||
		m_conf.getDMRYSFEnable()   ||
		m_conf.getYSFYSFEnable()   ||
		m_conf.getP25YSFEnable()   ||
		m_conf.getNXDNYSFEnable()  ||
		m_conf.getFMYSFEnable()) {
		toYSF = true;
	}

	if (toYSF) {
		std::string remoteAddress = m_conf.getYSFToRemoteAddress();
		std::string localAddress  = m_conf.getYSFToLocalAddress();
		uint16_t remotePort       = m_conf.getYSFToRemotePort();
		uint16_t localPort        = m_conf.getYSFToLocalPort();
		bool debug                = m_conf.getYSFToDebug();

		CYSFNetwork* network = new CYSFNetwork(NETWORK::TO, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the System Fusion TO network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::YSF, network));
	}

	if (m_conf.getDStarP25Enable() ||
		m_conf.getDMRP25Enable()   ||
		m_conf.getYSFP25Enable()   ||
		m_conf.getP25P25Enable()   ||
		m_conf.getNXDNP25Enable()  ||
		m_conf.getFMP25Enable()) {
		toP25 = true;
	}

	if (toP25) {
		std::string remoteAddress = m_conf.getP25ToRemoteAddress();
		std::string localAddress  = m_conf.getP25ToLocalAddress();
		uint16_t remotePort       = m_conf.getP25ToRemotePort();
		uint16_t localPort        = m_conf.getP25ToLocalPort();
		bool debug                = m_conf.getP25ToDebug();

		CP25Network* network = new CP25Network(NETWORK::TO, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the P25 TO network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::P25, network));
	}

	if (m_conf.getDStarNXDNEnable() ||
		m_conf.getDMRNXDNEnable()   ||
		m_conf.getYSFNXDNEnable()   ||
		m_conf.getP25NXDNEnable()   ||
		m_conf.getNXDNNXDNEnable()  ||
		m_conf.getFMNXDNEnable()) {
		toNXDN = true;
	}

	if (toNXDN) {
		std::string remoteAddress = m_conf.getNXDNToRemoteAddress();
		std::string localAddress  = m_conf.getNXDNToLocalAddress();
		uint16_t remotePort       = m_conf.getNXDNToRemotePort();
		uint16_t localPort        = m_conf.getNXDNToLocalPort();
		bool debug                = m_conf.getNXDNToDebug();

		CNXDNNetwork* network = new CNXDNNetwork(NETWORK::TO, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the NXDN TO network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::NXDN, network));
	}

	if (m_conf.getDStarFMEnable() ||
		m_conf.getDMRFMEnable()   ||
		m_conf.getYSFFMEnable()   ||
		m_conf.getP25FMEnable()   ||
		m_conf.getNXDNFMEnable()  ||
		m_conf.getFMFMEnable()) {
		toFM = true;
	}

	if (toFM) {
		std::string remoteAddress = m_conf.getFMToRemoteAddress();
		std::string localAddress  = m_conf.getFMToLocalAddress();
		uint16_t remotePort       = m_conf.getFMToRemotePort();
		uint16_t localPort        = m_conf.getFMToLocalPort();
		bool debug                = m_conf.getFMToDebug();

		CFMNetwork* network = new CFMNetwork(NETWORK::TO, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the FM TO network interface");
			closeToNetworks();
			return false;
		}

		m_toNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::FM, network));
	}

	return true;
}

void CMMDVMCrossMode::setThroughModes(CMetaData& data)
{
	data.setThroughModes(
		m_conf.getDStarDStarEnable(),
		m_conf.getDMRDMREnable1(),
		m_conf.getDMRDMREnable2(),
		m_conf.getYSFYSFEnable(),
		m_conf.getP25P25Enable(),
		m_conf.getNXDNNXDNEnable(),
		m_conf.getFMFMEnable());
}

DATA_MODE CMMDVMCrossMode::hasFromNetworkGotData() const
{
	for (const auto& it : m_fromNetworks) {
		bool ret = it.second->hasData();
		if (ret)
			return it.first;
	}

	return DATA_MODE::NONE;
}

DATA_MODE CMMDVMCrossMode::hasToNetworkGotData() const
{
	for (const auto& it : m_toNetworks) {
		bool ret = it.second->hasData();
		if (ret)
			return it.first;
	}

	return DATA_MODE::NONE;
}

bool CMMDVMCrossMode::readFromNetwork(DATA_MODE mode, CMetaData& data)
{
	bool ret = false;

	for (const auto& it : m_fromNetworks) {
		if (mode == it.first)
			ret = it.second->read(data);
		else
			it.second->read();
	}

	return ret;
}

bool CMMDVMCrossMode::readToNetwork(DATA_MODE mode, CMetaData& data)
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

bool CMMDVMCrossMode::writeFromNetworkData(DATA_MODE mode, CMetaData& data)
{
	const auto& it = m_fromNetworks.find(mode);
	if (it == m_fromNetworks.end())
		return false;

	return it->second->writeData(data);
}

bool CMMDVMCrossMode::writeToNetworkData(DATA_MODE mode, CMetaData& data)
{
	const auto& it = m_toNetworks.find(mode);
	if (it == m_toNetworks.end())
		return false;

	return it->second->writeData(data);
}

bool CMMDVMCrossMode::writeFromNetworkRaw(DATA_MODE mode, CMetaData& data)
{
	const auto& it = m_fromNetworks.find(mode);
	if (it == m_fromNetworks.end())
		return false;

	return it->second->writeRaw(data);
}

bool CMMDVMCrossMode::writeToNetworkRaw(DATA_MODE mode, CMetaData& data)
{
	const auto& it = m_toNetworks.find(mode);
	if (it == m_toNetworks.end())
		return false;

	return it->second->writeRaw(data);
}

void CMMDVMCrossMode::resetFromNetworks()
{
	for (auto& it : m_fromNetworks)
		it.second->reset();
}

void CMMDVMCrossMode::resetToNetworks()
{
	for (auto& it : m_toNetworks)
		it.second->reset();
}

void CMMDVMCrossMode::drainFromNetworks()
{
	for (auto& it : m_fromNetworks)
		it.second->read();
}

void CMMDVMCrossMode::drainToNetworks()
{
	for (auto& it : m_toNetworks)
		it.second->read();
}

void CMMDVMCrossMode::clockFromNetworks(unsigned int ms)
{
	for (auto& it : m_fromNetworks)
		it.second->clock(ms);
}

void CMMDVMCrossMode::clockToNetworks(unsigned int ms)
{
	for (auto& it : m_toNetworks)
		it.second->clock(ms);
}

void CMMDVMCrossMode::closeFromNetworks()
{
	for (auto& it : m_fromNetworks) {
		it.second->close();
		delete it.second;
	}

	m_fromNetworks.clear();
}

void CMMDVMCrossMode::closeToNetworks()
{
	for (auto& it : m_toNetworks) {
		it.second->close();
		delete it.second;
	}

	m_toNetworks.clear();
}

void CMMDVMCrossMode::loadModeTranslationTables(CMetaData& data)
{
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

	if (m_conf.getP25DStarEnable())
		data.setP25DStarTGs(m_conf.getP25DStarTGs());
	if (m_conf.getP25DMREnable())
		data.setP25DMRTGs(m_conf.getP25DMRTGs());
	if (m_conf.getP25YSFEnable())
		data.setP25YSFTGs(m_conf.getP25YSFTGs());
	if (m_conf.getP25NXDNEnable())
		data.setP25NXDNTGs(m_conf.getP25NXDNTGs());
	if (m_conf.getP25FMEnable())
		data.setP25FMTG(m_conf.getP25FMTG());

	if (m_conf.getNXDNDStarEnable())
		data.setNXDNDStarTGs(m_conf.getNXDNDStarTGs());
	if (m_conf.getNXDNDMREnable())
		data.setNXDNDMRTGs(m_conf.getNXDNDMRTGs());
	if (m_conf.getNXDNYSFEnable())
		data.setNXDNYSFTGs(m_conf.getNXDNYSFTGs());
	if (m_conf.getNXDNP25Enable())
		data.setNXDNP25TGs(m_conf.getNXDNP25TGs());
	if (m_conf.getNXDNFMEnable())
		data.setNXDNFMTG(m_conf.getNXDNFMTG());
}

bool CMMDVMCrossMode::loadIdLookupTables(CMetaData& data)
{
	std::string dmrFileName  = m_conf.getDMRLookupFile();
	std::string nxdnFileName = m_conf.getNXDNLookupFile();
	unsigned int reloadTime  = m_conf.getReloadTime();

	bool ret = data.setDMRLookup(dmrFileName, reloadTime);
	if (!ret)
		return false;

	return data.setNXDNLookup(nxdnFileName, reloadTime);
}

void CMMDVMCrossMode::writeJSONMessage(const std::string& message)
{
	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();
	json["message"] = message;

	WriteJSON("Message", json);
}
