#include "Request.hpp"

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

	if (request.empty())
		return ;
	// Find position after last LF or position after last CRLF
	if (request.at(0) == LF)
	{
		while (pos < request.length() && request.at(pos) == LF)
			pos++;
	}
	else if (request.at(0) == CR)
	{
		while ((pos + 1) < request.length() && request.at(pos) == CR && request.at(pos + 1) == LF)
				pos += 2;
	}

	// Trim string to the first character of the request-line
	request = request.substr(pos);
}

/*

*/
bool	Request::parseHeaderFields(std::vector<std::string> & headerVec)
{
	for (std::vector<std::string>::iterator it = headerVec.begin(); it != headerVec.end(); it++)
	{
		std::string	headerLine = *it;
	}
	return (true);
}


/*
	Parse request-line following RFC 9112: 
	
	"A request-line begins with a method token, 
	followed by a single space (SP), the request-target, and another single space (SP), 
	and ends with the protocol version.
	
	request-line   = method SP request-target SP HTTP-version"
*/
bool	Request::parseRequestLine(std::string & requestLineStr)
{
	// Check basic syntax errors on request-line
	if (requestLineStr.empty() || requestLineStr.at(requestLineStr.length() - 1) == SP)
		return (this->_errorCode = BAD_REQUEST, false);

	// Check request-line length against server max length accepted
	if (requestLineStr.length() > MAX_LEN_REQUEST_LINE)
		return (this->_errorCode = BAD_REQUEST, false);

	// Split request-line in strings separated by SP
	std::vector<std::string>	requestLineVec = split_r(requestLineStr, SP);

	// Check request-line consists of 3 space separated strings, if not return 400
	if (requestLineVec.size() != 3)
		return (this->_errorCode = BAD_REQUEST, false);
	std::string	method = requestLineVec[0];
	std::string	reqTarget = requestLineVec[1];
	std::string	protocol = requestLineVec[2];

	// Check if method token is implemented in the server and return error if needed
	if (Server::isServerMethod(method))
		this->_requestLine._method = method;
	else
	{
		size_t	i = 0;
		while (i < method.length() && isUsAscii(method.at(i)) && !std::isspace(method.at(i)))
			i++;
		if (i == method.length())
			this->_errorCode = NOT_IMPLEMENTED;
		else
			this->_errorCode = BAD_REQUEST;
		return (false);
	}

	// Check if request-target syntax is valid
	if (!reqTarget.empty())
	{
		size_t	i = 0;
		while (i < reqTarget.length() && isUsAscii(reqTarget.at(i)) && !std::isspace(reqTarget.at(i)))
			i++;
		if (i != reqTarget.length())
			return (this->_errorCode = BAD_REQUEST, false);
		this->_requestLine._requestTarget = reqTarget;
	}
	else
		return (this->_errorCode = BAD_REQUEST, false);

	// Check if HTTP-version is correct
	if (protocol == "HTTP/1.1")
		this->_requestLine._protocolVersion = protocol;
	else
		return (this->_errorCode = HTTP_VERSION_NOT_SUPPORTED, false);
	
	return (true);
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
Request::Request(const char * req)
{
	std::string	request(req);
	
	// Init request error code to 0
	this->_errorCode = 0;

	// Remove leading CRLF or LF
	this->skipLeadingGarbage(request);
	// Makes sure request is not empty, if it's empty it sets _errorCode to 400
	if (request.empty())
	{
		this->_errorCode = BAD_REQUEST;
		return ;
	}
	// Split message in vectors using LF as delimitor
	std::vector<std::string>	reqSplit = split_r(request, LF);
	// If last char of each vector is CR we erase it from the chain
	for (std::vector<std::string>::iterator it = reqSplit.begin(); it != reqSplit.end(); it++)
	{
		std::string &	line = (*it);
		if (!line.empty() && line.at(line.length() - 1) == CR)
			line.erase(line.length() - 1);
	}
	// Set iterators to split request into request-line, headers and body message
	// This allows us to generate vectors only containing the info we need on each step
	std::vector<std::string>::iterator	reqLineIt = reqSplit.begin();
	std::vector<std::string>::iterator	headerItBegin = reqSplit.begin() + 1;
	std::vector<std::string>::iterator	headerItEnd;
	std::vector<std::string>::iterator	bodyIt;
	
	std::vector<std::string>::iterator	it = headerItBegin;
	while (it != reqSplit.end() && (*it) != "\n")
		it++;
	if (it == reqSplit.end())
	{
		headerItEnd = reqSplit.end();
		bodyIt = reqSplit.end();
	}
	else
	{
		headerItEnd = it;
		bodyIt = it + 1;
	}
		
	// Check request-line syntax and save information in class
	if (!this->parseRequestLine(*reqLineIt))
		return ;

	std::vector<std::string> headerFields(headerItBegin, headerItEnd);
	
	if (!this->parseHeaderFields(headerFields))
		return ;

	// Check if its better to throw execeptiooooons insetad of if'sssss
}

std::ostream	&operator<<(std::ostream &out, const Request &req)
{
	out << "Request line: " <<std::endl;
	out << "\tMethod: " << req._requestLine._method <<std::endl;
	out << "\tRequest target: " << req._requestLine._requestTarget <<std::endl;
	out << "\tVersion: " << req._requestLine._protocolVersion <<std::endl;
	out << "Header fields: " << std::endl;
	return (out);
}
