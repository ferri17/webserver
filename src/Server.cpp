
#include "Server.hpp"


Server::Server( void )
{
	listen = 4242;
	server_name.push_back("default.com");
	client_max_body_size = 1024;
	root = ".";
	upload_store = "/tmp";
	error_page.insert(std::pair<int, std::string>(4242, "./html/error.html"));
}

Server::Server( bool init)
{
	if (init == true)
		Server();
}

/// GET & SET
int Server::getListen() const { return listen; }
std::vector<std::string> Server::getServerName() const { return server_name; }
int Server::getClientMaxBodySize() const { return client_max_body_size; }
std::string Server::getRoot() const { return root; }
std::string Server::getUploadStore() const { return upload_store; }
std::map<int, std::string> Server::getErrorPage() const { return error_page; }
void Server::setListen(int listen_) { listen = listen_; }
void Server::setServerName(const std::vector<std::string> &serverName) { server_name = serverName; }
void Server::setClientMaxBodySize(int clientMaxBodySize) { client_max_body_size = clientMaxBodySize; }
void Server::setRoot(const std::string &root_) { root = root_; }
void Server::setUploadStore(const std::string &uploadStore) { upload_store = uploadStore; }
void Server::setErrorPage(const std::map<int, std::string> &errorPage) { error_page = errorPage; }

Server::~Server( void )
{

}

std::ostream	&operator<<(std::ostream &out, const Server &nb)
{
	out << "Listen: " << nb.getListen() <<std::endl;
	out << "Names: " << std::endl;
	std::vector<std::string> name = nb.getServerName();
	std::vector<std::string>::iterator it = name.begin();
	for (; it != name.end(); it++)
		out << "\t" << *it <<std::endl;
	out << "ClientMaxBodySize: " << nb.getClientMaxBodySize() <<std::endl;
	out << "Root: " << nb.getRoot() <<std::endl;
	out << "Error Page: " << std::endl;
	std::map<int, std::string> error = nb.getErrorPage();
	std::map<int, std::string>::iterator itE = error.begin();
	for (; itE != error.end(); itE++)
		out << "\t" << itE->second <<std::endl;
	return (out);
}