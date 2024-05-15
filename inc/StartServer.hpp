
#pragma once

#include <exception>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

#define MAX_CONNECTION_BACKLOG 10
#define BUFFER_SIZE 10

typedef struct socketServ {
	int					servSock;
	std::vector<int>	clientSock;
	Server 				serv;
}	socketServ;

typedef struct mssg {
	Request		req;
	Response	res;
} mssg;

void					startServers(std::vector<Server> & s);
std::vector<socketServ>	initSockets(std::vector<Server> & s);
Server &				getTargetServer(std::vector<std::pair<Server &, int> >, int fdTarget);
void					runEventLoop(int kq, std::vector<socketServ> & sockets, size_t size);
bool					isServerSocket(int fd, std::vector<socketServ> & sockets);
socketServ &			getSocketServ(int targetFd, std::vector<socketServ> & sockets);
void					cleanServer(int kq, std::vector<socketServ> & sockets);
