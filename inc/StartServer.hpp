
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

#define MAX_CONNECTION_BACKLOG 10
#define BUFFER_SIZE 64
#define HTTP_ERROR_START 400

#define TIMER_EV_IDENT 4200000
#define TIMER_LOOP_MS 1000
#define REQ_TIMEOUT_MS 20000


typedef struct socketServ {
	int					servSock;
	std::vector<int>	clientSock;
	Server 				serv;
}	socketServ;

typedef struct mssg {
	Request		req;
	std::string	res;
	size_t		timeout;
	bool		closeOnEnd;
} mssg;

void					startServers(std::vector<Server> & s);
std::vector<socketServ>	initSockets(std::vector<Server> & s);
Server &				getTargetServer(std::vector<std::pair<Server &, int> >, int fdTarget);
void					runEventLoop(int kq, std::vector<socketServ> & sockets, size_t size);
bool					isServerSocket(int fd, std::vector<socketServ> & sockets);
socketServ &			getSocketServ(int targetFd, std::vector<socketServ> & sockets);
void					cleanServer(int kq, std::vector<socketServ> & sockets);
void					disconnectClient(int kq, int fd, std::vector<socketServ> & sockets, std::map<int, mssg> & mssg);
void					readFromSocket(int kq, int clientSocket, std::map<int, mssg> & mssg, std::vector<socketServ> & sockets);
void					manageRequestState(mssg & message, int clientSocket, int kq, std::vector<socketServ> & sockets);
void					manageResponse(int clientSocket, int kq, std::vector<socketServ> & sockets, std::map<int, mssg> & m);