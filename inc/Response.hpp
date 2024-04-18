#pragma once

#include "Server.hpp"
#include "Utils.hpp"
#include "ErrorsHttp.hpp"
#include <map>
#include <iostream>
#include <ctype.h>
#include <string>
#include <sstream>
#include <string>

typedef struct	statusLine
{
	std::string	_protocolVersion;
	int			_statusCode;
	std::string	_reasonPhrase;
}	statusLine;

class	Response
{
	private:
		statusLine							_statusLine;
		std::map<std::string, std::string>	_headerField;
		std::string							_bodyMssg;
	public:
		Response(void);
		std::string	generateResponse(void);

		void		setStatusLine(statusLine statusLine);
		void		setHeaders(std::map<std::string, std::string> headers);
		void		addHeaderField(std::pair<std::string, std::string> headerField);
		void		setBody(std::string  body);
		//Response(const Response & other);
		//Response &	operator=(const Response & other);
		//~Response(void);
};

std::ostream &	operator<<(std::ostream &out, const Response &req);