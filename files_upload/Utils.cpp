
#include "Utils.hpp"

std::string toString(int num)
{
	std::stringstream	ss;

	ss << num;
	return(ss.str());
}

std::string	getTime(void)
{
	time_t		rawtime;
	struct tm *	timeinfo;
	char		buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);	
	strftime(buffer,sizeof(buffer), "[%d %b %T] ", timeinfo);
	std::string str(buffer);
	return (str);
}

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

std::string trim(std::string& str)
{
    str.erase(str.find_last_not_of(' ')+1);
    str.erase(0, str.find_first_not_of(' '));
    return str;
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

int	hex_to_int(const std::string& hex_string)
{
	int	result = 0;
	for (size_t i = 0; i < hex_string.length(); ++i)
	{
		result *= 16;
		char	c = hex_string[i];
		if (c >= '0' && c <= '9')
			result += c - '0';
		else if (c >= 'A' && c <= 'F')
			result += c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			result += c - 'a' + 10;
		else
			return (-1);
	}
	return (result);
}

------WebKitFormBoundaryzVZGRAaC9jJmAUUg
Content-Disposition: form-data; name="submit"

Subir Archivo
------WebKitFormBoundaryzVZGRAaC9jJmAUUg--
