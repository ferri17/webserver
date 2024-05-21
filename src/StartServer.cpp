
#include "Headers.hpp"
#include "Signals.hpp"
#include "StartServer.hpp"
#include <dirent.h>
#include "ResponseGen.hpp"

int	createNewSocket(t_listen & list)
{
	std::string		ip = list.ip;
	std::string		port = toString(list.port);
	int				localSocket, errGai, option;
	struct addrinfo	*addr;
	struct addrinfo	hints;
	
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
		sockaddr_in* sockAddrIn = reinterpret_cast<sockaddr_in*>(addr->ai_addr);
		int port = ntohs(sockAddrIn->sin_port);
		std::string	mssgErr = strerror(errno);
		mssgErr += ": Port -> ";
		mssgErr += toString(port);
		freeaddrinfo(addr);
		throw std::runtime_error(mssgErr);
	}
	if (listen(localSocket, MAX_CONNECTION_BACKLOG) != 0)
	{
		freeaddrinfo(addr);
		throw std::runtime_error(strerror(errno));
	}
	freeaddrinfo(addr);
	return (localSocket);
}

std::vector<socketServ>	initSockets(std::vector<Server> & s)
{
	int						sId = 0;
	socketServ				socks;
	std::vector<t_listen>	cListen;
	std::vector<socketServ>	sockets;

	for (std::vector<Server>::iterator itServ = s.begin(); itServ != s.end(); itServ++)
	{
		Server &	currentServ = (*itServ);

		socks.serv = currentServ;
		cListen = currentServ.getListen();
		std::cout << getTime() << BOLD GREEN "Server #" << sId << " running:" NC << std::endl;
		for (std::vector<t_listen>::iterator itListen = cListen.begin(); itListen != cListen.end(); itListen++)
		{
			try
			{
				int	fd = createNewSocket(*itListen);
				socks.servSock = fd;
				sockets.push_back(socks);
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
		kevent(kq, &evSet, 1, NULL, 0, NULL);
		std::cout << getTime() << GREEN BOLD "Client #" << newClient << " connected" NC << std::endl;
	}
}

void	cleanServer(int kq, std::vector<socketServ> & sockets)
{
	for (std::vector<socketServ>::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		std::vector<int>	cliVec = (*itS).clientSock;
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
		std::vector<int>	cliVec = (*itS).clientSock;
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
	kevent(kq, &evSet, 1, NULL, 0, NULL);
	close(fd);
	std::cout << getTime() << PURPLE BOLD "Client #" << fd << " disconnected" NC << std::endl;
}

void	readFromSocket(int kq, int clientSocket, std::map<int, mssg> & m, std::vector<socketServ> & sockets)
{
	char		buffer[BUFFER_SIZE + 1];
	Request &	currentReq = m[clientSocket].req;
	Server &	currentServ = getSocketServ(clientSocket, sockets).serv;

	int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
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
	std::string	remainder;
	struct kevent	evSet[2];

	if (m.req.getState() == __FINISHED__)
	{
		/* std::ofstream outfile("test_img.mov", std::ios::binary);
		outfile.write(m.req.getBodyMssg().data(), m.req.getBodyMssg().size()); */
		ResponseGen	res(m.req, getSocketServ(clientSocket, sockets).serv);
		if (m.req.getErrorCode() >= HTTP_ERROR_START)
			m.closeOnEnd = true;
		else
			m.closeOnEnd = false;
		m.res = res.DoResponse().generateResponse();
		EV_SET(&evSet[0], clientSocket, EVFILT_READ, EV_DELETE, 0, 0, 0);
		EV_SET(&evSet[1], clientSocket, EVFILT_WRITE, EV_ADD, 0, 0, 0);
		kevent(kq, evSet, 2, 0, 0, 0);		
		remainder = m.req.getRemainder();
		m.req = Request();
		m.req.setRemainder(remainder);
	}
}

void	manageResponse(int clientSocket, int kq, std::vector<socketServ> & sockets, std::map<int, mssg> & m)
{
	mssg &			message = m[clientSocket];
	std::string		buff;
	struct kevent	evSet[2];

	buff = message.res.substr(0, BUFFER_SIZE);
	message.res = message.res.substr(buff.size(), std::string::npos);
	std::cout << getTime() << YELLOW BOLD "Sending data to client #" << clientSocket << "..." << NC << std::endl;
	send(clientSocket, buff.data(), buff.size(), 0);
	if (message.res.empty())
	{
		std::cout << getTime() << GREEN BOLD "Request served successfully to client #" << clientSocket << NC << std::endl;
		EV_SET(&evSet[0], clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
		EV_SET(&evSet[1], clientSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
		kevent(kq, evSet, 2, 0, 0, 0);
		message.res.clear();
		if (message.closeOnEnd)
			disconnectClient(kq, clientSocket, sockets, m);
	}
}

void	updateTimers(int kq, std::map<int, mssg> & m, std::vector<socketServ> & sockets)
{
	for (std::map<int, mssg>::iterator it = m.begin(); it != m.end(); it++)
	{
		std::pair<int, mssg>	currentM = (*it);
		currentM.second.timeout += TIMER_LOOP_MS;
		if (currentM.second.timeout > REQ_TIMEOUT_MS)
		{
			
		}
	}
	(void)kq;
	(void)m;
	(void)sockets;
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
			std::cerr << RED BOLD << strerror(errno) << NC << std::endl;
			break ;
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
		throw std::runtime_error("Error creating kqueue()");
	}
	for (std::vector<socketServ>::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		struct kevent	sEvent;
		
		EV_SET(&sEvent, (*itS).servSock, EVFILT_READ, EV_ADD, 0, 0, 0);
		evSet.push_back(sEvent);
	}
	EV_SET(&tEvent, TIMER_EV_IDENT, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, TIMER_LOOP_MS, 0);
	evSet.push_back(tEvent);
	if (kevent(kq, evSet.data(), evSet.size(), NULL, 0, NULL) == -1)
	{
		throw std::runtime_error("Error calling kevent()");
	}
	runEventLoop(kq, sockets, evSet.size());
}