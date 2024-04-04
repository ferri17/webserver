#pragma once

#include "Utils.hpp"
#include "ErrorsHttp.hpp"
#include <unordered_map>
#include <iostream>

#define CR '\r'
#define LF '\n'

typedef struct	requestLine
{
	std::string	_method;
	std::string	_requestTarget;
	std::string	_protocolVersion;
}	requestLine;

class	Request
{
	private:
		requestLine										_requestInfo;
		std::unordered_map<std::string, std::string>	_headerField;
		std::string										_bodyMssg;
		short int										_errorCode;

		void	skipLeadingGarbage(std::string & req);
	public:
		Request(void);
		void	parseRequest(char * request);
		//Request(const Request & other);
		//Request &	operator=(const Request & other);
		//~Request(void);
};