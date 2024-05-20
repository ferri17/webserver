#pragma once

#include "Server.hpp"
#include "Utils.hpp"
#include "ErrorsHttp.hpp"
#include <map>
#include <iostream>
#include <ctype.h>
#include <string>

#define __SKIPPING_GRBG__ 0
#define __PARSING_REQ_LINE__ 1
#define __PARSING_HEADERS__ 2
#define __PARSING_BODY__ 3
#define __FINISHED__ 4

typedef struct	requestLine
{
	std::string	_method;
	std::string	_requestTarget;
	std::string	_protocolVersion;
}	requestLine;

class	Request
{
	private:
		requestLine							_requestLine;
		std::map<std::string, std::string>	_headerField;
		std::string							_bodyMssg;
		int									_errorCode;
		std::string							_errorMssg;
		std::string							_remainder;
		int									_state;

		static bool			isValidFieldName(std::string & str);
		static bool			isValidFieldValue(std::string & str);
		static std::string	cleanOWS(std::string str);
		static void			skipLeadingGarbage(std::string & str);
		static void			removeEndCarriage(std::string & str);

		bool				parseRequestLine(std::string & requestLineStr);
		bool				checkHeaderFields(void);
		bool				parseHeaderField(std::string & headerLine);

		void				skippingGarbage(void);
		void				parsingRequestLine(void);
		void				parsingHeaders(void);
		void				parsingBody(long maxBodySize);
		//bool				readBodyMessage(std::string & body);
	public:
		Request(void);
		Request &	operator=(const Request & other);
		~Request(void);

		void								parseNewBuffer(const char * buffer, long maxBodySize);
		requestLine							getRequestLine(void) const;
		std::string							getMethod(void) const;
		std::string							getRequestTarget(void) const;
		std::string							getProtocolVersion(void) const;
		std::map<std::string, std::string>	getHeaderField(void) const;
		std::string							getBodyMssg(void) const;
		std::string							getRemainder(void) const;
		void								setRemainder(std::string str);
		std::string							getErrorMessage(void) const;
		int									getErrorCode(void) const;
		int									getState(void) const;

};

std::ostream &	operator<<(std::ostream &out, const Request &req);
