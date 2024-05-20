#include "Request.hpp"

Request::Request(void) : _errorCode(0), _state(__SKIPPING_GRBG__) {};
Request::~Request(void) {};

/*
	Parse the request syntax according to HTTP/1.1 as defined in RFC 9112.

	RFC 9112:"The normal procedure for parsing an HTTP message is to read 
	the start-line into a structure, read each header field line into a hash 
	table by field name until the empty line, and then use the parsed data to 
	determine if a message body is expected. If a message body has been 
	indicated, then it is read as a stream until an amount of octets equal to 
	the message body length is read or the connection is closed."
*/
void	Request::parseNewBuffer(const char * buffer, long maxBodySize)
{
	try
	{
		this->_remainder += buffer;
		if (this->_remainder.empty())
			return ;
		if (this->_state == __SKIPPING_GRBG__)
			this->skippingGarbage();
		if (this->_state == __PARSING_REQ_LINE__)
			this->parsingRequestLine();
		if (this->_state == __PARSING_HEADERS__)
			this->parsingHeaders();
		if (this->_state == __PARSING_BODY__)
			this->parsingBody(maxBodySize);
	}
	catch(const std::exception& e)
	{
		this->_state = __FINISHED__;
		std::cerr << RED BOLD << "Error parsing:"  << this->getErrorMessage() << NC << std::endl;
	}
}

void	Request::skippingGarbage(void)
{
	Request::skipLeadingGarbage(this->_remainder);
	if (this->_remainder.empty())
	{
		this->_state = __SKIPPING_GRBG__;
		return ;
	}
	else
		this->_state = __PARSING_REQ_LINE__;
}

void	Request::parsingRequestLine(void)
{
	std::string	requestLine;
	size_t	k = this->_remainder.find(LF);
	
	if (k == std::string::npos)
		return ;
	requestLine = this->_remainder.substr(0, k);
	if (requestLine.back() == CR)
		requestLine.erase(requestLine.length() - 1);
	this->_remainder = this->_remainder.substr(k + 1, std::string::npos);
	// Check request-line syntax and save information in class
	if (!this->parseRequestLine(requestLine))
		throw std::runtime_error("Error parsing request-line");
	this->_state = __PARSING_HEADERS__;
}

void	Request::parsingHeaders(void)
{
	size_t	k = this->_remainder.find(LF);
	while(k != std::string::npos)
	{
		std::string	newLine;
		if (this->_remainder.find(LF) == 0 || this->_remainder.find("\r\n") == 0)
		{
			this->_remainder = this->_remainder.substr(this->_remainder.find(LF) + 1, std::string::npos);
			if (!checkHeaderFields())
				throw std::runtime_error("Error parsing headers");
			this->_state = __PARSING_BODY__;
			break ;
		}
		newLine = this->_remainder.substr(0, k);
		if (newLine.back() == CR)
			newLine.erase(newLine.length() - 1);
		if (!this->parseHeaderField(newLine))
			throw std::runtime_error("Error parsing headers");
		this->_remainder = this->_remainder.substr(k + 1, std::string::npos);			
		if (!this->_remainder.empty())
			k = this->_remainder.find(LF);
		else
			break ;
	}
}

void	Request::parsingBody(long maxBodySize)
{
	size_t		contentLength = -1;
	std::string	transferEncoding;
	std::map<std::string, std::string>::iterator	itLength = this->_headerField.find("content-length");
	std::map<std::string, std::string>::iterator	itEncoding = this->_headerField.find("transfer-encoding");
	if (itLength != this->_headerField.end())
	{
		// Read body message using content-length
		contentLength = std::strtol((*itLength).second.c_str(), NULL, 10);
		if (this->_bodyMssg.length() + this->_remainder.length() > contentLength)
		{
			int	newPos = contentLength - this->_bodyMssg.length();
			this->_remainder = this->_remainder.substr(newPos,std::string::npos);
			this->_errorCode = BAD_REQUEST;
			this->_errorMssg = WRONG_CONTENT_LENGTH_STR;
			this->_state = __FINISHED__;
			return ;
		}
		if (static_cast<long>(this->_bodyMssg.length() + this->_remainder.length()) > maxBodySize)
		{
			int	newPos = maxBodySize - this->_bodyMssg.length();
			this->_remainder = this->_remainder.substr(newPos,std::string::npos);
			this->_errorCode = REQUEST_ENTITY_TOO_LARGE;
			this->_errorMssg = REQUEST_TOO_LARGE_STR;
			this->_state = __FINISHED__;
			return ;
		}
		this->_bodyMssg += this->_remainder;
		this->_remainder = "";
		if (this->_bodyMssg.length() == contentLength)
		{
			this->_state = __FINISHED__;
			return ;
		}
	}
	else if (itEncoding != this->_headerField.end())
	{
		
	}
	else
	{
		this->_state = __FINISHED__;
	}
}

/*
	Cleans and fills header fields/value pairs
*/
bool	Request::parseHeaderField(std::string & headerLine)
{
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

Request &	Request::operator=(const Request & other)
{
	if (this != &other)
	{
		this->_requestLine = other._requestLine;
		this->_headerField = other._headerField;
		this->_bodyMssg = other._bodyMssg;
		this->_errorCode = other._errorCode;
		this->_errorMssg = other._errorMssg;
		this->_remainder = other._remainder;
		this->_state = other._state;
	}
	return (*this);
}

requestLine							Request::getRequestLine(void) const { return (this->_requestLine); }
std::map<std::string, std::string>	Request::getHeaderField(void) const { return (this->_headerField); }
std::string							Request::getBodyMssg(void) const { return (this->_bodyMssg); }
std::string							Request::getMethod(void) const { return (this->_requestLine._method); }
std::string							Request::getRequestTarget(void) const { return (this->_requestLine._requestTarget); }
std::string							Request::getProtocolVersion(void) const { return (this->_requestLine._protocolVersion); }
std::string							Request::getErrorMessage(void) const { return (this->_errorMssg); }
int									Request::getErrorCode(void) const { return (this->_errorCode); }
int									Request::getState(void) const { return (this->_state); }
std::string							Request::getRemainder(void) const { return(this->_remainder); }
void								Request::setRemainder(std::string str) { this->_remainder = str; }

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
