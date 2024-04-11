#pragma once

#include "Server.hpp"
#include "Utils.hpp"
#include "ErrorsHttp.hpp"
#include <map>
#include <iostream>
#include <ctype.h>
#include <string>

typedef struct	requestLine
{
	std::string	_method;
	std::string	_requestTarget;
	std::string	_protocolVersion;
}	requestLine;

class	Request
{
	private:
		static bool			isValidFieldName(std::string & str);
		static bool			isValidFieldValue(std::string & str);
		static std::string	cleanOWS(std::string str);
		void				skipLeadingGarbage(std::string & req);
		bool				parseRequestLine(std::string & requestLineStr);
		bool				parseHeaderFields(std::vector<std::string> & headerVec);
		bool				readBodyMessage(std::vector<std::string> & bodyVec);
	public:

		//temporary public to find errors
		requestLine													_requestLine;
		std::map<std::string, std::string>	_headerField;
		std::string													_bodyMssg;
		short int													_errorCode;
		//


		Request(const char * req);
		//Request(const Request & other);
		//Request &	operator=(const Request & other);
		//~Request(void);
};

std::ostream &	operator<<(std::ostream &out, const Request &req);