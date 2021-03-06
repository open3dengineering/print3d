/*
 * This file is part of the Doodle3D project (http://doodle3d.com).
 *
 * Copyright (c) 2013, Doodle3D
 * This software is licensed under the terms of the GNU GPL v2 or later.
 * See file LICENSE.txt or visit http://www.gnu.org/licenses/gpl.html for full license details.
 */

#ifndef SERVER_H_SEEN
#define SERVER_H_SEEN

#include <sys/un.h>
#include <string>
#include <set>
#include <vector>
#include "Logger.h"
#include "../drivers/AbstractDriver.h"

class Client;

class Server {
public:
	static const bool FORK_BY_DEFAULT;

	typedef std::vector<Client*> vec_ClientP;
	typedef std::set<int> set_int;

	Server(const std::string& serialPortName, const std::string& socketPath, const std::string& printerName = "");
	~Server();

	int start(bool fork = FORK_BY_DEFAULT);
	bool registerFileDescriptor(int fd);
	bool unregisterFileDescriptor(int fd);

	AbstractDriver* getDriver();
	const AbstractDriver* getDriver() const;

	//will either not return, or return false
	bool requestExit(int rv);

private:
	static const int SOCKET_MAX_BACKLOG;
	static const int SELECT_LOG_FAST_LOOP; ///A message will be logged if select returns quicker than this threshold (-1 to disable)

	Server(const Server& o);
	void operator=(const Server& o);

	const std::string socketPath_;

	const Logger& log_;
	int socketFd_;
	set_int registeredFds_;
	AbstractDriver* printerDriver_;

	vec_ClientP clients_;

	bool openSocket();
	bool closeSocket();
	int forkProcess();
	int openPort();

  int driverDelay;
};

#endif /* ! SERVER_H_SEEN */
