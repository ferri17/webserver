
#include <poll.h>
#include "Headers.hpp"
#include "Signals.hpp"
#include "StartServer.hpp"
#include <dirent.h>

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


Server &	getTargetServer(std::vector<std::pair<Server &, int> > sockets, int fdTarget)
{
	for (std::vector<std::pair<Server &, int> >::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		if ((*itS).second == fdTarget)
			return ((*itS).first);
	}
	return ((*sockets.begin()).first);
}

bool	isServerSocket(int fd, std::vector<socketServ> & sockets)
{
	for (std::vector<socketServ>::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		if ((*itS).servSock == fd)
			return (true);
	}
	return (false);
}
/*
	Returns socketServ reference that contains a specific socket, it doesn't
	take into account if it's a server or client socket
*/
socketServ &	getSocketServ(int targetFd, std::vector<socketServ> & sockets)
{
	for (std::vector<socketServ>::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		if ((*itS).servSock == targetFd)
			return (*itS);
		for (std::vector<int>::iterator itV = (*itS).clientSock.begin(); itV != (*itS).clientSock.end(); itV++)
		{
			if (*itV == targetFd)
				return (*itS);
		}
	}
	return (sockets.front());
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

void	disconnectClient(int kq, int fd, std::vector<socketServ> & sockets)
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
	EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, NULL ,0, NULL);
	kevent(kq, &evSet, 1, NULL, 0, NULL);
	close(fd);
	std::cout << getTime() << PURPLE BOLD "Client #" << fd << " disconnected" NC << std::endl;
}

int	readFromSocket(int clientSocket, std::map<int, mssg> & mssg, std::vector<socketServ> & sockets)
{
	char		buffer[BUFFER_SIZE + 1];
	Request &	currentReq = mssg[clientSocket].req;
	Server &	currentServ = getSocketServ(clientSocket, sockets).serv;

	int bytes_read = recv(clientSocket, buffer, BUFFER_SIZE, 0);
	if (bytes_read < 0)
	{
		std::cerr << strerror(errno) << std::endl;
		return (1);
	}
	else if (bytes_read == 0)
		return (0);
	buffer[bytes_read] = '\0';
	currentReq.parseNewBuffer(buffer, currentServ.getClientMaxBodySize());
	std::cout << getTime() << BLUE BOLD "Reading data from client #" << clientSocket << "..." << NC << std::endl;
	return (0);
}

Response	generateResponse(Request & req, Server & serv)
{
	Response	res;

	std::string	str = "<!DOCTYPE html><html><head><title>Beautiful Website</title><style>body{font-family:sans-serif;margin:20px;background-color:#f0f0f0}header{background-color:#3498db;color:#fff;padding:10px}nav{background-color:#eee;padding:10px}nav ul{list-style:none;padding:0}nav li{display:inline-block;margin-right:20px}nav a{color:#333;text-decoration:none}main{padding:20px}main section{text-align:center}main img{width:50%;border-radius:5px}main p{font-size:18px}footer{background-color:#3498db;color:#fff;padding:10px;text-align:center}</style></head><body><header><h1>Welcome!</h1></header><nav><ul><li><a href=\"#\">Home</a></li><li><a href=\"#\">About</a></li><li><a href=\"#\">Contact</a></li></ul></nav><main><section><img src=\"banner.jpg\" alt=\"Banner Image\"><p>This is a beautiful website.</p></section></main><footer><p>&copy; 2024 Beautiful Website</p></footer></body></html>";
	res.setBody(str);
	res.addHeaderField(std::pair<std::string, std::string>("content-length", toString(str.length())));
	(void)serv;
	(void)req;
	return (res);
}


void	manageRequestState(mssg & message, int clientSocket, int kq, std::vector<socketServ> & sockets)
{
	std::string	remainder;
	struct kevent	evSet[2];

	if (message.req.getState() == __SUCCESFUL_PARSE__ || message.req.getState() == __UNSUCCESFUL_PARSE__)
	{
		if (message.req.getState() == __UNSUCCESFUL_PARSE__)
			std::cerr << RED BOLD << "Error parsing:"  << message.req.getErrorMessage() << NC << std::endl;
		message.res = generateResponse(message.req, getSocketServ(clientSocket, sockets).serv);
		EV_SET(&evSet[0], clientSocket, EVFILT_READ, EV_DELETE, 0, 0, 0);
		EV_SET(&evSet[1], clientSocket, EVFILT_WRITE, EV_ADD, 0, 0, 0);
		kevent(kq, evSet, 2, 0, 0, 0);		
		remainder = message.req.getRemainder();
		message.req = Request();
		message.req.setRemainder(remainder);
	}
}

void	manageResponse(mssg & message, int clientSocket, int kq)
{
	struct kevent evSet[2];

	std::cout << getTime() << YELLOW BOLD "Sending data to client #" << clientSocket << "..." << NC << std::endl;
	send(clientSocket, message.res.generateResponse().data(), message.res.generateResponse().size(), 0);
	EV_SET(&evSet[0], clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
	EV_SET(&evSet[1], clientSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
	kevent(kq, evSet, 2, 0, 0, 0);
	message.res = Response();
}



void	runEventLoop(int kq, std::vector<socketServ> & sockets, size_t size)
{
	std::map<int, mssg>			mssg;
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
			{
				addNewClient(kq, clientSocket, sockets);
			}
			else if (evList[i].flags & EV_EOF)
			{
				disconnectClient(kq, clientSocket, sockets);
			}
			else if (evList[i].filter == EVFILT_READ)
			{
				if (readFromSocket(clientSocket, mssg, sockets) != 0)
				{
					disconnectClient(kq, clientSocket, sockets);
				}
				manageRequestState(mssg[clientSocket], clientSocket, kq, sockets);
			}
			else if (evList[i].filter == EVFILT_WRITE)
			{
				manageResponse(mssg[clientSocket], clientSocket, kq);
			}
		}
	}
	cleanServer(kq, sockets);
}

void startServers(std::vector<Server> & s)
{
	int							kq;
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
	if (kevent(kq, evSet.data(), evSet.size(), NULL, 0, NULL) == -1)
	{
		throw std::runtime_error("Error calling kevent()");
	}
	runEventLoop(kq, sockets, evSet.size());
}