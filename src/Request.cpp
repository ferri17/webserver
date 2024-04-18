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
		size_t	separator = headerLine.find(COLON);
		std::string	fieldName;
		std::string	fieldValue;
		if (separator == std::string::npos)
		{
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = SYNTAX_ERROR_HEADER_STR;
			return (false);
		}
		else
		{
			fieldName = headerLine.substr(0, separator);
			fieldValue = headerLine.substr(separator + 1, std::string::npos);
			fieldValue = Request::cleanOWS(fieldValue);
		}
		if (!Request::isValidFieldName(fieldName) || !Request::isValidFieldValue(fieldValue))
		{
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = SYNTAX_ERROR_HEADER_STR;
			return (false);
		}
		this->_headerField.insert(std::pair<std::string, std::string>(stringToLower(fieldName), fieldValue));
	}
	return (true);
}

/*
	Check the validity of the header fields found in the request.
	Headers implemented: content-length, transfer-encoding(chunked) and host.
*/
bool	Request::checkHeaderFields(void)
{
	std::map<std::string, std::string>::iterator	itLength = this->_headerField.find("content-length");
	std::map<std::string, std::string>::iterator	itEncoding = this->_headerField.find("transfer-encoding");
	std::map<std::string, std::string>::iterator	itHost = this->_headerField.find("host");

	// Check host header field
	if (itHost == this->_headerField.end())
	{
		this->_errorCode = BAD_REQUEST;
		this->_errorMssg = HOST_NOT_FOUND_STR;
		return (false);
	}
	// Check content-length and transfer-encoding header field
	if (itLength != this->_headerField.end() && itEncoding != this->_headerField.end())
	{
		this->_errorCode = BAD_REQUEST;
		this->_errorMssg = CONTRADICTORY_HEADERS_STR;
		return (false);
	}
	else if (itLength != this->_headerField.end())
	{
		if (!isInt((*itLength).second))
		{
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = INVALID_CONTENT_LENGTH_STR;
			return (false);
		}
	}
	else if (itEncoding != this->_headerField.end())
	{
		if (stringToLower((*itEncoding).second) != "chunked")
		{
			this->_errorCode = NOT_IMPLEMENTED;
			this->_errorMssg = ENCODING_NOT_IMPLEMENTED_STR;
			return (false);
		}
	}
	return (true);
}

/*
	Check if content-length and transfer-encoding headers exists,
	reads the body message according to that, if there is no message
	_bodyMssg is left empty()
*/
bool	Request::readBodyMessage(std::string & body)
{
	size_t			contentLength = -1;
	std::string	transferEncoding;

	std::map<std::string, std::string>::iterator	itLength = this->_headerField.find("content-length");
	std::map<std::string, std::string>::iterator	itEncoding = this->_headerField.find("transfer-encoding");

	if (itLength != this->_headerField.end())
	{
		// Read body message using content-length
		contentLength = std::strtol((*itLength).second.c_str(), NULL, 10);
		if (body.length() == contentLength)
			this->_bodyMssg = body;
		else
		{
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = WRONG_CONTENT_LENGTH_STR;
			return (false);
		}
	}
	else if (itEncoding != this->_headerField.end())
	{
		// Read body message using transfer-encoding(chunked)
		size_t	endSignal = body.find("0\r\n");
		(void)endSignal;
		if (body.find("0\r\n") != std::string::npos)
		{
			std::vector<std::string>	bodyVec = split_r(body, LF);

			for (std::vector<std::string>::iterator it = bodyVec.begin(); *it != "0\r"; it++)
			{
				
			}
		}
		else
		{
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = INVALID_CHUNK_STR;
			return (false);	
		}
	}
	else
	{
		if (!body.empty())
		{
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = WRONG_CONTENT_LENGTH_STR;
			return (false);
		}
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
	{
		this->_errorCode = BAD_REQUEST;
		this->_errorMssg = SYNTAX_ERROR_REQLINE_STR;
		return (false);
	}

	// Check request-line length against server max length accepted
	if (requestLineStr.length() > MAX_LEN_REQUEST_LINE)
	{
		this->_errorCode = BAD_REQUEST;
		this->_errorMssg = REQLINE_LONG_STR;
		return (false);
	}

	// Split request-line in strings separated by SP
	std::vector<std::string>	requestLineVec = split_r(requestLineStr, SP);

	// Check request-line consists of 3 space separated strings, if not return 400
	if (requestLineVec.size() != 3)
	{
		this->_errorCode = BAD_REQUEST;
		this->_errorMssg = SYNTAX_ERROR_REQLINE_STR;
		return (false);
	}
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
		{
			this->_errorCode = NOT_IMPLEMENTED;
			this->_errorMssg = METHOD_NOT_IMPLEMENTED_STR;
		}
		else
		{
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = SYNTAX_ERROR_METHOD_STR;
		}
		return (false);
	}

	// Check if request-target syntax is valid
	if (!reqTarget.empty())
	{
		size_t	i = 0;
		while (i < reqTarget.length() && isUsAscii(reqTarget.at(i)) && !std::isspace(reqTarget.at(i)))
			i++;
		if (i != reqTarget.length())
		{
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = SYNTAX_ERROR_REQTARGET_STR;
			return (false);
		}
		this->_requestLine._requestTarget = reqTarget;
	}
	else
	{
		this->_errorCode = BAD_REQUEST;
		this->_errorMssg = SYNTAX_ERROR_REQTARGET_STR;
		return (false);
	}

	// Check if HTTP-version is correct
	if (protocol == "HTTP/1.1")
		this->_requestLine._protocolVersion = protocol;
	else
	{
		this->_errorCode = HTTP_VERSION_NOT_SUPPORTED;
		this->_errorMssg = HTTP_VERSION_NOT_SUPPORTED_STR;
		return (false);
	}
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
	std::string	body;
	
	// Init request error code to 0
	this->_errorCode = 0;

	// Remove leading CRLF or LF
	this->skipLeadingGarbage(request);
	// Makes sure request is not empty, if it's empty it sets _errorCode to 400
	if (request.empty())
	{
		this->_errorCode = BAD_REQUEST;
		this->_errorMssg = EMPTY_REQUEST_STR;
		return ;
	}
	// Split body message from request-line and headers
	std::string	opt1 = "\r\n\r\n";
	std::string	opt2 = "\n\n";
	size_t	k = request.find(opt1);
	size_t	l = request.find(opt2);

	if (k < l)
	{
		body = request.substr(k + 4, std::string::npos);
		request = request.substr(0, k);
	}
	else if (l < k)
	{
		body = request.substr(l + 2, std::string::npos);
		request = request.substr(0, l);
	}
	// Split request in vectors using LF as delimitor
	std::vector<std::string>	reqSplit = split_r(request, LF);
	// If last char of each vector is CR we erase it from the chain.
	// We don't remove it from the body message.
	for (std::vector<std::string>::iterator it = reqSplit.begin(); it != reqSplit.end(); it++)
	{
		std::string &	line = (*it);
		if (!line.empty() && line.at(line.length() - 1) == CR)
			line.erase(line.length() - 1);
	}
	// Set iterators to split request into request-line and headers
	// This allows us to generate vectors only containing the info we need on each step
	std::vector<std::string>::iterator	reqLineIt = reqSplit.begin();
	std::vector<std::string>::iterator	headerItBegin = reqSplit.begin() + 1;
	
	if (headerItBegin == reqSplit.end())
	{
		this->_errorCode = BAD_REQUEST;
		this->_errorMssg = SYNTAX_ERROR_HEADER_STR;
		return ;
	}

	// Check request-line syntax and save information in class
	if (!this->parseRequestLine(*reqLineIt))
		return ;
	// Parse header fields 
	std::vector<std::string> headerFields(headerItBegin, reqSplit.end());
	if (!this->parseHeaderFields(headerFields))
		return ;
	// Check header fields validity
	if (!this->checkHeaderFields())
		return ;
	// Read message and check content-length / transfer-encoding
	if (!this->readBodyMessage(body))
		return ;


	// Check if its better to throw execeptiooooons insetad of if'sssss
}

bool	Request::isValidFieldName(std::string & str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!std::isalnum(str.at(i)) && str.at(i) != HYPHEN && str.at(i) != USCORE)
			return (false);
	}
	return (true);
}

bool	Request::isValidFieldValue(std::string & str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!std::isprint(str.at(i)))
			return (false);
	}
	return (true);
}

// Cleans leading and trailing OWS (space character(32) and horizontal tab(9))
std::string	Request::cleanOWS(std::string str)
{
	std::string	cleanStr;

	if (str.empty())
		return (str);

	size_t	begin = str.find_first_not_of(" \t");
	if (begin == std::string::npos)
		cleanStr = "";
	else
	{
		size_t	end = str.find_last_not_of(" \t");
		cleanStr = str.substr(begin, end - begin + 1);
	}
	return (cleanStr);
}

requestLine							Request::getRequestLine(void) const { return (this->_requestLine); }
std::map<std::string, std::string>	Request::getHeaderField(void) const { return (this->_headerField); }
std::string							Request::getBodyMssg(void) const { return (this->_bodyMssg); }
std::string							Request::getMethod(void) const { return (this->_requestLine._method); }
std::string							Request::getRequestTarget(void) const { return (this->_requestLine._requestTarget); }
std::string							Request::getProtocolVersion(void) const { return (this->_requestLine._protocolVersion); }
std::string							Request::getErrorMessage(void) const { return (this->_errorMssg); }
int									Request::getErrorCode(void) const { return (this->_errorCode); }



std::ostream	&operator<<(std::ostream &out, const Request &req)
{
	requestLine							reqLine  = req.getRequestLine();
	std::map<std::string, std::string>	reqHeader = req.getHeaderField();
	std::string							body = req.getBodyMssg();

	out << "Request line: " <<std::endl;
	out << "\tMethod: " << reqLine._method <<std::endl;
	out << "\tRequest target: " << reqLine._requestTarget <<std::endl;
	out << "\tVersion: " << reqLine._protocolVersion <<std::endl;
	out << "Header fields: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = reqHeader.begin(); it != reqHeader.end(); it++)
	{
		out << "\t" << (*it).first << ": " << (*it).second << std::endl;
	}
	out << "Body: " << body;
	return (out);
}

