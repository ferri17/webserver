#pragma once

#include "Server.hpp"
#include "Utils.hpp"
#include "ErrorsHttp.hpp"
#include <unordered_map>
#include <iostream>
#include <ctype.h>

typedef struct	requestLine
{
	std::string	_method;
	std::string	_requestTarget;
	std::string	_protocolVersion;
}	requestLine;

class	Request
{
	private:

		void	skipLeadingGarbage(std::string & req);
		bool	parseRequestLine(std::string & requestLineStr);
		bool	parseHeaderFields(std::vector<std::string> & headerVec);
	public:

		//temporary public to find errors
		requestLine										_requestLine;
		std::unordered_map<std::string, std::string>	_headerField;
		std::string										_bodyMssg;
		short int										_errorCode;
		//


		Request(const char * req);
		//Request(const Request & other);
		//Request &	operator=(const Request & other);
		//~Request(void);
};

std::ostream	&operator<<(std::ostream &out, const Request &req);
