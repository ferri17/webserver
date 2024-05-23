#include "Request.hpp"

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

/*
	Remove all CRLF "\r\n" before the request line, it can also remove
	LF "\n", it doesn't accept any combination like CRLF + LF.

	RFC 9112: "In the interest of robustness, a server that is expecting 
	to receive and parse a request-line SHOULD ignore at least one empty 
	line (CRLF) received prior to the request-line."

*/
void	Request::skipLeadingGarbage(std::string & str)
{
	size_t pos = 0;

	if (str.empty())
		return ;
	// Find position after last LF or position after last CRLF
	if (str.at(0) == LF)
	{
		while (pos < str.length() && str.at(pos) == LF)
			pos++;
	}
	else if (str.at(0) == CR)
	{
		while ((pos + 1) < str.length() && str.at(pos) == CR && str.at(pos + 1) == LF)
				pos += 2;
	}

	// Trim string to the first character of the str-line
	str = str.substr(pos);
}

void	Request::removeEndCarriage(std::string & str)
{
	if (!str.empty() && str.at(str.length() - 1) == CR)
		str.erase(str.length() - 1);
}

void	Request::pushBackBuffer(const char *buffer, int buffSize)
{
	for (int i = 0; i < buffSize; i++)
		this->_remainder.push_back(buffer[i]);
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
			//this->_errorMssg = SYNTAX_ERROR_HEADER_STR;
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
			//this->_errorMssg = SYNTAX_ERROR_HEADER_STR;
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
		//this->_errorMssg = HOST_NOT_FOUND_STR;
		return (false);
	}
	// Check content-length and transfer-encoding header field
	if (itLength != this->_headerField.end() && itEncoding != this->_headerField.end())
	{
		this->_errorCode = BAD_REQUEST;
		//this->_errorMssg = CONTRADICTORY_HEADERS_STR;
		return (false);
	}
	else if (itLength != this->_headerField.end())
	{
		if (!isInt((*itLength).second))
		{
			this->_errorCode = BAD_REQUEST;
			//this->_errorMssg = INVALID_CONTENT_LENGTH_STR;
			return (false);
		}
	}
	else if (itEncoding != this->_headerField.end())
	{
		if (stringToLower((*itEncoding).second) != "chunked")
		{
			this->_errorCode = NOT_IMPLEMENTED;
			//this->_errorMssg = ENCODING_NOT_IMPLEMENTED_STR;
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
		//this->_errorMssg = SYNTAX_ERROR_REQLINE_STR;
		return (false);
	}

	// Check request-line length against server max length accepted
	if (requestLineStr.length() > MAX_LEN_REQUEST_LINE)
	{
		this->_errorCode = BAD_REQUEST;
		//this->_errorMssg = REQLINE_LONG_STR;
		return (false);
	}

	// Split request-line in strings separated by SP
	std::vector<std::string>	requestLineVec = split_r(requestLineStr, SP);

	// Check request-line consists of 3 space separated strings, if not return 400
	if (requestLineVec.size() != 3)
	{
		this->_errorCode = BAD_REQUEST;
		//this->_errorMssg = SYNTAX_ERROR_REQLINE_STR;
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
			//this->_errorMssg = METHOD_NOT_IMPLEMENTED_STR;
		}
		else
		{
			this->_errorCode = BAD_REQUEST;
			//this->_errorMssg = SYNTAX_ERROR_METHOD_STR;
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
			//this->_errorMssg = SYNTAX_ERROR_REQTARGET_STR;
			return (false);
		}
		this->_requestLine._requestTarget = reqTarget;
	}
	else
	{
		this->_errorCode = BAD_REQUEST;
		//this->_errorMssg = SYNTAX_ERROR_REQTARGET_STR;
		return (false);
	}

	// Check if HTTP-version is correct
	if (protocol == "HTTP/1.1")
		this->_requestLine._protocolVersion = protocol;
	else
	{
		this->_errorCode = HTTP_VERSION_NOT_SUPPORTED;
		//this->_errorMssg = HTTP_VERSION_NOT_SUPPORTED_STR;
		return (false);
	}
	return (true);
}
