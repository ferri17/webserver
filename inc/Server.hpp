
#pragma once

#include <iostream>
#include <map>
#include <vector>

class Server
{
	private:
		int							listen;
		std::vector<std::string>	server_name;
		int							client_max_body_size;
		std::string					root;
		std::string					upload_store;
		std::map<int, std::string>	error_page;
	public:
		Server( void );
		Server( bool init);
		int getListen() const;
		std::vector<std::string> getServerName() const;
		int getClientMaxBodySize() const;
		std::string getRoot() const;
		std::string getUploadStore() const;
		std::map<int, std::string> getErrorPage() const;
		void setListen(int listen_);
		void setServerName(const std::vector<std::string> &serverName);
		void setClientMaxBodySize(int clientMaxBodySize);
		void setRoot(const std::string &root_);
		void setUploadStore(const std::string &uploadStore);
		void setErrorPage(const std::map<int, std::string> &errorPage);
		~Server( void );
};

std::ostream	&operator<< (std::ostream &out, const Server &nb);
