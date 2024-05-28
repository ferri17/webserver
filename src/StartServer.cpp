
#include "Headers.hpp"
#include "Signals.hpp"
#include "StartServer.hpp"
#include <dirent.h>
#include "ResponseGen.hpp"

int	createNewSocket(t_listen & list, struct sockaddr_in & cAddress, std::vector<struct sockaddr_in> & addresses)
{
	std::string			ip = list.ip;
	std::string			port = toString(list.port);
	int					localSocket, errGai, option;
	struct addrinfo		*addr;
	struct addrinfo		hints;
	
	if (ip.empty())
		ip = "localhost";
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((errGai = getaddrinfo(ip.c_str(), port.c_str(), &hints, &addr)) != 0)
	{
		throw std::runtime_error(gai_strerror(errGai));
	}
	memcpy(&cAddress, addr->ai_addr, sizeof(sockaddr_in));
	if ((localSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0)
	{
		freeaddrinfo(addr);
		throw std::runtime_error(strerror(errno));
	}
	if (setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
	{
		freeaddrinfo(addr);
		throw std::runtime_error(strerror(errno));
	}
	if (fcntl(localSocket, F_SETFL, O_NONBLOCK | FD_CLOEXEC) < 0)
	{
		freeaddrinfo(addr);
		throw std::runtime_error(strerror(errno));
	}
	if (bind(localSocket, addr->ai_addr, addr->ai_addrlen) != 0)
	{
		freeaddrinfo(addr);
		return (-1);
		/* sockaddr_in* errAddr = reinterpret_cast<sockaddr_in*>(addr->ai_addr);
		for (std::vector<struct sockaddr_in>::iterator it = addresses.begin(); it != addresses.end(); it++)
		{
			if (!memcmp(&(errAddr->sin_addr),&(it->sin_addr), sizeof (in_addr)) && errAddr->sin_port == (*it).sin_port)
				std::cout << "match" << std::endl;
		}
		int port = ntohs(errAddr->sin_port);
		std::string	mssgErr = strerror(errno);
		mssgErr += ": Port -> ";
		mssgErr += toString(port);
		throw std::runtime_error(mssgErr);*/
	}
	if (listen(localSocket, MAX_CONNECTION_BACKLOG) != 0)
	{
		freeaddrinfo(addr);
		throw std::runtime_error(strerror(errno));
	}
	freeaddrinfo(addr);
	addresses.push_back(cAddress);
	return (localSocket);
}

std::vector<socketServ>	initSockets(std::vector<Server> & s)
{
	int							sId = 0;
	socketServ					socks;
	std::vector<t_listen>		cListen;
	std::vector<socketServ>		sockets;
	struct sockaddr_in			cAddress;
	std::vector<struct sockaddr_in>	addresses;


	for (std::vector<Server>::iterator itServ = s.begin(); itServ != s.end(); itServ++)
	{
		Server &	currentServ = (*itServ);

		socks.servers.push_back(currentServ);
		cListen = currentServ.getListen();
		std::cout << getTime() << BOLD GREEN "Server #" << sId << " running:" NC << std::endl;
		for (std::vector<t_listen>::iterator itListen = cListen.begin(); itListen != cListen.end(); itListen++)
		{
			try
			{
				socks.servSock = createNewSocket(*itListen, cAddress, addresses);
				if (socks.servSock > 0)
					sockets.push_back(socks);
				else
				{
					resolveBindingError(cAddress, addresses, sockets, (*itListen), currentServ);
				}
				std::cout << PURPLE "\t\t\tListening: " << (*itListen).ip << ":" << (*itListen).port << NC << std::endl;
			}
			catch(const std::exception& e)
			{
				std::cerr << getTime() << BOLD RED << e.what() << NC << std::endl;
			}
		}
		sId++;
	}
	return (sockets);
}

void	addNewClient(int kq, int targetSock, std::vector<socketServ> & sockets)
{
	int							newClient;
	struct kevent				evSet;
	struct sockaddr_in			addrCl;
	socklen_t					addrLenCl = sizeof(addrCl);

	if ((newClient = accept(targetSock, (sockaddr *)&addrCl, &addrLenCl)) < 0)
		std::cerr << getTime() << RED BOLD "Accept: " << strerror(errno) << NC << std::endl;
	else
	{
		socketServ & tmpServ = getSocketServ(targetSock, sockets);
		tmpServ.clientSock.push_back(newClient);
		EV_SET(&evSet, newClient, EVFILT_READ, EV_ADD, 0, 0, NULL);
		if (kevent(kq, &evSet, 1, NULL, 0, NULL) < 0)
		{
			cleanServer(kq, sockets);
			throw std::runtime_error(strerror(errno));
		}
		std::cout << getTime() << GREEN BOLD "Client #" << newClient << " connected" NC << std::endl;
	}
}

void	cleanServer(int kq, std::vector<socketServ> & sockets)
{
	for (std::vector<socketServ>::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		std::vector<int> &	cliVec = (*itS).clientSock;
		for (std::vector<int>::iterator itV = cliVec.begin(); itV != cliVec.end(); itV++)
		{
			close(*itV);
		}
		close((*itS).servSock);
	}
	if (kq > 0)
		close(kq);
}

void	disconnectClient(int kq, int fd, std::vector<socketServ> & sockets, std::map<int, mssg> & m)
{
	struct kevent	evSet;

	for (std::vector<socketServ>::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		std::vector<int> &	cliVec = (*itS).clientSock;
		for (std::vector<int>::iterator itV = cliVec.begin(); itV != cliVec.end(); itV++)
		{
			if (*itV == fd)
			{
				cliVec.erase(itV);
				break ;
			}
		}
	}
	m.erase(fd);
	EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, NULL ,0, NULL);
	if (kevent(kq, &evSet, 1, NULL, 0, NULL) < 0)
	{
		close(fd);
		cleanServer(kq, sockets);
		throw std::runtime_error(strerror(errno));
	}
	close(fd);
	std::cout << getTime() << PURPLE BOLD "Client #" << fd << " disconnected" NC << std::endl;
}

void	readFromSocket(int kq, int clientSocket, std::map<int, mssg> & m, std::vector<socketServ> & sockets)
{
	int			bytesRead;
	char		buffer[BUFFER_SIZE + 1];
	Request &	currentReq = m[clientSocket].req;
	Server &	currentServ = getSocketServ(clientSocket, sockets).servers.front();

	bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
	if (bytesRead <= 0)
	{
		disconnectClient(kq, clientSocket, sockets, m);
		if (bytesRead < 0)
			std::cerr << strerror(errno) << std::endl;
		return ;
	}
	buffer[bytesRead] = '\0';
	currentReq.parseNewBuffer(buffer, bytesRead, currentServ.getClientMaxBodySize());
	std::cout << getTime() << BLUE BOLD "Reading data from client #" << clientSocket << "..." << NC << std::endl;
}

void	manageRequestState(mssg & m, int clientSocket, int kq, std::vector<socketServ> & sockets)
{
	struct kevent	evSet[2];

	if (m.req.getState() == __FINISHED__)
	{
		m.req.setTimeout(-1);
		Server &	serv = resolveServerName(m.req, getSocketServ(clientSocket, sockets).servers);
		ResponseGen	res(m.req, serv, m.closeOnEnd);
		m.res = res.DoResponse().generateResponse();
		EV_SET(&evSet[0], clientSocket, EVFILT_READ, EV_DELETE, 0, 0, 0);
		EV_SET(&evSet[1], clientSocket, EVFILT_WRITE, EV_ADD, 0, 0, 0);
		if (kevent(kq, evSet, 2, 0, 0, 0) < 0)
		{
			cleanServer(kq, sockets);
			throw std::runtime_error(strerror(errno));
		}
	}
}

void	manageResponse(int clientSocket, int kq, std::vector<socketServ> & sockets, std::map<int, mssg> & m)
{
	int				bytesSent;
	mssg &			message = m[clientSocket];
	std::string		buff;
	struct kevent	evSet[2];

	buff = message.res.substr(0, BUFFER_SIZE);
	std::cout << getTime() << YELLOW BOLD "Sending data to client #" << clientSocket << "..." << NC << std::endl;
	if ((bytesSent = send(clientSocket, buff.data(), buff.size(), 0)) < 0)
		std::cerr << RED BOLD << strerror(errno) << NC << std::endl;
	else
		message.res = message.res.substr(bytesSent, std::string::npos);
	if (message.res.empty())
	{
		std::string		tmpRemainder = message.req.getRemainder();

		std::cout << getTime() << GREEN BOLD "Request served successfully to client #" << clientSocket << NC << std::endl;
		EV_SET(&evSet[0], clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
		EV_SET(&evSet[1], clientSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
		if (kevent(kq, evSet, 2, 0, 0, 0) < 0)
		{
			cleanServer(kq, sockets);
			throw std::runtime_error(strerror(errno));
		}
		if (message.closeOnEnd)
			disconnectClient(kq, clientSocket, sockets, m);
		else
		{
			m.erase(clientSocket);
			m[clientSocket].req.setRemainder(tmpRemainder);
		}
	}
}

void	updateTimers(int kq, std::map<int, mssg> & m, std::vector<socketServ> & sockets)
{
	struct kevent	evSet[2];

	for (std::map<int, mssg>::iterator it = m.begin(); it != m.end(); it++)
	{
		if ((*it).second.req.getTimeout() >= 0)
			(*it).second.req.addTimeout(TIMER_LOOP_MS);
		if ((*it).second.req.getTimeout() >= REQ_TIMEOUT_MS)
		{
			(*it).second.req.setTimeout(-1);
			EV_SET(&evSet[0], (*it).first, EVFILT_READ, EV_DELETE, 0, 0, 0);
			EV_SET(&evSet[1], (*it).first, EVFILT_WRITE, EV_ADD, 0, 0, 0);
			if (kevent(kq, evSet, 2, 0, 0, 0) < 0)
			{
				cleanServer(kq, sockets);
				throw std::runtime_error(strerror(errno));
			}
			m[(*it).first].req.setErrorCode(REQUEST_TIMEOUT);
			m[(*it).first].req.setState(__FINISHED__);
			Server &	serv = resolveServerName(m[(*it).first].req, getSocketServ((*it).first, sockets).servers);
			ResponseGen	res(m[(*it).first].req, serv, m[(*it).first].closeOnEnd);
			m[(*it).first].res = res.DoResponse().generateResponse();
		}
	}
}


void	runEventLoop(int kq, std::vector<socketServ> & sockets, size_t size)
{
	std::map<int, mssg>			m;
	std::vector<struct kevent>	evList(size);
	int							nbEvents;
	
	while (signaled)
	{
		if ((nbEvents = kevent(kq, NULL, 0, evList.data(), size, NULL)) < 0)
		{
			cleanServer(kq, sockets);
			throw std::runtime_error(strerror(errno));
		}
		for (int i = 0; i < nbEvents; i++)
		{
			int	clientSocket = evList[i].ident;

			if (isServerSocket(clientSocket, sockets))
				addNewClient(kq, clientSocket, sockets);
			else if (evList[i].flags & EV_EOF)
				disconnectClient(kq, clientSocket, sockets, m);
			else if (evList[i].filter == EVFILT_READ)
			{
				readFromSocket(kq, clientSocket, m, sockets);
				manageRequestState(m[clientSocket], clientSocket, kq, sockets);
			}
			else if (evList[i].filter == EVFILT_WRITE)
				manageResponse(clientSocket, kq, sockets, m);
			else if (evList[i].filter == EVFILT_TIMER)
				updateTimers(kq, m, sockets);
		}
	}
	cleanServer(kq, sockets);
}

void startServers(std::vector<Server> & s)
{
	int							kq;
	struct kevent				tEvent;
	std::vector<socketServ>		sockets;
	std::vector<struct kevent>	evSet;

	std::cout << getTime() << BOLD GREEN "Starting servers..." NC << std::endl;
	sockets = initSockets(s);
	if ((kq = kqueue()) == -1)
	{
		throw std::runtime_error(strerror(errno));
	}
	for (std::vector<socketServ>::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		struct kevent	sEvent;
		
		EV_SET(&sEvent, (*itS).servSock, EVFILT_READ, EV_ADD, 0, 0, 0);
		evSet.push_back(sEvent);
	}
	EV_SET(&tEvent, TIMER_EV_IDENT, EVFILT_TIMER, EV_ADD, 0, TIMER_LOOP_MS, 0);
	evSet.push_back(tEvent);
	if (kevent(kq, evSet.data(), evSet.size(), NULL, 0, NULL) == -1)
	{
		cleanServer(kq, sockets);
		throw std::runtime_error(strerror(errno));
	}
	runEventLoop(kq, sockets, evSet.size());
}
