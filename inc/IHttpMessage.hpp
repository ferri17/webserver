#pragma once

#include <unordered_map>
#include <iostream>

class IHttpMessage
{
	protected:
		std::unordered_map<std::string, std::string>	_headerField;
		std::string										_bodyMssg;
	public:
		virtual std::string	createMessage(void) = 0;
};