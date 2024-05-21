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
