#pragma once

#include <iostream>
#include <map>
#include <vector>

typedef struct cgi_type
{
	std::string type;
	std::string file;
} t_cgi_type;

class Location
{
	private:
			int _autoindex;
			std::map<int, std::string> _error_page;
			std::vector<std::string> _allow_methods;
			std::string _upload_store;
			std::vector<t_cgi_type> _cgi;
			std::string _returnPag;
			std::vector<std::string> _index;
			std::string _root;
			bool close;
	public:
		Location( void );
		~Location( void );
		int checkMethod( std::string meth);
		void preparePages( std::string rootServ);
		void setAutoindex(int autoindex);
		void setErrorPage(const std::map<int, std::string> &error_page);
		void setCgi(t_cgi_type cgi);
		void setUploadStore(const std::string &upload_store);
		void setReturnPag(const std::string &returnPag);
		void setIndex(const std::vector<std::string> &index);
		std::string getRoot() const;
		void setRoot(std::string root);
		int getAutoindex() const;
		std::map<int, std::string> getErrorPage() const;
		std::vector<std::string > getAllowMethods() const;
		std::string getUploadStore() const;
		std::vector<t_cgi_type> getCgi() const;
		std::string getReturnPag() const;
		std::vector<std::string> getIndex() const;
		void pushErrorPage(std::pair<int, std::string> node);
		void pushAllowMethods(std::string methods);
		void setClose(bool close_);
		bool getClose() const;
		void pushIndex(std::string index);
};

std::ostream	&operator<< (std::ostream &out, const Location &nb);
