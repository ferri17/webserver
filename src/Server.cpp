
#include "Server.hpp"
#include <poll.h>

Server::Server( void )
{
	_listen = -1;
	_client_max_body_size = -1;
}

void Server::initDef(void)
{
	_listen = 4242;
	_server_name.push_back("default.com");
	_client_max_body_size = 1024;
	_root = ".";
	_upload_store = "/tmp";
	_error_page.insert(std::pair<int, std::string>(4242, "./html/error.html"));
}

/// GET & SET
int Server::getListen() const { return _listen; }
std::vector<std::string> Server::getServerName() const { return _server_name; }
long Server::getClientMaxBodySize() const { return _client_max_body_size; }
std::string Server::getRoot() const { return _root; }
std::string Server::getUploadStore() const { return _upload_store; }
std::map<int, std::string> Server::getErrorPage() const { return _error_page; }

Location &Server::getLocations(std::string dir)
{
	return (_locations[dir]);
}
std::map<std::string, Location> &Server::getLocations()
{
	return (_locations);
}

void Server::setLocations(const std::map<std::string, Location> &locations) { _locations = locations; }
void Server::setListen(int listen_) { _listen = listen_; }
void Server::setServerName(const std::vector<std::string> &serverName) { _server_name = serverName; }
void Server::setClientMaxBodySize(long clientMaxBodySize) { _client_max_body_size = clientMaxBodySize; }
void Server::setRoot(const std::string &root) { _root = root; }
void Server::setUploadStore(const std::string &uploadStore) { _upload_store = uploadStore; }
void Server::setErrorPage(const std::map<int, std::string> &errorPage) { _error_page = errorPage; }
void Server::pushLoactions(const std::pair<std::string, Location> node) { _locations.insert(node); }

void Server::pushServerName(std::string str)
{
	_server_name.push_back(str);
}

void Server::pushErrorPage(std::pair<int, std::string> node)
{
	_error_page.insert(node);
}
	
std::string checkLine(std::vector<std::string> line)
{
	if (line.size() == 3 && (line[0] == "GET" || line[0] == "POST"))
	{
		return (line[1]);
	}
	return (NULL);
}

int Server::initSocket()
{ 
	int serverSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSockfd == -1)
		throw std::invalid_argument("Error creating socket");
	
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(_listen);
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
        if (ret == -1) {
            std::cerr << "Error en poll()\n";
            break;
        }
		if (fds[0].fd == serverSocket && fds[0].revents & POLLIN)
		{
			std::cout << "HOLA" << std::endl;
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
					std::ifstream file;
					std::vector<std::string> test = split(buffer, '\n');
					std::string pag = checkLine(split(test[0], ' '));

					file.open("./html/index.html");

					std::string html;

					std::getline(file, html, '\0');

					file.close();

					std::string html_hola = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + html;
					// Enviar HTML al cliente
					if (send(fds[i].fd, html_hola.c_str(), strlen(html_hola.c_str()), 0) == -1) {
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
	_listen = -1;
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

std::ostream	&operator<<(std::ostream &out, const Server &nb)
{
	out << "Listen: " << nb.getListen() <<std::endl;
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