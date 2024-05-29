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

#include "MQTTConnection.h"
#include "DStarNetwork.h"
#include "YSFNetwork.h"
#include "M17Network.h"
#include "FMNetwork.h"
#include "StopWatch.h"
#include "Version.h"
#include "Thread.h"
#include "Timer.h"
#include "Log.h"
#include "GitVersion.h"

#include <cstring>
#include <cassert>

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#endif

enum DIRECTION {
	DIR_NONE,
	DIR_FROM_TO,
	DIR_TO_FROM
};

#if defined(_WIN32) || defined(_WIN64)
const char* DEFAULT_INI_FILE = "CrossMode.ini";
#else
const char* DEFAULT_INI_FILE = "/etc/CrossMode.ini";
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

DATA_MODE convertMode(const char* text)
{
	if (::strcmp(text, "dstar") == 0)
		return DATA_MODE_DSTAR;
	else if (::strcmp(text, "dmr") == 0)
		return DATA_MODE_DMR;
	else if (::strcmp(text, "ysf") == 0)
		return DATA_MODE_YSFDN;
	else if (::strcmp(text, "p25") == 0)
		return DATA_MODE_P25;
	else if (::strcmp(text, "nxdn") == 0)
		return DATA_MODE_NXDN;
	else if (::strcmp(text, "fm") == 0)
		return DATA_MODE_FM;
	else if (::strcmp(text, "m17") == 0)
		return DATA_MODE_M17;
	else
		return DATA_MODE_NONE;
}

int main(int argc, char** argv)
{
	std::string iniFile = DEFAULT_INI_FILE;
	DATA_MODE fromMode  = DATA_MODE_DSTAR;
	DATA_MODE toMode    = DATA_MODE_M17;

	if ((argc == 2) && ((::strcmp(argv[1U], "-v") == 0) || (::strcmp(argv[1U], "--version") == 0))) {
		::fprintf(stdout, "CrossMode version %s git #%.7s\n", VERSION, gitversion);
		return 0;
	}

	printf("%s %s %s\n", argv[1], argv[2], argv[3]);

	if (argc == 4) {
		iniFile  = std::string(argv[1U]);
		fromMode = convertMode(argv[2U]);
		toMode   = convertMode(argv[3U]);
	} else {
		::fprintf(stderr, "Usage: CrossMode [-v|--version] [<filename> <from> <to>]\n");
		return 1;
	}

	if ((fromMode == DATA_MODE_NONE) || (toMode == DATA_MODE_NONE)) {
		::fprintf(stderr, "CrossMode: <from> and <to> must be one of:\n");
		::fprintf(stderr, "\tdstar, dmr, ysf, p25, nxdn, fm, m17\n");
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

		CCrossMode* host = new CCrossMode(iniFile, fromMode, toMode);
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

CCrossMode::CCrossMode(const std::string& fileName, DATA_MODE fromMode, DATA_MODE toMode) :
m_conf(fileName),
m_fromMode(fromMode),
m_toMode(toMode),
m_fromNetwork(nullptr),
m_toNetwork(nullptr),
m_throughNetwork(nullptr)
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

	::LogInitialise(m_conf.getLogDisplayLevel(), m_conf.getLogMQTTLevel());

	std::vector<std::pair<std::string, void (*)(const unsigned char*, unsigned int)>> subscriptions;

	m_mqtt = new CMQTTConnection(m_conf.getMQTTAddress(), m_conf.getMQTTPort(), m_conf.getMQTTName(), subscriptions, m_conf.getMQTTKeepalive());
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

	ret = createFromNetwork();
	if (!ret)
		return 1;

	ret = createToNetwork();
	if (!ret) {
		m_fromNetwork->close();
		delete m_fromNetwork;
		return 1;
	}

	ret = createThroughNetwork();
	if (!ret) {
		m_fromNetwork->close();
		m_toNetwork->close();
		delete m_fromNetwork;
		delete m_toNetwork;
		return 1;
	}

	CData data(m_conf.getTranscoderPort(), m_conf.getTranscoderSpeed(), m_conf.getTranscoderDebug(), m_conf.getDefaultCallsign(), m_conf.getDefaultDMRId(), m_conf.getDefaultNXDNId());
	ret = data.open();
	if (!ret) {
		m_fromNetwork->close();
		m_toNetwork->close();
		m_throughNetwork->close();
		delete m_fromNetwork;
		delete m_toNetwork;
		delete m_throughNetwork;
		return 1;
	}

	data.setYSFM17Mapping(m_conf.getYSFM17Mapping());
	data.setM17YSFMapping(m_conf.getM17YSFMapping());

	CStopWatch stopwatch;
	CTimer watchdog(1000U, 0U, 500U);

	DIRECTION direction = DIR_NONE;

	LogMessage("CrossMode-%s is starting", VERSION);
	LogMessage("Built %s %s (GitID #%.7s)", __TIME__, __DATE__, gitversion);

	while (!m_killed) {
		stopwatch.start();

		switch (direction) {
		case DIR_FROM_TO:
			ret = m_fromNetwork->read(data);
			if (ret)
				watchdog.start();

			if (data.isTranscode()) {
				if (data.hasData() || data.isEnd())
					m_toNetwork->writeData(data);
			} else {
				if (data.hasRaw())
					m_throughNetwork->writeRaw(data);
			}
			break;

		case DIR_TO_FROM:
			ret = m_toNetwork->read(data);
			if (ret)
				watchdog.start();

			if (data.isTranscode()) {
				if (data.hasData() || data.isEnd())
					m_fromNetwork->writeData(data);
			}

			ret = m_throughNetwork->read(data);
			if (ret)
				watchdog.start();

			if (!data.isTranscode()) {
				if (data.hasRaw())
					m_fromNetwork->writeRaw(data);
			}
			break;

		default:
			ret = m_fromNetwork->hasData();
			if (ret) {
				watchdog.start();

				ret = data.setModes(m_fromMode, m_toMode);
				if (!ret)
					break;

				direction = DIR_FROM_TO;
				break;
			}

			ret = m_toNetwork->hasData();
			if (ret) {
				watchdog.start();

				ret = data.setModes(m_toMode, m_fromMode);
				if (!ret)
					break;

				direction = DIR_TO_FROM;
				break;
			}
			break;
		}

		if (data.isEnd()) {
			m_fromNetwork->reset();
			m_toNetwork->reset();
			m_throughNetwork->reset();
			data.reset();
			direction = DIR_NONE;
			watchdog.stop();
		}

		CThread::sleep(5U);

		unsigned int elapsed = stopwatch.elapsed();

		m_fromNetwork->clock(elapsed);
		m_toNetwork->clock(elapsed);
		m_throughNetwork->clock(elapsed);
		data.clock(elapsed);

		watchdog.clock(elapsed);
		if (watchdog.isRunning() && watchdog.hasExpired()) {
			::LogMessage("The watchdog timer has exprired");

			m_fromNetwork->reset();
			m_toNetwork->reset();
			m_throughNetwork->reset();
			data.reset();
			direction = DIR_NONE;

			watchdog.stop();
		}
	}

	LogInfo("CrossMode is stopping");

	data.close();

	m_fromNetwork->close();
	m_toNetwork->close();
	m_throughNetwork->close();

	delete m_fromNetwork;
	delete m_toNetwork;
	delete m_throughNetwork;

	CUDPSocket::shutdown();

	return 0;
}

bool CCrossMode::createFromNetwork()
{
	const std::string callsign1 = m_conf.getDefaultCallsign();
	const std::string callsign2 = m_conf.getDStarCallsign();
	std::string localAddress;
	uint16_t    localPort;
	std::string remoteAddress;
	uint16_t    remotePort;
	bool        debug;

	switch (m_fromMode) {
	case DATA_MODE_DSTAR:
		remoteAddress = m_conf.getDStarFromRemoteAddress();
		localAddress  = m_conf.getDStarFromLocalAddress();
		remotePort    = m_conf.getDStarFromRemotePort();
		localPort     = m_conf.getDStarFromLocalPort();
		debug         = m_conf.getDStarFromDebug();
		m_fromNetwork = new CDStarNetwork(callsign2, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_YSFDN:
		remoteAddress = m_conf.getYSFFromRemoteAddress();
		localAddress  = m_conf.getYSFFromLocalAddress();
		remotePort    = m_conf.getYSFFromRemotePort();
		localPort     = m_conf.getYSFFromLocalPort();
		debug         = m_conf.getYSFFromDebug();
		m_fromNetwork = new CYSFNetwork(callsign1, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_FM:
		remoteAddress = m_conf.getFMFromRemoteAddress();
		localAddress  = m_conf.getFMFromLocalAddress();
		remotePort    = m_conf.getFMFromRemotePort();
		localPort     = m_conf.getFMFromLocalPort();
		debug         = m_conf.getFMFromDebug();
		m_fromNetwork = new CFMNetwork(callsign1, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_M17:
		remoteAddress = m_conf.getM17FromRemoteAddress();
		localAddress  = m_conf.getM17FromLocalAddress();
		remotePort    = m_conf.getM17FromRemotePort();
		localPort     = m_conf.getM17FromLocalPort();
		debug         = m_conf.getM17FromDebug();
		m_fromNetwork = new CM17Network(localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	default:
		::LogError("Unknown from mode specified");
		return false;
	}

	bool ret = m_fromNetwork->open();
	if (!ret) {
		::LogError("Unable to open the from network interface");
		delete m_fromNetwork;
		return false;
	}

	return true;
}

bool CCrossMode::createToNetwork()
{
	const std::string callsign1 = m_conf.getDefaultCallsign();
	const std::string callsign2 = m_conf.getDStarCallsign();
	std::string localAddress;
	uint16_t    localPort;
	std::string remoteAddress;
	uint16_t    remotePort;
	bool        debug;

	switch (m_toMode) {
	case DATA_MODE_DSTAR:
		remoteAddress = m_conf.getDStarToRemoteAddress();
		localAddress  = m_conf.getDStarToLocalAddress();
		remotePort    = m_conf.getDStarToRemotePort();
		localPort     = m_conf.getDStarToLocalPort();
		debug         = m_conf.getDStarToDebug();
		m_toNetwork = new CDStarNetwork(callsign2, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_YSFDN:
		remoteAddress = m_conf.getYSFToRemoteAddress();
		localAddress  = m_conf.getYSFToLocalAddress();
		remotePort    = m_conf.getYSFToRemotePort();
		localPort     = m_conf.getYSFToLocalPort();
		debug         = m_conf.getYSFToDebug();
		m_toNetwork = new CYSFNetwork(callsign1, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_FM:
		remoteAddress = m_conf.getFMToRemoteAddress();
		localAddress  = m_conf.getFMToLocalAddress();
		remotePort    = m_conf.getFMToRemotePort();
		localPort     = m_conf.getFMToLocalPort();
		debug         = m_conf.getFMToDebug();
		m_toNetwork = new CFMNetwork(callsign1, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_M17:
		remoteAddress = m_conf.getM17ToRemoteAddress();
		localAddress  = m_conf.getM17ToLocalAddress();
		remotePort    = m_conf.getM17ToRemotePort();
		localPort     = m_conf.getM17ToLocalPort();
		debug         = m_conf.getM17ToDebug();
		m_toNetwork = new CM17Network(localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	default:
		::LogError("Unknown to mode specified");
		return false;
	}

	bool ret = m_toNetwork->open();
	if (!ret) {
		::LogError("Unable to open the to network interface");
		delete m_toNetwork;
		return false;
	}

	return true;
}

bool CCrossMode::createThroughNetwork()
{
	const std::string callsign1 = m_conf.getDefaultCallsign();
	const std::string callsign2 = m_conf.getDStarCallsign();
	std::string localAddress;
	uint16_t    localPort;
	std::string remoteAddress;
	uint16_t    remotePort;
	bool        debug;

	switch (m_fromMode) {
	case DATA_MODE_DSTAR:
		remoteAddress = m_conf.getDStarToRemoteAddress();
		localAddress  = m_conf.getDStarToLocalAddress();
		remotePort    = m_conf.getDStarToRemotePort();
		localPort     = m_conf.getDStarToLocalPort();
		debug         = m_conf.getDStarToDebug();
		m_throughNetwork = new CDStarNetwork(callsign2, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_YSFDN:
		remoteAddress = m_conf.getYSFToRemoteAddress();
		localAddress  = m_conf.getYSFToLocalAddress();
		remotePort    = m_conf.getYSFToRemotePort();
		localPort     = m_conf.getYSFToLocalPort();
		debug         = m_conf.getYSFToDebug();
		m_throughNetwork = new CYSFNetwork(callsign1, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_FM:
		remoteAddress = m_conf.getFMToRemoteAddress();
		localAddress  = m_conf.getFMToLocalAddress();
		remotePort    = m_conf.getFMToRemotePort();
		localPort     = m_conf.getFMToLocalPort();
		debug         = m_conf.getFMToDebug();
		m_throughNetwork = new CFMNetwork(callsign1, localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	case DATA_MODE_M17:
		remoteAddress = m_conf.getM17ToRemoteAddress();
		localAddress  = m_conf.getM17ToLocalAddress();
		remotePort    = m_conf.getM17ToRemotePort();
		localPort     = m_conf.getM17ToLocalPort();
		debug         = m_conf.getM17ToDebug();
		m_throughNetwork = new CM17Network(localAddress, localPort, remoteAddress, remotePort, debug);
		break;
	default:
		::LogError("Unknown through mode specified");
		return false;
	}

	bool ret = m_throughNetwork->open();
	if (!ret) {
		::LogError("Unable to open the through network interface");
		delete m_throughNetwork;
		return false;
	}

	return true;
}
