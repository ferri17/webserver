
#include "Server.hpp"
#include <poll.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Headers.hpp"
#include "Signals.hpp"
#include <dirent.h>

/* IMPORTANT THINKS:
		- ERROR PAGE FROM ROOT (NOT DONE)
*/


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
    DIR *opened = opendir(dir.c_str());
	struct dirent *entry;
	std::string body;

	if (!opened)
		return (1);

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

	getline(fileicon, html);////BAD FORMAT FORM BINARY FILES
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
	if (!errorPageLoc.empty() && errorPageLoc.find(codeError) != errorPageLoc.end())
	{
		if (createResponseHtml(errorPageLoc[codeError], res))
			return (0);
	}
	else if (!errorPageServ.empty() && errorPageServ.find(codeError) != errorPageServ.end())
	{
		if (createResponseHtml(errorPageServ[codeError], res))
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
		std::cout << itLoc->first << std::endl;
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

std::string locFind(std::map<std::string, Location> loc, std::string reqTarget)
{
	std::string	test = absolutFind(loc, reqTarget);
	if (test.empty())
		test = partialFind(loc, reqTarget);
	return (test);
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

					std::string nameLoc = locFind(loc, req.getRequestTarget());
					Response res;

					std::cout << "----------------" << nameLoc << "----------------"<< std::endl;
					if (nameLoc.empty())
					{
						/////// CARNE DE SEGFAULT
						res.setStatusLine((statusLine){"HTTP/1.1", 404, "Page Not Found"});
						
						std::string fileToOpen = s.getRoot() + s.getErrorPage()[404];
						createResponseHtml(fileToOpen, res);
					}
					else
					{

						std::map<std::string, Location>::iterator itLoc = loc.find(nameLoc);
						if (!itLoc->second.getReturnPag().empty())
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
									fileToOpen = *it;
									if (access((*it).c_str(), F_OK | R_OK) == 0)
										break;
								}
								if (it == indexs.end() && loca.getAutoindex() == true)
								{
									std::string dirToOpen;						
									if (loca.getRoot().empty())
										dirToOpen = nameLoc;
									else
										dirToOpen = loca.getRoot();
									if (createDirectory(res, dirToOpen))
										createResponseError(res, NOT_FOUND, s.getErrorPage(), loca.getErrorPage());
									
								}
								else if (it == indexs.end())
								{
									createResponseError(res, NOT_FOUND, s.getErrorPage(), loca.getErrorPage());
								}
								int type = test(req);
								std::cout << "_________________" << fileToOpen << "_________________"<< std::endl;
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