
#include "Utils.hpp"

std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
		if (!token.empty())
			tokens.push_back(token);
	return (tokens);
}

std::vector<std::string> split_r(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);
	return (tokens);
}

bool isInt(const std::string & value)
{
	const char	*str = value.c_str();
	char	*endptr;
	long	nb = std::strtol(str, &endptr, 10);
	
	if (endptr != str && *endptr == '\0' && nb <= std::numeric_limits<int>::max() && nb >= std::numeric_limits<int>::min())
		return (true);
	return (false);
}

bool	isUsAscii(int c)
{
	return (c >= 0 && c <= 127);
}

bool	isOWS(int c)
{
	return (c == HTAB || c == SP);
}

std::string	stringToLower(std::string str)
{
	if (str.empty())
		return (str);
	for (size_t i = 0; i < str.length(); i++)
	{
		if (std::isupper(str.at(i)))
			str[i] = std::tolower(str.at(i));
	}
	return (str);
}


