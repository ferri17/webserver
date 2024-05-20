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

/*
	Check if content-length and transfer-encoding headers exists,
	reads the body message according to that, if there is no message
	_bodyMssg is left empty()
*/
/* bool	Request::readBodyMessage(std::string & body)
{
	size_t		contentLength = -1;
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
} */