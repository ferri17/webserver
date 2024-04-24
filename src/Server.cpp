
#include "Server.hpp"
#include <poll.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Headers.hpp"

Server::Server( void )
{
	_client_max_body_size = -1;
}

void Server::initDef(void)
{
	_listen.push_back((t_listen){ "", 4242});
	_server_name.push_back("default.com");
	_client_max_body_size = 1024;
	_root = "./html";
	_upload_store = "/tmp";
	_error_page.insert(std::pair<int, std::string>(4242, "./html/error.html"));
}

/// GET & SET
std::vector<t_listen> Server::getListen() const { return _listen; }
std::vector<std::string> Server::getServerName() const { return _server_name; }
long Server::getClientMaxBodySize() const { return _client_max_body_size; }
std::string Server::getRoot() const { return _root; }
std::string Server::getUploadStore() const { return _upload_store; }
std::map<int, std::string> Server::getErrorPage() const { return _error_page; }
Location &Server::getLocations(std::string dir) { return (_locations[dir]); }
std::map<std::string, Location> &Server::getLocations() { return (_locations); }
void Server::setLocations(const std::map<std::string, Location> &locations) { _locations = locations; }
void Server::addListen(t_listen listen_) { _listen.push_back(listen_); }
void Server::setServerName(const std::vector<std::string> &serverName) { _server_name = serverName; }
void Server::setClientMaxBodySize(long clientMaxBodySize) { _client_max_body_size = clientMaxBodySize; }
void Server::setRoot(const std::string &root) { _root = root; }
void Server::setUploadStore(const std::string &uploadStore) { _upload_store = uploadStore; }
void Server::setErrorPage(const std::map<int, std::string> &errorPage) { _error_page = errorPage; }
void Server::pushLoactions(const std::pair<std::string, Location> node) { _locations.insert(node); }
void Server::pushServerName(std::string str) { _server_name.push_back(str); }
void Server::pushErrorPage(std::pair<int, std::string> node) { _error_page.insert(node); }

int Server::initSocket()
{ 
	int serverSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSockfd == -1)
		throw std::invalid_argument("Error creating socket");
	
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(_listen[0].port);
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

void Server::startServ( void )
{
    int serverSocket = initSocket();

	std::vector<pollfd> fds;
    fds.push_back(((pollfd){serverSocket, POLLIN, -1}));

	while (true)
	{
        int ret = poll(fds.data(), fds.size(), -1); // Espera indefinidamente
        if (ret == -1)
		{
            std::cerr << "Error en poll()\n";
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
					std::map<std::string, Location> loc = this->getLocations();

					std::map<std::string, Location>::iterator itLoc = loc.find(req.getRequestTarget());
					std::cout << req.getRequestTarget() << std::endl;
					Response res;

					if (itLoc == loc.end())
					{
						res.setStatusLine((statusLine){"HTTP/1.1", 404, "Page Not Found"});
						std::ifstream file("./html/error_pages/404.html");

						std::string html;

						getline(file, html, '\0');
						file.close();
						res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
						res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
						res.setBody(html);
					}
					else
					{
						Location loc = itLoc->second;

						std::vector<std::string> indexs = loc.getIndex();
						std::string fileToOpen;
						std::vector<std::string>::iterator it = indexs.begin();
						for (; it != indexs.end(); it++)
						{
							fileToOpen = _root + "/" + *it;
							std::cout << fileToOpen << std::endl;
							if (access(fileToOpen.c_str(), F_OK | R_OK))
								break;
						}
						if (fileToOpen.empty())
						{
							res.setStatusLine((statusLine){"HTTP/1.1", 404, "Page Not Found"});
						}
						else if (fileToOpen.find("favicon.ico") != fileToOpen.npos)
						{
							std::ifstream fileicon(fileToOpen, std::ios::binary);

							std::string html;

							getline(fileicon, html);
							fileicon.close();
							res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "image/x-icon"));
							res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
							res.setBody(html);
						}
						else
						{
							std::ifstream file(fileToOpen);

							std::string html;

							getline(file, html, '\0');
							file.close();
							res.addHeaderField(std::pair<std::string, std::string>(CONTENT_TYPE, "text/html"));
							res.addHeaderField(std::pair<std::string, std::string>(CONTENT_LENGTH, toString(html.size())));
							res.setBody(html);
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

void Server::clean( void )
{
	_listen.clear();
	_client_max_body_size = -1;
	_root.clear();
	_upload_store.clear();
	_error_page.clear();
	_server_name.clear();
	_locations.clear();
}


Server::~Server( void )
{

}

bool	Server::isServerMethod(std::string & method)
{
	std::string	validMethods[4] = {"GET", "HEAD", "POST", "DELETE"};

	for (size_t i = 0; i < 4; i++)
		if (method == validMethods[i])
			return (true);
	return (false);
}


std::ostream	&operator<<(std::ostream &out, const Server &nb)
{
	out << "Listen: " << nb.getListen()[0].ip << " " << nb.getListen()[0].port << std::endl;
	out << "ClientMaxBodySize: " << nb.getClientMaxBodySize() <<std::endl;
	out << "Root: " << nb.getRoot() << std::endl;
	out << "Upoad Store: "  << nb.getUploadStore() << std::endl;
	std::vector<std::string> name = nb.getServerName();
	std::vector<std::string>::iterator it = name.begin();
	out << "Names: " << std::endl;
	for (; it != name.end(); it++)
		out << "\t- " << *it <<std::endl;
	out << "Error Page: " << std::endl;
	std::map<int, std::string> error = nb.getErrorPage();
	std::map<int, std::string>::iterator itE = error.begin();
	for (; itE != error.end(); itE++)
		out << "\t- " << itE->second <<std::endl;
	return (out);
}