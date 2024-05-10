
#include "Server.hpp"
#include <poll.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Headers.hpp"
#include "Signals.hpp"
#include "StartServer.hpp"
#include <dirent.h>

/* IMPORTANT THINKS:
		- ERROR PAGE FROM ROOT (NOT DONE)
*/

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
		freeaddrinfo(addr);
		throw std::runtime_error(strerror(errno));
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
	socketServ				socks;
	std::vector<t_listen>	cListen;
	std::vector<socketServ>	sockets;

	for (std::vector<Server>::iterator itServ = s.begin(); itServ != s.end(); itServ++)
	{
		Server &	currentServ = (*itServ);

		socks.serv = currentServ;
		cListen = currentServ.getListen();
		std::cout << "server #" << itServ - s.begin() << "{";
		for (std::vector<t_listen>::iterator itListen = cListen.begin(); itListen != cListen.end(); itListen++)
		{
			int	fd = createNewSocket(*itListen);
			socks.servSock = fd;
			sockets.push_back(socks);
			std::cout << fd;
			if (itListen < cListen.end() - 1)
				std::cout << ",";
		}
		std::cout << "}" << std::endl;
	}
	return (sockets);
}

int createDirectory(Response &res, std::string dir)
{
    DIR *opened = opendir(dir.c_str());
	struct dirent *entry;
	std::string body;

	if (!opened)
		return (1);

	std::cout << "HIIHIHIHIHI" << std::endl;
	entry = readdir(opened);

	body = "<h1>Directory: " + dir + "</h1>";
	while(entry)
	{
		body += "<li>";
		body += entry->d_name;
		body += "</li>\n";
		entry = readdir(opened);
	}
	closedir(opened);
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(body.size())));
	res.setBody(body);
	return(0);
}

int createResponseImage( std::string fileToOpen, Response &res)
{
	std::ifstream fileicon(fileToOpen, std::ios::binary);

	if (!fileicon.is_open())
		return (1);

	std::string html;
	char c;

	while (fileicon.get(c))
		html += c;
	fileicon.close();
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "image/x-icon"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
	res.setBody(html);
	return (0);
}

int createResponseHtml( std::string fileToOpen, Response &res)
{
	std::ifstream file(fileToOpen);

	if (!file.is_open())
		return (1);
	std::string html;
	std::cout << fileToOpen << std::endl;

	getline(file, html, '\0');
	file.close();
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
	res.setBody(html);
	return (0);
}

int createResponseError( Response &res, int codeError, std::map<int, std::string> errorPageServ)
{
	res.setStatusLine((statusLine){"HTTP/1.1", codeError, ERROR_MESSAGE(codeError)});
	if (!errorPageServ.empty() && errorPageServ.find(codeError) != errorPageServ.end())
	{
		if (createResponseHtml(errorPageServ[codeError], res) == 0)
			return (0);
	}
	std::string body;

	body += "<h1 style=\"text-align: center;\">" + toString(codeError) + " " + ERROR_MESSAGE(codeError) + "</h1>";
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(body.size())));
	res.setBody(body);
	return (0);
}

int createResponseError( Response &res, int codeError, std::map<int, std::string> errorPageServ, std::map<int, std::string> errorPageLoc)
{
	res.setStatusLine((statusLine){"HTTP/1.1", codeError, ERROR_MESSAGE(codeError)});
	if (!errorPageLoc.empty() && errorPageLoc.find(codeError) != errorPageLoc.end())
	{
		if (createResponseHtml(errorPageLoc[codeError], res) == 0)
			return (0);
	}
	else if (!errorPageServ.empty() && errorPageServ.find(codeError) != errorPageServ.end())
	{
		if (createResponseHtml(errorPageServ[codeError], res) == 0)
			return (0);
	}
	std::string body;

	body += "<h1 style=\"text-align: center;\">" + toString(codeError) + " " + ERROR_MESSAGE(codeError) + "</h1>";
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(body.size())));
	res.setBody(body);
	return (0);
}

int test(Request req)
{
	std::map<std::string, std::string> headers = req.getHeaderField();

	std::string acceptTypes = headers["accept"];

	if (acceptTypes.find("text/html") != acceptTypes.npos)
		return (1);
	else if (acceptTypes.find("image/") != acceptTypes.npos)
		return (2);
	else if (acceptTypes.find("*/*") != acceptTypes.npos)
		return (1);
	return (0);
}

int comparePratial(std::string src, std::string find)
{
	int i = 0;

	for (; src[i] && find[i]; i++)
	{
		if (src[i] != find[i])
			return (i);
	}
	return (i);
}

std::string partialFind(std::map<std::string, Location> loc, std::string reqTarget)
{
	std::map<std::string, Location>::iterator itLoc = loc.begin();

	(void)reqTarget;
	for (; itLoc != loc.end(); itLoc++)
	{
		int i = comparePratial(itLoc->first, reqTarget);
		if (itLoc->first[i] == '/')
			return (itLoc->first);
	}
	return ("");
}

std::string absolutFind(std::map<std::string, Location> loc, std::string reqTarget)
{
	std::map<std::string, Location>::iterator itLoc = loc.find(reqTarget);
	if (itLoc == loc.end())
		return ("");
	return (itLoc->first);
}

std::pair<std::string, std::string> locFind(std::map<std::string, Location> loc, std::string reqTarget)
{
	if (reqTarget[reqTarget.size() - 1] == '/')
		reqTarget.erase(reqTarget.size() - 1);
	std::string	test = absolutFind(loc, reqTarget);
	if (test.empty())
		test = partialFind(loc, reqTarget);
	if (test.empty())
	{
		std::vector<std::string> splited = split(reqTarget, '/');
		if (splited.size() == 0)
			return(std::pair<std::string, std::string>(test, ""));
		std::string newTarget = reqTarget;
		int i = newTarget.size();

		newTarget.erase(i - splited[splited.size() - 1].size() - 1, i);

		test = absolutFind(loc, newTarget);
		if (test.empty())
			return (std::pair<std::string, std::string>(test, ""));
		std::cout << test << std::endl;
		return(std::pair<std::string, std::string>(test, splited[splited.size() - 1]));
	}
	return (std::pair<std::string, std::string>(test, ""));
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
		std::cerr << "Error on accept()" << std::endl;
	else
	{
		socketServ & tmpServ = getSocketServ(targetSock, sockets);
		tmpServ.clientSock.push_back(newClient);
		EV_SET(&evSet, newClient, EVFILT_READ, EV_ADD, 0, 0, NULL);
		kevent(kq, &evSet, 1, NULL, 0, NULL);
	/* 	std::string mssg = "## hello new client ##\n";
		send(newClient, mssg.data(), mssg.size(), 0); */
	}
}

void	cleanServer(int kq, std::vector<std::pair<Server &, int> > & localSockets, std::vector<int> & clientSockets)
{
	for (std::vector<int>::iterator itV = clientSockets.begin(); itV != clientSockets.end(); itV++)
	{
		std::cout << "closing client socket " << (*itV) << std::endl;
		if (*itV > 0)
			close(*itV);
	}
	for (std::vector<std::pair<Server &, int> >::iterator itS = localSockets.begin(); itS != localSockets.end(); itS++)
	{
		std::cout << "closing server socket " << (*itS).second << std::endl;
		if ((*itS).second > 0)
			close((*itS).second);
	}
	if (kq > 0)
		close(kq);
	std::cout << "out" << std::endl;
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
	std::cout << "Client with fd " << fd << " disconnected." << std::endl;
}

Response	readFromSocket(int clientSocket, std::vector<socketServ> & sockets)
{
	char buffer[BUFFER_SIZE];
	int bytes_read = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytes_read > 0)
		buffer[bytes_read] = 0;
	else
		std::cerr << "Error recv returned -1" << std::endl;
	std::cout << "Listening from client #" << clientSocket << "#" << std::endl;
	std::cout << buffer << std::endl;
	Request	req(buffer);
	if (req.getErrorCode())
	{
		std::cerr << "Error parsing request: " << req.getErrorMessage() << std::endl;
	}
	Response	res;

	std::string mssg = "<!DOCTYPE html><html><body><h1>Hey</h1></body></html>";
	res.setBody(mssg);
	res.addHeaderField(std::pair<std::string, std::string>("content-length", toString(mssg.size())));
	(void)sockets;
	return (res);
}

void	runEventLoop(int kq, std::vector<socketServ> & sockets, size_t size)
{
	Response					res;
	struct kevent				evSet;
	std::vector<struct kevent>	evList(size);
	int							nbEvents;

	while (signaled)
	{
		if ((nbEvents = kevent(kq, NULL, 0, evList.data(), size, NULL)) < 0)
		{
			std::cerr << strerror(errno) << std::endl;
			signaled = false;
		}
		for (int i = 0; i < nbEvents; i++)
		{
			if (isServerSocket(evList[i].ident, sockets))
			{
				addNewClient(kq, evList[i].ident, sockets);
			}
			else if (evList[i].flags & EV_EOF)
			{
				disconnectClient(kq, evList[i].ident, sockets);
			}
			else if (evList[i].filter == EVFILT_READ)
			{
				//read100bytesMessageClient();

				// Generate response and store it

				//Add kevent with EVFILT_WRITE on client socket
				res = readFromSocket(evList[i].ident, sockets);
				EV_SET(&evSet, evList[i].ident, EVFILT_READ, EV_DELETE, 0, 0, 0);
				kevent(kq, &evSet, 1, 0, 0, 0);
				EV_SET(&evSet, evList[i].ident, EVFILT_WRITE, EV_ADD, 0, 0, 0);
				kevent(kq, &evSet, 1, 0, 0, 0);
			}
			else if (evList[i].filter == EVFILT_WRITE)
			{
				std::cout << "Writing to client #" << evList[i].ident << "#" << std::endl;
				std::cout << res.generateResponse() << std::endl;
				send(evList[i].ident, res.generateResponse().data(), res.generateResponse().size(), 0);
				EV_SET(&evSet, evList[i].ident, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
				kevent(kq, &evSet, 1, 0, 0, 0);
				EV_SET(&evSet, evList[i].ident, EVFILT_READ, EV_ADD, 0, 0, 0);
				kevent(kq, &evSet, 1, 0, 0, 0);
			}

		}
	}
	//cleanServer(kq, sockets, clientSockets);
}

void startServers(std::vector<Server> & s)
{
	int							kq;
	std::vector<socketServ>		sockets;
	std::vector<struct kevent>	evSet;

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