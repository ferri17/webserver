
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
	int				localSocket, errGai;
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

std::vector<std::pair<Server &, int > >	initSockets(std::vector<Server> & s)
{
	std::vector<t_listen>									cListen;
	std::vector<std::pair<Server &, int > >	sockets;

	for (std::vector<Server>::iterator itServ = s.begin(); itServ != s.end(); itServ++)
	{
		Server &	currentServ = (*itServ);

		cListen = currentServ.getListen();
		std::cout << "server #" << itServ - s.begin() << "{";
		for (std::vector<t_listen>::iterator itListen = cListen.begin(); itListen != cListen.end(); itListen++)
		{
			int	sock = createNewSocket(*itListen);
			sockets.push_back(std::pair<Server &, int>(currentServ, sock));
			std::cout << sock;
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

bool	isClientSocket(int fd, std::vector<std::pair<Server &, int> > & sockets)
{
	for (std::vector<std::pair<Server &, int> >::iterator itS = sockets.begin(); itS != sockets.end(); itS++)
	{
		if ((*itS).second == fd)
			return (true);
	}
	return (false);
}

void	runEventLoop(int kq, std::vector<std::pair<Server &, int> > localSockets, size_t size)
{
	struct kevent				evSet;
	struct sockaddr_in			addrCl;
	socklen_t addrLenCl = sizeof(addrCl);
	std::vector<struct kevent>	evList(size);
	int							nbEvents, clientSocket;

	while (signaled)
	{
		nbEvents = kevent(kq, NULL, 0, evList.data(), size, NULL);
		for (int i = 0; i < nbEvents; i++)
		{
			if (isClientSocket(evList[i].ident, localSockets))
			{
				if ((clientSocket = accept(evList[i].ident, (sockaddr *)&addrCl, &addrLenCl)) < 0)
					std::cerr << "Error on accept()" << std::endl;
				EV_SET(&evSet, clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
				kevent(kq, &evSet, 1, NULL, 0, NULL);
				std::cout << "client found" << std::endl;
				std::string mssg = "echo";
				send(clientSocket, mssg.data(), mssg.size(), 0);
				close(clientSocket);
			}
		}
	}
}


/* void	runEventLoop(int kq, std::vector<std::pair<Server &, int> > localSockets)
{
	struct kevent			evSet;
	struct kevent			evList[2];
	struct sockaddr_storage	addr;
	socklen_t				socklen = sizeof(addr);

	while (1) {
		int num_events = kevent(kq, NULL, 0, evList, 2, NULL);
		for (int i = 0; i < num_events; i++) {
            // receive new connection
            if (evList[i].ident == (uintptr_t)local_s) {
                int fd = accept(evList[i].ident, (struct sockaddr *) &addr, &socklen);
                if (conn_add(fd) == 0) {
                    EV_SET(&evSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(kq, &evSet, 1, NULL, 0, NULL);
                    send_welcome_msg(fd);
                } else {
                    printf("connection refused.\n");
                    close(fd);
                }
            } // client disconnected
            else if (evList[i].flags & EV_EOF) {
                int fd = evList[i].ident;
                printf("client #%d disconnected.\n", get_conn(fd));
                EV_SET(&evSet, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                kevent(kq, &evSet, 1, NULL, 0, NULL);
                conn_del(fd);
            } // read message from client
            else if (evList[i].filter == EVFILT_READ) {
                recv_msg(evList[i].ident);
            }
		}
    }
} */


void startServers(std::vector<Server> & s)
{
	int										kq;
	std::vector<std::pair<Server &, int> >	localSockets;
	std::vector<struct kevent>				evSet;

	localSockets = initSockets(s);
	if ((kq = kqueue()) == -1)
	{
		throw std::runtime_error("Error creating kqueue()");
	}
	for (std::vector<std::pair<Server &, int> >::iterator itS = localSockets.begin(); itS != localSockets.end(); itS++)
	{
		struct kevent	sEvent;
		
		EV_SET(&sEvent, (*itS).second, EVFILT_READ, EV_ADD, 0, 0, 0);
		evSet.push_back(sEvent);
	}
	if (kevent(kq, evSet.data(), evSet.size(), NULL, 0, NULL) == -1)
	{
		throw std::runtime_error("Error calling kevent()");
	}
	runEventLoop(kq, localSockets, evSet.size());
}