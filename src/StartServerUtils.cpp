#include "Request.hpp"
#include "StartServer.hpp"

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

void	resolveBindingError(struct sockaddr_in & cAddress, std::vector<struct sockaddr_in> & addresses, std::vector<socketServ> & sockets, t_listen & cListen, Server &	cServ)
{
	for (size_t i = 0; i < addresses.size() && i < sockets.size(); i++)
	{
		if (!memcmp(&(cAddress.sin_addr),&(addresses[i].sin_addr), sizeof (in_addr)) 
			&& cAddress.sin_port == addresses[i].sin_port)
		{
			sockets.at(i).servers.push_back(cServ);
			return ;
		}
	}
	std::string	mssgErr = strerror(errno);
	mssgErr += ": ";
	mssgErr += cListen.ip;
	mssgErr += ":";
	mssgErr += toString(cListen.port);
	throw std::runtime_error(mssgErr);
}

Server &	resolveServerName(Request & req, std::vector<Server> & servers)
{
	std::map<std::string, std::string>	headers = req.getHeaderField();
	std::map<std::string, std::string>::iterator itHost = headers.find("host");

		std::cout << "serv name:" << servers.front().getServerName().front();
	if (servers.size() == 1)
	{
		return (servers.front());
	}
	if (itHost != headers.end())
	{
		std::string	targetServName = (*itHost).second;
		for (std::vector<Server>::iterator	itS = servers.begin(); itS != servers.end(); itS++)
		{
			std::vector<std::string>	servNameVec = (*itS).getServerName();
			for (std::vector<std::string>::iterator	itName = servNameVec.begin(); itName != servNameVec.end(); itName++)
			{
				if (targetServName == (*itName))
					return (*itS);
			}
		}
	}
	return (servers.front());
}


