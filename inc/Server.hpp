
#pragma once

#include "Utils.hpp"
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
		std::map<int, std::string> getErrorPage() const;
		void setListen(int listen_);
		void setServerName(const std::vector<std::string> &serverName);
		void setClientMaxBodySize(int clientMaxBodySize);
		void setRoot(const std::string &root);
		void setUploadStore(const std::string &uploadStore);
		void setErrorPage(const std::map<int, std::string> &errorPage);
		void pushServerName(std::string str);
		void pushErrorPage(std::pair<int, std::string> node);
		//END
		~Server( void );
};

std::ostream	&operator<< (std::ostream &out, const Server &nb);
