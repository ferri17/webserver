
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
#include "Server.hpp"

#define MAX_CONNECTION_BACKLOG 10

void									startServers(std::vector<Server> & s);
std::vector<std::pair<Server &, int > >	initSockets(std::vector<Server> & s);
Server &								getTargetServer(std::vector<std::pair<Server &, int> >, int fdTarget);
void									runEventLoop(int kq, std::vector<std::pair<Server &, int> > localSockets, size_t size);
bool									isClientSocket(int fd, std::vector<std::pair<Server &, int> > & sockets);

