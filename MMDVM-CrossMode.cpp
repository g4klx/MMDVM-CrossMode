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
m_rfNetworks(),
m_netNetworks()
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
	DATA_MODE rfMode    = DATA_MODE::NONE;
	DATA_MODE netMode   = DATA_MODE::NONE;

	ret = createRFNetworks();
	if (!ret)
		return 1;

	ret = createNetNetworks();
	if (!ret) {
		closeRFNetworks();
		return 1;
	}

	setThroughModes(data);

	loadModeTranslationTables(data);

	ret = loadIdLookupTables(data);
	if (!ret) {
		closeRFNetworks();
		closeNetNetworks();
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
		bool end = false;

		switch (direction) {
		case DIRECTION::RF_TO_NET:
			ret = readRFNetwork(rfMode, data);
			if (ret) {
				if (netMode == DATA_MODE::NONE) {
					netMode = data.getNetMode();
					if (netMode == DATA_MODE::NONE) {
						// Not a valid cross-mode combination
						resetRFNetworks();
						resetNetNetworks();

						data.reset();

						direction = DIRECTION::NONE;
						data.setDirection(direction);

						netTimer.stop();
						rfTimer.stop();
						watchdog.stop();
					} else {
						// Valid
						::LogMessage("Switched by RF activity RF:%s Net:%s", CUtils::getModeName(rfMode).c_str(), CUtils::getModeName(netMode).c_str());
						data.setTranscoder();
					}
				}

				if (netMode != DATA_MODE::NONE) {
					netTimer.stop();
					rfTimer.start();
					watchdog.start();
				}
			}

			drainNetNetworks();

			if (netMode != DATA_MODE::NONE) {
				end = data.isEnd();

				if (data.isTranscode()) {
					if (data.hasData() || end)
						writeNetNetworkData(netMode, data);
				} else {
					if (data.hasRaw() || end)
						writeNetNetworkRaw(netMode, data);
				}
			}

			break;

		case DIRECTION::NET_TO_RF:
			ret = readNetNetwork(netMode, data);
			if (ret) {
				if (rfMode == DATA_MODE::NONE) {
					rfMode = data.getRFMode();
					if (rfMode == DATA_MODE::NONE) {
						// Not a valid cross-mode combination
						resetRFNetworks();
						resetNetNetworks();

						data.reset();

						direction = DIRECTION::NONE;
						data.setDirection(direction);

						netTimer.stop();
						rfTimer.stop();
						watchdog.stop();
					} else {
						// Valid
						::LogMessage("Switched by Net activity RF:%s Net:%s", CUtils::getModeName(rfMode).c_str(), CUtils::getModeName(netMode).c_str());
						data.setTranscoder();
					}
				}

				if (rfMode != DATA_MODE::NONE) {
					rfTimer.stop();
					netTimer.start();
					watchdog.start();
				}
			}

			drainRFNetworks();

			if (rfMode != DATA_MODE::NONE) {
				end = data.isEnd();

				if (data.isTranscode()) {
					if (data.hasData() || end)
						writeRFNetworkData(rfMode, data);
				} else {
					if (data.hasRaw() || end)
						writeRFNetworkRaw(rfMode, data);
				}
			}

			break;

		default:
			rfMode = hasRFNetworkGotData();
			if (rfMode != DATA_MODE::NONE) {
				direction = DIRECTION::RF_TO_NET;
				data.setDirection(direction);
				netTimer.stop();
				rfTimer.start();
				watchdog.start();
				break;
			}

			netMode = hasNetNetworkGotData();
			if (netMode != DATA_MODE::NONE) {
				direction = DIRECTION::NET_TO_RF;
				data.setDirection(direction);
				rfTimer.stop();
				netTimer.start();
				watchdog.start();
				break;
			}

			break;
		}

		if (end) {
			resetRFNetworks();
			resetNetNetworks();
			data.reset();
			direction = DIRECTION::NONE;
			data.setDirection(direction);
			watchdog.stop();
		}

		CThread::sleep(5U);

		unsigned int elapsed = stopwatch.elapsed();
		stopwatch.start();

		clockRFNetworks(elapsed);
		clockNetNetworks(elapsed);
		data.clock(elapsed);

		rfTimer.clock(elapsed);
		if (rfTimer.isRunning() && rfTimer.hasExpired()) {
			resetRFNetworks();
			resetNetNetworks();
			data.setLost();
			data.reset();
			direction = DIRECTION::NONE;
			rfMode    = DATA_MODE::NONE;
			netMode   = DATA_MODE::NONE;
			rfTimer.stop();
			::LogMessage("Switched back to Idle by the RF timer");
		}

		netTimer.clock(elapsed);
		if (netTimer.isRunning() && netTimer.hasExpired()) {
			resetRFNetworks();
			resetNetNetworks();
			data.reset();
			direction = DIRECTION::NONE;
			rfMode    = DATA_MODE::NONE;
			netMode   = DATA_MODE::NONE;
			netTimer.stop();
			::LogMessage("Switched back to Idle by the Net timer");
		}

		watchdog.clock(elapsed);
		if (watchdog.isRunning() && watchdog.hasExpired()) {
			resetRFNetworks();
			resetNetNetworks();
			data.reset();
			direction = DIRECTION::NONE;
			rfMode    = DATA_MODE::NONE;
			netMode   = DATA_MODE::NONE;
			watchdog.stop();
			::LogMessage("The watchdog timer has expired");
		}
	}

	LogInfo("MMDVM-CrossMode is stopping");
	writeJSONMessage("MMDVM-CrossMode is stopping");

	data.close();

	closeRFNetworks();

	closeNetNetworks();

	CUDPSocket::shutdown();

	return 0;
}

bool CMMDVMCrossMode::createRFNetworks()
{
	closeRFNetworks();
	
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

		std::string remoteAddress = m_conf.getDStarRFRemoteAddress();
		std::string localAddress  = m_conf.getDStarRFLocalAddress();
		uint16_t remotePort       = m_conf.getDStarRFRemotePort();
		uint16_t localPort        = m_conf.getDStarRFLocalPort();
		bool debug                = m_conf.getDStarRFDebug();

		CDStarNetwork* network = new CDStarNetwork(NETWORK::RF, dstarCallsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the D-Star RF network interface");
			closeRFNetworks();
			return false;
		}

		m_rfNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::DSTAR, network));
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

		std::string remoteAddress = m_conf.getDMRRFRemoteAddress();
		std::string localAddress  = m_conf.getDMRRFLocalAddress();
		uint16_t remotePort       = m_conf.getDMRRFRemotePort();
		uint16_t localPort        = m_conf.getDMRRFLocalPort();
		bool debug                = m_conf.getDMRRFDebug();

		CDMRNetwork* network = new CDMRNetwork(NETWORK::RF, id, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the DMR RF network interface");
			closeRFNetworks();
			return false;
		}

		m_rfNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::DMR, network));
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
		std::string remoteAddress = m_conf.getYSFRFRemoteAddress();
		std::string localAddress  = m_conf.getYSFRFLocalAddress();
		uint16_t remotePort       = m_conf.getYSFRFRemotePort();
		uint16_t localPort        = m_conf.getYSFRFLocalPort();
		bool debug                = m_conf.getYSFRFDebug();

		CYSFNetwork* network = new CYSFNetwork(NETWORK::RF, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the System Fusion RF network interface");
			closeRFNetworks();
			return false;
		}

		m_rfNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::YSF, network));
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
		std::string remoteAddress = m_conf.getP25RFRemoteAddress();
		std::string localAddress  = m_conf.getP25RFLocalAddress();
		uint16_t remotePort       = m_conf.getP25RFRemotePort();
		uint16_t localPort        = m_conf.getP25RFLocalPort();
		bool debug                = m_conf.getP25RFDebug();

		CP25Network* network = new CP25Network(NETWORK::RF, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the P25 RF network interface");
			closeRFNetworks();
			return false;
		}

		m_rfNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::P25, network));
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
		std::string remoteAddress = m_conf.getNXDNRFRemoteAddress();
		std::string localAddress  = m_conf.getNXDNRFLocalAddress();
		uint16_t remotePort       = m_conf.getNXDNRFRemotePort();
		uint16_t localPort        = m_conf.getNXDNRFLocalPort();
		bool debug                = m_conf.getNXDNRFDebug();

		CNXDNNetwork* network = new CNXDNNetwork(NETWORK::RF, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the NXDN RF network interface");
			closeRFNetworks();
			return false;
		}

		m_rfNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::NXDN, network));
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
		std::string remoteAddress = m_conf.getFMRFRemoteAddress();
		std::string localAddress  = m_conf.getFMRFLocalAddress();
		uint16_t remotePort       = m_conf.getFMRFRemotePort();
		uint16_t localPort        = m_conf.getFMRFLocalPort();
		bool debug                = m_conf.getFMRFDebug();

		CFMNetwork* network = new CFMNetwork(NETWORK::RF, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the FM RF network interface");
			closeRFNetworks();
			return false;
		}

		m_rfNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::FM, network));
	}

	return true;
}

bool CMMDVMCrossMode::createNetNetworks()
{
	closeNetNetworks();

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

		std::string remoteAddress = m_conf.getDStarNetRemoteAddress();
		std::string localAddress  = m_conf.getDStarNetLocalAddress();
		uint16_t remotePort       = m_conf.getDStarNetRemotePort();
		uint16_t localPort        = m_conf.getDStarNetLocalPort();
		bool debug                = m_conf.getDStarNetDebug();

		CDStarNetwork* network = new CDStarNetwork(NETWORK::NET, dstarCallsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the D-Star Net network interface");
			closeNetNetworks();
			return false;
		}

		m_netNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::DSTAR, network));
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

		std::string remoteAddress = m_conf.getDMRNetRemoteAddress();
		std::string localAddress  = m_conf.getDMRNetLocalAddress();
		uint16_t remotePort       = m_conf.getDMRNetRemotePort();
		uint16_t localPort        = m_conf.getDMRNetLocalPort();
		bool debug                = m_conf.getDMRNetDebug();

		uint32_t txFrequency = m_conf.getInfoTXFrequency();
		uint32_t rxFrequency = m_conf.getInfoRXFrequency();
		uint8_t  colorCode   = m_conf.getInfoColorCode();
		uint16_t power       = m_conf.getInfoPower();

		CDMRNetwork* network = new CDMRNetwork(NETWORK::NET, id, localAddress, localPort, remoteAddress, remotePort, debug);
		network->setConfig(callsign, VERSION, txFrequency, rxFrequency, colorCode, power);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the DMR Net network interface");
			closeNetNetworks();
			return false;
		}

		m_netNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::DMR, network));
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
		std::string remoteAddress = m_conf.getYSFNetRemoteAddress();
		std::string localAddress  = m_conf.getYSFNetLocalAddress();
		uint16_t remotePort       = m_conf.getYSFNetRemotePort();
		uint16_t localPort        = m_conf.getYSFNetLocalPort();
		bool debug                = m_conf.getYSFNetDebug();

		CYSFNetwork* network = new CYSFNetwork(NETWORK::NET, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the System Fusion Net network interface");
			closeNetNetworks();
			return false;
		}

		m_netNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::YSF, network));
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
		std::string remoteAddress = m_conf.getP25NetRemoteAddress();
		std::string localAddress  = m_conf.getP25NetLocalAddress();
		uint16_t remotePort       = m_conf.getP25NetRemotePort();
		uint16_t localPort        = m_conf.getP25NetLocalPort();
		bool debug                = m_conf.getP25NetDebug();

		CP25Network* network = new CP25Network(NETWORK::NET, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the P25 Net network interface");
			closeNetNetworks();
			return false;
		}

		m_netNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::P25, network));
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
		std::string remoteAddress = m_conf.getNXDNNetRemoteAddress();
		std::string localAddress  = m_conf.getNXDNNetLocalAddress();
		uint16_t remotePort       = m_conf.getNXDNNetRemotePort();
		uint16_t localPort        = m_conf.getNXDNNetLocalPort();
		bool debug                = m_conf.getNXDNNetDebug();

		CNXDNNetwork* network = new CNXDNNetwork(NETWORK::NET, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the NXDN Net network interface");
			closeNetNetworks();
			return false;
		}

		m_netNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::NXDN, network));
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
		std::string remoteAddress = m_conf.getFMNetRemoteAddress();
		std::string localAddress  = m_conf.getFMNetLocalAddress();
		uint16_t remotePort       = m_conf.getFMNetRemotePort();
		uint16_t localPort        = m_conf.getFMNetLocalPort();
		bool debug                = m_conf.getFMNetDebug();

		CFMNetwork* network = new CFMNetwork(NETWORK::NET, callsign, localAddress, localPort, remoteAddress, remotePort, debug);

		bool ret = network->open();
		if (!ret) {
			LogError("Unable to open the FM Net network interface");
			closeNetNetworks();
			return false;
		}

		m_netNetworks.insert(std::pair<DATA_MODE, INetwork*>(DATA_MODE::FM, network));
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

DATA_MODE CMMDVMCrossMode::hasRFNetworkGotData() const
{
	for (const auto& it : m_rfNetworks) {
		bool ret = it.second->hasData();
		if (ret)
			return it.first;
	}

	return DATA_MODE::NONE;
}

DATA_MODE CMMDVMCrossMode::hasNetNetworkGotData() const
{
	for (const auto& it : m_netNetworks) {
		bool ret = it.second->hasData();
		if (ret)
			return it.first;
	}

	return DATA_MODE::NONE;
}

bool CMMDVMCrossMode::readRFNetwork(DATA_MODE mode, CMetaData& data)
{
	bool ret = false;

	for (const auto& it : m_rfNetworks) {
		if (mode == it.first)
			ret = it.second->read(data);
		else
			it.second->read();
	}

	return ret;
}

bool CMMDVMCrossMode::readNetNetwork(DATA_MODE mode, CMetaData& data)
{
	bool ret = false;

	for (const auto& it : m_netNetworks) {
		if (mode == it.first)
			ret = it.second->read(data);
		else
			it.second->read();
	}

	return ret;
}

bool CMMDVMCrossMode::writeRFNetworkData(DATA_MODE mode, CMetaData& data)
{
	const auto& it = m_rfNetworks.find(mode);
	if (it == m_rfNetworks.end())
		return false;

	return it->second->writeData(data);
}

bool CMMDVMCrossMode::writeNetNetworkData(DATA_MODE mode, CMetaData& data)
{
	const auto& it = m_netNetworks.find(mode);
	if (it == m_netNetworks.end())
		return false;

	return it->second->writeData(data);
}

bool CMMDVMCrossMode::writeRFNetworkRaw(DATA_MODE mode, CMetaData& data)
{
	const auto& it = m_rfNetworks.find(mode);
	if (it == m_rfNetworks.end())
		return false;

	return it->second->writeRaw(data);
}

bool CMMDVMCrossMode::writeNetNetworkRaw(DATA_MODE mode, CMetaData& data)
{
	const auto& it = m_netNetworks.find(mode);
	if (it == m_netNetworks.end())
		return false;

	return it->second->writeRaw(data);
}

void CMMDVMCrossMode::resetRFNetworks()
{
	for (auto& it : m_rfNetworks)
		it.second->reset();
}

void CMMDVMCrossMode::resetNetNetworks()
{
	for (auto& it : m_netNetworks)
		it.second->reset();
}

void CMMDVMCrossMode::drainRFNetworks()
{
	for (auto& it : m_rfNetworks)
		it.second->read();
}

void CMMDVMCrossMode::drainNetNetworks()
{
	for (auto& it : m_netNetworks)
		it.second->read();
}

void CMMDVMCrossMode::clockRFNetworks(unsigned int ms)
{
	for (auto& it : m_rfNetworks)
		it.second->clock(ms);
}

void CMMDVMCrossMode::clockNetNetworks(unsigned int ms)
{
	for (auto& it : m_netNetworks)
		it.second->clock(ms);
}

void CMMDVMCrossMode::closeRFNetworks()
{
	for (auto& it : m_rfNetworks) {
		it.second->close();
		delete it.second;
	}

	m_rfNetworks.clear();
}

void CMMDVMCrossMode::closeNetNetworks()
{
	for (auto& it : m_netNetworks) {
		it.second->close();
		delete it.second;
	}

	m_netNetworks.clear();
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
