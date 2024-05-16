#include "Response.hpp"

Response::Response(void)
{
	statusLine	statLine = {"HTTP/1.1", 200, "OK"};
	this->setStatusLine(statLine);
}

std::string	Response::generateResponse(void)
{
	std::string	responseStr;

	responseStr += this->_statusLine._protocolVersion;
	responseStr += " ";
	responseStr += toString(this->_statusLine._statusCode);
	responseStr += " ";
	responseStr += this->_statusLine._reasonPhrase;
	responseStr += "\r\n";
	for (std::map<std::string, std::string>::iterator it = _headerField.begin(); it != _headerField.end(); it++)
	{
		responseStr += (*it).first;
		responseStr += ":";
		responseStr += (*it).second;
		responseStr += "\r\n";
	}
	responseStr += "\r\n";
	responseStr += this->_bodyMssg;
	return (responseStr);
}

Response &	Response::operator=(const Response & other)
{
	if (this != &other)
	{
		this->_statusLine = other._statusLine;
		this->_headerField = other._headerField;
		this->_bodyMssg = other._bodyMssg;
	}
	return (*this);
}



void	Response::setStatusLine(statusLine statusLine)
{
	this->_statusLine = statusLine;
}

void	Response::setHeaders(std::map<std::string, std::string> headers)
{
	this->_headerField = headers;
}

void	Response::addHeaderField(std::pair<std::string, std::string> headerField)
{
	this->_headerField.insert(headerField);
}

void	Response::setBody(std::string  body)
{
	this->_bodyMssg = body;
}
