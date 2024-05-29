#include "Request.hpp"

Request::Request(void) : _errorCode(0), _chunkSize(-1), _loadChunk(false), _state(__SKIPPING_GRBG__), _timeout(-1) {}
Request::~Request(void) {}

/*
	Parse the request syntax according to HTTP/1.1 as defined in RFC 9112.

	RFC 9112:"The normal procedure for parsing an HTTP message is to read 
	the start-line into a structure, read each header field line into a hash 
	table by field name until the empty line, and then use the parsed data to 
	determine if a message body is expected. If a message body has been 
	indicated, then it is read as a stream until an amount of octets equal to 
	the message body length is read or the connection is closed."
*/
void	Request::parseNewBuffer(const char * buffer, int buffSize, long maxBodySize)
{
	try
	{
		if (this->_timeout == -1)
			this->_timeout = 0;
		this->pushBackBuffer(buffer, buffSize);
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
		std::cerr << getTime() << RED BOLD << "Error parsing: "  << ERROR_MESSAGE(_errorCode) << NC << std::endl;
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
	size_t	i = 0;
	size_t	contentLength = -1;
	std::string	transferEncoding;
	std::map<std::string, std::string>::iterator	itLength = this->_headerField.find("content-length");
	std::map<std::string, std::string>::iterator	itEncoding = this->_headerField.find("transfer-encoding");
	if (itLength != this->_headerField.end())
	{
		// Read body message using content-length
		contentLength = std::strtol((*itLength).second.c_str(), NULL, 10);
		for (i = 0; i < this->_remainder.length() && this->_bodyMssg.length() < contentLength; i++)
		{
			this->_bodyMssg.push_back(this->_remainder.at(i));
		}
		this->_remainder = this->_remainder.substr(i, std::string::npos);
		if (this->_bodyMssg.length() > static_cast<size_t>(maxBodySize))
		{
			this->_errorCode = REQUEST_ENTITY_TOO_LARGE;
			this->_state = __FINISHED__;
			throw std::runtime_error("Error max client body size");
		}
		if (this->_bodyMssg.length() == contentLength)
		{
			this->_state = __FINISHED__;
			return ;
		}
	}
	else if (itEncoding != this->_headerField.end() && stringToLower((*itEncoding).second) == "chunked")
	{
		// Read body message using transfer-encoding
		if (!this->_loadChunk)	
			findChunkSize();
		else if (this->_state != __FINISHED__)
			loadChunk();
	}
	else
	{
		this->_state = __FINISHED__;
	}
}

void	Request::loadChunk(void)
{
	size_t	i;

	for (i = 0; i < this->_remainder.size() && this->_chunkSize; i++, this->_chunkSize--)
	{
		this->_bodyMssg.push_back(this->_remainder.at(i));
	}
	if (this->_chunkSize)
	{
		this->_remainder.clear();
	}
	else
	{
		if (i == this->_remainder.size())
			this->_remainder.clear();
		else if (i == this->_remainder.size() - 1)
		{
			if (this->_remainder.at(i) == LF)
			{
				this->_remainder.clear();
				this->_loadChunk = false;
			}
		}
		else
		{
			this->_loadChunk = false;
			if (this->_remainder.at(i) == CR && this->_remainder.at(i + 1) == LF)
				this->_remainder = this->_remainder.substr(i + 2, std::string::npos);
			else if (this->_remainder.at(i) == LF)
				this->_remainder = this->_remainder.substr(i + 1, std::string::npos);
			else
			{
				this->_errorCode = BAD_REQUEST;
				throw std::runtime_error("Error reading chunks");
			}
			if (!this->_remainder.empty())
				findChunkSize();
		}
	}
}

void	Request::findChunkSize(void)
{
	size_t	i;

	for (i = 0; i < this->_remainder.size(); i++)
	{
		if (this->_remainder.at(i) == LF)
		{
			this->_remainder = this->_remainder.substr(i + 1, std::string::npos);
			if (this->_chunk.back() == CR)
				this->_chunk.erase(this->_chunk.length() - 1);
			if ((this->_chunkSize = hex_to_int(this->_chunk)) < 0)
			{
				this->_errorCode = BAD_REQUEST;
				throw std::runtime_error("Error reading chunks");
			}
			else if (this->_chunkSize == 0)
			{
				this->_state = __FINISHED__;
				return ;
			}
			this->_chunk.clear();
			this->_loadChunk = true;
			loadChunk();
		}
		else
			this->_chunk.push_back(this->_remainder.at(i));
	}
	this->_remainder.clear();
}

Request &	Request::operator=(const Request & other)
{
	if (this != &other)
	{
		this->_requestLine = other._requestLine;
		this->_headerField = other._headerField;
		this->_bodyMssg = other._bodyMssg;
		this->_errorCode = other._errorCode;
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
int									Request::getErrorCode(void) const { return (this->_errorCode); }
void								Request::setErrorCode(int err) { this->_errorCode = err; }
int									Request::getState(void) const { return (this->_state); }
void								Request::setState(int state) { this->_state = state; }
std::string							Request::getRemainder(void) const { return(this->_remainder); }
void								Request::setRemainder(std::string str) { this->_remainder = str; }
ssize_t								Request::getTimeout(void) const { return (this->_timeout); }
void								Request::setTimeout(size_t time) { this->_timeout = time; }
void								Request::addTimeout(size_t time) { this->_timeout += time; }


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
