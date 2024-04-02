
#include "Server.hpp"

Server::Server( void ) {}

void Server::initDef(void)
{
	listen = 4242;
	server_name.push_back("default.com");
	client_max_body_size = 1024;
	root = ".";
	upload_store = "/tmp";
	error_page.insert(std::pair<int, std::string>(4242, "./html/error.html"));
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

void Server::pushServerName(std::string str)
{
	server_name.push_back(str);
}
void Server::pushErrorPage(std::pair<int, std::string> node)
{
	error_page.insert(node);
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