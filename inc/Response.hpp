#pragma once

#include <unordered_map>
#include <iostream>

typedef struct	statusLine
{
	std::string	_protocolVersion;
	short int	_statusCode;
	std::string	_reasonPhrase;
}	statusLine;

class	Response
{
	private:
		statusLine										_statusInfo;
		std::unordered_map<std::string, std::string>	_headerField;
		std::string										_bodyMssg;
	public:
		std::string	createResponse(void);
};