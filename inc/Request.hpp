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
		requestLine							_requestLine;
		std::map<std::string, std::string>	_headerField;
		std::string							_bodyMssg;
		int									_errorCode;
		std::string							_errorMssg;

		static bool			isValidFieldName(std::string & str);
		static bool			isValidFieldValue(std::string & str);
		static std::string	cleanOWS(std::string str);
		void				skipLeadingGarbage(std::string & req);
		bool				parseRequestLine(std::string & requestLineStr);
		bool				checkHeaderFields(void);
		bool				parseHeaderFields(std::vector<std::string> & headerVec);
		bool				readBodyMessage(std::string & body);
	public:
		Request(const char * req);
		//Request(const Request & other);
		//Request &	operator=(const Request & other);
		//~Request(void);

		requestLine							getRequestLine(void) const;
		std::string							getMethod(void) const;
		std::string							getRequestTarget(void) const;
		std::string							getProtocolVersion(void) const;
		std::map<std::string, std::string>	getHeaderField(void) const;
		std::string							getBodyMssg(void) const;
		std::string							getErrorMessage(void) const;
		int									getErrorCode(void) const;
};

std::ostream &	operator<<(std::ostream &out, const Request &req);
