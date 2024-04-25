
#include "Server.hpp"
#include <poll.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Headers.hpp"
#include "Signals.hpp"
#include <dirent.h>

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

int createDirectory(Response &res, std::string dir)
{
	dir = "." + dir;
    DIR *opened = opendir(dir.c_str());
	struct dirent *entry;
	std::string body;

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
		return (0);

	std::string html;

	getline(fileicon, html);
	fileicon.close();
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "image/x-icon"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
	res.setBody(html);
	return (1);
}

int createResponseHtml( std::string fileToOpen, Response &res)
{
	std::ifstream file(fileToOpen);

	if (!file.is_open())
		return (0);

	std::string html;

	getline(file, html, '\0');
	file.close();
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
	res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
	res.setBody(html);
	return (1);
}

int createResponseError( Response &res, int codeError, std::map<int, std::string> errorPageServ, std::map<int, std::string> errorPageLoc)
{
	res.setStatusLine((statusLine){"HTTP/1.1", codeError, ERROR_MESSAGE(codeError)});
	if (errorPageLoc.find(codeError) != errorPageLoc.end())
	{
		createResponseHtml(errorPageLoc[codeError], res);	
	}
	else if (errorPageServ.find(codeError) != errorPageServ.end())
	{
		createResponseHtml(errorPageServ[codeError], res);
	}
	else
	{
		std::string body;

		body += "<h1 style=\"text-align: center;\">" + toString(codeError) + " " + ERROR_MESSAGE(codeError) + "</h1>";
		res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
		res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(body.size())));
		res.setBody(body);
	}
	return (0);
}

int test(Request req)
{
	std::map<std::string, std::string> headers = req.getHeaderField();

	std::string acceptTypes = headers["accept"];

	if (acceptTypes.find("text/html") != acceptTypes.npos)
		return (1);
	else if (acceptTypes.find("*/*") != acceptTypes.npos)
		return (1);
	else if (acceptTypes.find("image/") != acceptTypes.npos)
		return (2);
	return (0);
}

void startServ( Server &s )
{
    int serverSocket = initSocket(s);

	std::vector<pollfd> fds;
    fds.push_back(((pollfd){serverSocket, POLLIN, -1}));
	while (signaled == true)
	{
        int ret = poll(fds.data(), fds.size(), -1); // Espera indefinidamente
        if (ret == -1)
            break;
		if (fds[0].fd == serverSocket && fds[0].revents & POLLIN)
		{
			int clientSocketfd = accept(serverSocket, NULL, NULL);
			fds.push_back(((pollfd){clientSocketfd, POLLIN, 0}));
		}
		for	(size_t i = 1; i < fds.size(); i++)
		{
			char buffer[1024];
			std::cout << "ITER: " << i << std::endl;
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
					--i; // Ajustar el índice ya que se eliminó un elemento
				}
				else
				{
					buffer[readBytes] = '\0';
					std::cout << "==========================================================\n"; 
					std::cout << buffer << std::endl;
					std::cout << "==========================================================\n";
					Request req(buffer);
					if (req.getErrorCode() != 0)
					{
						std::cout <<  req.getErrorCode() << ": " << req.getErrorMessage() << std::endl;
						exit(0);
					}
					std::map<std::string, Location> loc = s.getLocations();

					std::map<std::string, Location>::iterator itLoc = loc.find(req.getRequestTarget());
					std::cout << req.getRequestTarget() << std::endl;
					Response res;

					if (itLoc == loc.end())
					{
						res.setStatusLine((statusLine){"HTTP/1.1", 404, "Page Not Found"});
						
						std::string fileToOpen = s.getRoot() + s.getErrorPage()[404];
						createResponseHtml(fileToOpen, res);
					}
					else if (!itLoc->second.getReturnPag().empty())
					{
						Location loca = itLoc->second;

						res.setStatusLine((statusLine){"HTTP/1.1", FOUND, ERROR_MESSAGE(FOUND)});
						res.addHeaderField(std::pair<std::string, std::string>(LOCATION, loca.getReturnPag()));
					}
					else
					{
						Location loca = itLoc->second;

						std::vector<std::string> indexs = loca.getIndex();
						std::string fileToOpen;
						std::vector<std::string>::iterator it = indexs.begin();
						for (; it != indexs.end(); it++)
						{
							fileToOpen = s.getRoot() + "/" + *it;
							if (access(fileToOpen.c_str(), F_OK | R_OK) == 0)
								break;
						}
						std::cout << fileToOpen << std::endl;
						if (it == indexs.end() && loca.getAutoindex() == true)
						{
							createDirectory(res, req.getRequestTarget());
						}
						if (it == indexs.end())
						{
							createResponseError(res, NOT_FOUND, s.getErrorPage(), loca.getErrorPage());
						}
						int type = test(req);
						switch (type)
						{
							case 1:
								createResponseHtml(fileToOpen, res);
								break;
							case 2:
								createResponseImage(fileToOpen, res);
								break;
							default:
								createResponseError(res, NOT_ACCEPTABLE, s.getErrorPage(), loca.getErrorPage());
								break;
						}
					}
					
					std::string response = res.generateResponse();
					if (send(fds[i].fd, response.c_str(), response.size(), 0) == -1) {
						std::cerr << "Error al enviar HTML al cliente" << std::endl;
						break;
					}
				}
			}
		}
    }
}