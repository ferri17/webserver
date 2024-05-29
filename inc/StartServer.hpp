
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
#include "Colors.hpp"

#define MAX_CONNECTION_BACKLOG 200
#define BUFFER_SIZE 1024

#define TIMER_EV_IDENT 4200000
#define TIMER_LOOP_MS 1000
#define REQ_TIMEOUT_MS 5000


typedef struct socketServ {
	int					servSock;
	std::vector<int>	clientSock;
	std::vector<Server> servers;
}	socketServ;

typedef struct mssg {
	Request		req;
	std::string	res;
	bool		closeOnEnd;
} mssg;

void					startServers(std::vector<Server> & s);
std::vector<socketServ>	initSockets(std::vector<Server> & s);
Server &				getTargetServer(std::vector<std::pair<Server &, int> >, int fdTarget);
void					resolveBindingError(struct sockaddr_in & cAddress, std::vector<struct sockaddr_in> & addresses, 
							std::vector<socketServ> & sockets, t_listen & cListen, Server &	cServ);
Server &				resolveServerName(Request & req, std::vector<Server> & servers);
void					runEventLoop(int kq, std::vector<socketServ> & sockets, size_t size);
bool					isServerSocket(int fd, std::vector<socketServ> & sockets);
socketServ &			getSocketServ(int targetFd, std::vector<socketServ> & sockets);
void					cleanServer(int kq, std::vector<socketServ> & sockets);
void					disconnectClient(int kq, int fd, std::vector<socketServ> & sockets, std::map<int, mssg> & mssg);
void					readFromSocket(int kq, int clientSocket, std::map<int, mssg> & mssg, std::vector<socketServ> & sockets);
void					manageRequestState(std::map<int, mssg> & m, int clientSocket, int kq, std::vector<socketServ> & sockets);
void					manageResponse(int clientSocket, int kq, std::vector<socketServ> & sockets, std::map<int, mssg> & m);
