
#include "Server.hpp"

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
int Server::getClientMaxBodySize() const { return _client_max_body_size; }
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
void Server::setClientMaxBodySize(int clientMaxBodySize) { _client_max_body_size = clientMaxBodySize; }
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

void Server::startServ( void )
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket" << std::endl;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_listen);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error al vincular el socket" << std::endl;
        close(serverSocket);
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error al escuchar en el socket" << std::endl;
        close(serverSocket);
    }

	char buffer[1024] = { 0 }; 

	while (1)
	{
		int clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket == -1) {
			std::cerr << "Error al aceptar la conexión" << std::endl;
			close(serverSocket);
		}
		recv(clientSocket, buffer, sizeof(buffer), 0); 

		std::cout << buffer << std::endl;

		std::vector<std::string> lines = split(buffer, '\n');
		std::string filename;
		
		for (size_t i = 0; i < lines.size(); i++)
		{
			filename = checkLine(split(lines[i], ' '));
			if (!filename.empty())
				break;

		}
		std::cout << "hola" << std::endl;
		std::map<std::string ,Location>::iterator it = _locations.find(filename);

		if (it != _locations.end())
		{
			std::ifstream file;
			filename = "./html/" + filename;
			file.open(filename);
			if (!file.is_open()) {
				std::cerr << "Error al abrir el archivo HTML" << std::endl;
				break;
			}

			std::string html;

			std::getline(file, html, '\0');

			file.close();

			std::string html_hola = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + html;
			// Enviar HTML al cliente
			if (send(clientSocket, html_hola.c_str(), strlen(html_hola.c_str()), 0) == -1) {
				std::cerr << "Error al enviar HTML al cliente" << std::endl;
				break;
			}
		}
    	close(clientSocket);
	}
	
    
    close(serverSocket);

    // Cerrar los sockets
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