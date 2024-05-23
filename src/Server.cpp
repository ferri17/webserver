
#include "Server.hpp"

Server::Server( void )
{
	_client_max_body_size = -1;
}

void Server::initDef(void)
{
	_listen.push_back((t_listen){ "", 4242});
	_server_name.push_back("default.com");
	_client_max_body_size = 10000;
	_root = "./html";
	_upload_store = "/tmp";
	_error_page.insert(std::pair<int, std::string>(4242, "./html/error.html"));
	setDefLoc();
}

void Server::setDefLis(void)
{
	_listen.push_back((t_listen){ "", 4242});
}

void Server::setDefLoc(void)
{
	Location loca;

	loca.setAutoindex(1);
	loca.setClose(true);
	_locations.insert(std::pair<std::string, Location>("/", loca));
}

void Server::preparePages( void )
{
	std::map<int, std::string>::iterator itPag = _error_page.begin();

	for (; itPag != _error_page.end(); itPag++)
	{
		itPag->second = _root + itPag->second;
	}
	std::map<std::string, Location>::iterator itLoc = _locations.begin();

	for (; itLoc != _locations.end(); itLoc++)
	{
		itLoc->second.preparePages(_root);
	}
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
	//out << "Listen: " << nb.getListen()[0].ip << " " << nb.getListen()[0].port << std::endl;
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