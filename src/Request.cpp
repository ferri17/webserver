#include "Request.hpp"

Request::Request(void)
{
	this->_errorCode = 0;
}

/*
	Remove all CRLF "\r\n" before the request line, it can also remove
	LF "\n", it doesn't accept any combination like CRLF + LF.

	RFC 9112: "In the interest of robustness, a server that is expecting 
	to receive and parse a request-line SHOULD ignore at least one empty 
	line (CRLF) received prior to the request-line."

*/
void	Request::skipLeadingGarbage(std::string & request)
{
	size_t pos = 0;

	// Find position after last LF
	if (request.at(0) == LF)
	{
		while (pos < request.length() && request.at(pos) == LF)
			pos++;
	}
	// Find position after last CRLF
	else if (request.at(0) == CR)
	{
		while ((pos + 1) < request.length() && request.at(pos) == CR && request.at(pos + 1) == LF)
				pos += 2;
	}
	// Trim string to the first character of the request-line
	request = request.substr(pos);
}

/*
	Parse the request syntax according to HTTP/1.1 as defined in RFC 9112.

	RFC 9112:"The normal procedure for parsing an HTTP message is to read 
	the start-line into a structure, read each header field line into a hash 
	table by field name until the empty line, and then use the parsed data to 
	determine if a message body is expected. If a message body has been 
	indicated, then it is read as a stream until an amount of octets equal to 
	the message body length is read or the connection is closed."
*/
void	Request::parseRequest(char * req)
{
	std::string	request(req);
	
	// Makes sure request is not empty, if it's empty it sets _errorCode to 400
	if (request.empty())
	{
		this->_errorCode = BAD_REQUEST;
		return ;
	}
	// Remove leading CRLF or LF
	this->skipLeadingGarbage(request);
	if (this->_errorCode)
		return ;
	
	std::vector<std::string>	requestSplit = split_r(request, LF);
	for (std::vector<std::string>::iterator it = requestSplit.begin(); it != requestSplit.end(); it++)
	{
		if (!(*it).empty() && (*it).at((*it).length() - 1) == CR)
			(*it).erase((*it).length() - 1, std::string::npos);
		std::cout << *it << "@" << std::endl;
	}
}