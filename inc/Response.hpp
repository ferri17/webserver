#pragma once

#include "Server.hpp"
#include "Utils.hpp"
#include "ErrorsHttp.hpp"
#include <map>
#include <iostream>
#include <ctype.h>
#include <string>

typedef struct	statusLine
{
	std::string	_protocolVersion;
	short int	_statusCode;
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
		//Response(const Response & other);
		//Response &	operator=(const Response & other);
		//~Response(void);
};

std::ostream &	operator<<(std::ostream &out, const Response &req);