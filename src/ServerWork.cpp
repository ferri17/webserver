
#include "Server.hpp"
#include <poll.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Headers.hpp"
#include "Signals.hpp"
#include <dirent.h>
#include "Cgi.hpp"
#include "ResponseGen.hpp"

int initSocket( Server &s)
{ 
	int serverSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSockfd == -1)
		throw std::invalid_argument("Error creating socket");
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(s.getListen()[0].port);
	if (bind(serverSockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
        close(serverSockfd);
    	throw std::invalid_argument("Error al enlazar el socket a la dirección y puerto");
    }
	if (listen(serverSockfd, 5) == -1)
	{
        close(serverSockfd);
    	throw std::invalid_argument("Error al intentar escuchar por conexiones entrantes");
    }
	return (serverSockfd);
}


void startServ( Server &s )
{
    int serverSocket = initSocket(s);
	std::vector<pollfd> fds;

    fds.push_back(((pollfd){serverSocket, POLLIN, -1}));
	while (signaled == true)
	{
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret == -1)
		{
			std::cout << "BYEEE 😀" << std::endl;
            break;
		}
		if (fds[0].fd == serverSocket && fds[0].revents & POLLIN)
		{
			int clientSocketfd = accept(serverSocket, NULL, NULL);
			fds.push_back(((pollfd){clientSocketfd, POLLIN, 0}));
		}
		for	(size_t i = 1; i < fds.size(); i++)
		{
			char buffer[1028];
			if (fds[i].revents & POLLIN)
			{
				size_t readBytes = recv(fds[i].fd, buffer, sizeof(buffer),0);
				if (readBytes <= 0)
				{
					if (readBytes == 0)
						std::cout << "Cliente desconectado\n";
					else
						std::cerr << "Error al recibir datos del cliente\n";
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					--i;
				}
				else
				{
					std::cout << "=====================================================\n";
					std::cout << buffer << std::endl;
					std::cout << "=====================================================\n";
					Request req(buffer);
					ResponseGen gen(req, s);
					std::string response = gen.DoResponse();

					if (send(fds[i].fd, response.c_str(), response.size(), 0) == -1)
						std::cout << "Send Failure" << std::endl;
				}
			}
		}
    }
	std::vector<pollfd>::iterator itFd = fds.begin();
	for (;itFd != fds.end(); itFd++)
		close(itFd->fd);
	close(serverSocket);
}