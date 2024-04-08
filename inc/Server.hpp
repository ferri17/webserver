
#pragma once

#include "Utils.hpp"
#include "Location.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> 

class Server
{
	private:
		int							_listen;
		std::vector<std::string>	_server_name;
		int							_client_max_body_size;
		std::string					_root;
		std::string					_upload_store;
		std::map<int, std::string>	_error_page;
		std::map<std::string, Location> _locations;
	public:
		Server( void );
		void initDef( void );
		void startServ( void );
		//GET & SET
		int getListen() const;
		std::vector<std::string> getServerName() const;
		int getClientMaxBodySize() const;
		std::string getRoot() const;
		std::string getUploadStore() const;
		Location &getLocations(std::string const dir);
		std::map<std::string, Location> &getLocations();
		std::map<int, std::string> getErrorPage() const;
		void setListen(int listen_);
		void setServerName(const std::vector<std::string> &serverName);
		void setClientMaxBodySize(int clientMaxBodySize);
		void setRoot(const std::string &root);
		void setUploadStore(const std::string &uploadStore);
		void setErrorPage(const std::map<int, std::string> &errorPage);
		void setLocations(const std::map<std::string, Location> &locations);
		void pushLoactions(const std::pair<std::string, Location> node);
		void pushServerName(std::string str);
		void pushErrorPage(std::pair<int, std::string> node);
		//END
		void clean( void );
		~Server( void );
};

std::ostream	&operator<< (std::ostream &out, const Server &nb);
