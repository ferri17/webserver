
#include "Input.hpp"
#include "Utils.hpp"
#include "Colors.hpp"
#include <sys/stat.h>

Input::Input( void ) { correct = 0; }

Input::Input( char *fileToOpen )
{
	fileOpen = fileToOpen;
	reopenFile();
	correct = 0;
	
}

void Input::reopenFile ( void )
{
	file.open(fileOpen, std::ios::in);
	if (!file.is_open())
		throw std::invalid_argument("File inaccessible");

	struct stat fileInfo;
	if (stat(fileOpen.c_str(), &fileInfo) == 0)
	{
		if (S_ISDIR(fileInfo.st_mode))
			throw std::invalid_argument("Its a directory");
	}
	else
		throw std::runtime_error("Error with archive data");
}

void Input::eraseAllTabs(std::string& str, const std::string& token)
{
    size_t pos = 0;
    while ((pos = str.find(token, pos)) != std::string::npos)
        str[pos] = ' ';
}

void Input::throwExeptionFormat(std::string str, int i)
{
	std::stringstream ss;
	ss << RED "File format error in line " << i << ": " NC << str;
	throw std::invalid_argument(ss.str());
}

int	Input::limitsNum(std::string num, int min, int max)
{

	if (num[num.size() - 1] == ';')
		num.erase(num.size() - 1);
	else if (min != 400 && max != 255)
		return (0);
	if (isInt(num))
	{
		int number = atoi(num.c_str());
		if (number > max || number < min)
			return (0);
		return (1);
	}
	return (0);
}

int Input::checkValidNames(std::vector<std::string> &lineSplit)
{
	std::vector<std::string>::iterator it = lineSplit.begin();
	
	std::string str = lineSplit[lineSplit.size() - 1];

	if (str[str.size() - 1] == ';')
		lineSplit[lineSplit.size() - 1].erase(str.size() - 1);
	else
		return (0);
	if (lineSplit.size() == 2 && lineSplit[lineSplit.size() - 1].size() == 0)
		return (0);
	it++;
	for (; it != lineSplit.end(); it++)
	{
		for (int i = 0; (*it)[i]; i++)
		{
			if(isalnum((*it)[i]) == false && (*it)[i] != '.' && (*it)[i] != '-' && (*it)[i] != '_')
				return (0);
		}
	}
	return (1);
}

int Input::checkValidSizeInBytes(std::string str, int type)
{
	int num = atoi(str.c_str());

	if (type == GIGAS && num > 2)
		return (KO);
	else if (type == MEGAS && num > (2 * 1000))
		return (KO);
	else if (type == KILOS && num > (2 * 1000 * 1000))
		return (KO);
	return(type);
}

int Input::checkValidSize(std::string str)
{
	int type = BYTES;

	if (str[str.size() - 1] == ';')
		str.erase(str.size() - 1);
	else
		return (0);
	if (str[str.size() - 1] == 'M' || str[str.size() - 1] == 'K' || str[str.size() - 1] == 'G')
	{
		if (str[str.size() - 1] == 'K')
			type = KILOS;
		else if (str[str.size() - 1] == 'M')
			type = MEGAS;
		else if (str[str.size() - 1] == 'G')
			type = GIGAS;
		str.erase(str.size() - 1);
	}
	if (isInt(str))
		return (checkValidSizeInBytes(str, type));
	return(KO);
}

int Input::checkValidDir(std::string str)
{
	if (str.size() == 0)
		return (0);
	for (int i = 0; str[i]; i++)
	{
		if(isalnum(str[i]) == false && str[i] != '.' && str[i] != '-' && str[i] != '_' && str[i] != '/')
			return (0);
	}
	return (1);
}

int Input::checkValidDirSemiColon(std::string &str)
{
	if (str[str.size() - 1] == ';')
		str.erase(str.size() - 1);
	else
		return (0);
	if (str.size() == 0)
		return (0);
	for (int i = 0; str[i]; i++)
	{
		if(isalnum(str[i]) == false && str[i] != '.' && str[i] != '-' && str[i] != '_' && str[i] != '/')
			return (0);
	}
	return (1);
}

int Input::checkIp(std::string &str)
{
	std::vector<std::string>	ip;

	if (str == "localhost")
		return (1);
	ip = split(str, '.');
	if (ip.size() != 4)
		return (0);
	for (std::vector<std::string>::iterator it = ip.begin(); it != ip.end(); it++)
	{
		if (!limitsNum(*it, 0, 255))
			return (0);
	}
	return (1);
}

int Input::checkValidListen(std::string &str)
{
	if (str.find(":") != str.npos)
	{
		std::vector<std::string> splited = split(str, ':');
		if (checkIp(splited[0]))
			if (limitsNum(splited[1], 0, 65535))
				return(1);
	}
	else if (limitsNum(str, 0, 65535))
		return(1);
	return (0);
}


void addListen(std::string str, Server &s)
{
	if (str.find(":") != str.npos)
	{
		std::vector<std::string> splited = split(str, ':');
		s.addListen((t_listen){splited[0], std::atoi(splited[1].c_str())});
	}
	else
		s.addListen((t_listen){"", std::atoi(str.c_str())});
}

int Input::checkVarServer(std::vector<std::string> lineSplit, int flag, Server &s)
{
	if ((lineSplit.size() >= 2 && lineSplit[0] == "location") || flag == OPEN_KEY_LOC || flag == MISS_KEY_LOC || flag == VALID_ARG_LOC)
		return (checkLocation(lineSplit, flag, s));
	if (lineSplit.size() == 2 && lineSplit[0] == "listen" && checkValidListen(lineSplit[1]))
	{
		addListen(lineSplit[1], s);
		return (VALID_ARG);
	}
	if (lineSplit.size() >= 2 && lineSplit[0] == "server_name" && checkValidNames(lineSplit))
	{
		for (size_t j = 1; j < lineSplit.size(); j++)
			s.pushServerName(lineSplit[j]);
		return	(VALID_ARG);
	}
	if (lineSplit.size() == 2 && lineSplit[0] == "client_max_body_size")
	{
		int type = checkValidSize(lineSplit[1]);

		if (type != KO)
		{
			if (s.getClientMaxBodySize() == -1)
			{
				long num = atoi(lineSplit[1].c_str());
				for (int i = 0; i < type; i++)
					num *= 1000;
				s.setClientMaxBodySize(num);
				return	(VALID_ARG);
			}
		}
	}
	if (lineSplit.size() == 2 && lineSplit[0] == "root" && checkValidDirSemiColon(lineSplit[1]))
	{
		if (s.getRoot().empty())
			s.setRoot(lineSplit[1]);
		return (VALID_ARG);
	}
	if (lineSplit.size() == 3 && lineSplit[0] == "error_page" && limitsNum(lineSplit[1], 400, 599) && checkValidDirSemiColon(lineSplit[2]))
	{
		s.pushErrorPage(std::pair<int, std::string>(std::atoi(lineSplit[1].c_str()), lineSplit[2]));
		return (VALID_ARG);
	}
	if (lineSplit.size() == 2 && lineSplit[0] == "upload_store" && checkValidDirSemiColon(lineSplit[1]))
	{
		if (s.getUploadStore().empty())
			s.setUploadStore(lineSplit[1]);
		return (VALID_ARG);
	}
	if (lineSplit.size() == 1 && lineSplit[0] == "}" && flag == VALID_ARG)
		return (CLOSE_KEY);
	return (KO);
}

int Input::checkValidAutoIndex(std::string lineSplit)
{
	if (lineSplit == "on;" || lineSplit == "off;")
		return (1);
	return (0);
}

int Input::checkValidPag(std::string &str)
{
	if (str[str.size() - 1] == ';')
		str.erase(str.size() - 1);
	else
		return (0);
	if (str.size() == 0)
		return (0);
	for (int i = 0; str[i]; i++)
	{
		if(isalnum(str[i]) == false && str[i] != '.'  && str[i] != ':' && str[i] != '-' && str[i] != '_' && str[i] != '/' && str[i] != '?' && str[i] != '=')
			return (0);
	}
	return (1);
}

int Input::checkValidCgi(std::string str)
{
	if (str == ".sh")
		return (1);
	else if (str == ".py")
		return (1);
	else if (str == ".js")
		return (1);
	else if (str == ".php")
		return (1);
	return (0);
}

int Input::checkValidMethods(std::vector<std::string> &lineSplit)
{
	std::string &str = lineSplit[lineSplit.size() - 1];

	if (str[str.size() - 1] == ';')
		lineSplit[lineSplit.size() - 1].erase(str.size() - 1);
	else
		return (0);
	if (lineSplit.size() == 2 && lineSplit[lineSplit.size() - 1].size() == 0)
		return (0);
	for (size_t i = 1; i < lineSplit.size(); i++)
	{
		if (lineSplit[i] != "POST" && lineSplit[i] != "GET" 
		&& lineSplit[i] != "DELETE")
			return (0);
	}
	return (1);
}

int Input::checkLocationVar(std::vector<std::string> lineSplit, Location &loc)
{
	if (loc.getClose() == true)
		return (VALID_ARG_LOC);
	if (lineSplit.size() == 2 && lineSplit[0] == "autoindex" && checkValidAutoIndex(lineSplit[1]))
	{
		if (loc.getAutoindex() == -1)
			loc.setAutoindex(lineSplit[1] == "on;");
 		return (VALID_ARG_LOC);
	}
	if (lineSplit.size() == 3 && lineSplit[0] == "error_page" && limitsNum(lineSplit[1], 400, 599) && checkValidDirSemiColon(lineSplit[2]))
	{
		loc.pushErrorPage(std::pair<int, std::string>(std::atoi(lineSplit[1].c_str()), lineSplit[2]));
		return (VALID_ARG_LOC);
	}
	if (lineSplit.size() == 2 && lineSplit[0] == "upload_store" && checkValidDirSemiColon(lineSplit[1]))
	{
		if (loc.getUploadStore().empty())
			loc.setUploadStore(lineSplit[1]);
		return (VALID_ARG_LOC);
	}
	if (lineSplit.size() == 2 && lineSplit[0] == "return" && checkValidPag(lineSplit[1]))
	{
		if (loc.getReturnPag().empty())
			loc.setReturnPag(lineSplit[1]);
		return (VALID_ARG_LOC);
	}
	if (lineSplit.size() == 3 && lineSplit[0] == "cgi" && checkValidCgi(lineSplit[1]) && checkValidDirSemiColon(lineSplit[2]))
	{
		loc.setCgi((t_cgi_type){lineSplit[1], lineSplit[2]});
		return (VALID_ARG_LOC);
	}
	if (lineSplit.size() >= 2 && lineSplit[0] == "allow_methods" && checkValidMethods(lineSplit))
	{
		for (size_t i = 1; i < lineSplit.size(); i++)
			loc.pushAllowMethods(lineSplit[i]);
		return (VALID_ARG_LOC);
	}
	if (lineSplit.size() >= 2 && lineSplit[0] == "index" && checkValidNames(lineSplit))
	{
		for (size_t i = 1; i < lineSplit.size(); i++)
			loc.pushIndex(lineSplit[i]);
		return (VALID_ARG_LOC);
	}
	if (lineSplit.size() == 2 && lineSplit[0] == "root" && checkValidDirSemiColon(lineSplit[1]))
	{
		if (loc.getRoot().empty())
			loc.setRoot(lineSplit[1]);
		return (VALID_ARG_LOC);
	}
	return (KO);
}

int Input::checkLocation(std::vector<std::string> lineSplit, int flag, Server &s)
{
	static std::string dirLoc;
	if (lineSplit.size() == 2 && lineSplit[0] == "location" && checkValidDir(lineSplit[1]))
	{
		dirLoc = lineSplit[1];
		return(MISS_KEY_LOC);
	}
	else if (lineSplit.size() == 1 && lineSplit[0] == "{" && flag == MISS_KEY_LOC)
		return (OPEN_KEY_LOC);
	else if (lineSplit.size() == 3 && lineSplit[0] == "location" && checkValidDir(lineSplit[1]) && lineSplit[2] == "{")
	{
		dirLoc = lineSplit[1];
		return(OPEN_KEY_LOC);
	}
	else if (lineSplit.size() == 1 && lineSplit[0] == "}" && (flag == OPEN_KEY_LOC || flag == VALID_ARG_LOC))
	{
		s.getLocations(dirLoc).setClose(true);
		return(VALID_ARG);
	}
	else if (flag == OPEN_KEY_LOC || flag == VALID_ARG_LOC)
		return (checkLocationVar(lineSplit, s.getLocations(dirLoc)));
	return (KO);
}

int Input::checkServSplit(std::string str, int flag, bool key_serv, int i, Server &s)
{
	std::vector<std::string> lineSplit = split(str, ' ');

	if (lineSplit.size() > 0 && lineSplit[0].find('#') == 0)
		return (flag);
	if (key_serv == false)
	{
		if (lineSplit.size() == 1 && lineSplit[0] == "server" && (flag == NOT_INIT || flag == CLOSE_KEY || flag == EMPTY))
			return(MISS_KEY);
		else if (lineSplit.size() == 1 && lineSplit[0] == "server{" && (flag == NOT_INIT || flag == CLOSE_KEY || flag == EMPTY))
			return (OPEN_KEY);
		else if (lineSplit.size() == 1 && lineSplit[0] == "{" && flag == MISS_KEY)
			return (OPEN_KEY);
		else if (lineSplit.size() == 2 && lineSplit[0] == "server" && lineSplit[1] == "{")
			return(OPEN_KEY);
	}
	else if (key_serv == true)
	{
		int type = checkVarServer(lineSplit, flag, s);
		if (type != KO)
			return (type);
	}
	if (lineSplit.size() == 0)
		return (flag);
	throwExeptionFormat(str, i);
	return (KO);
}

bool Input::checkFormat( std::vector<Server> &servers )
{
	std::string line;
	int flag = NOT_INIT;
	bool key_open = false;
	int i = 1;
	Server s;
	do
	{
		std::getline(file, line);
		eraseAllTabs(line, "\t");
		flag = checkServSplit(line, flag, key_open, i, s);
		if (flag == OPEN_KEY)
		{
			key_open = true;
			correct = false;
		}
		else if (flag == CLOSE_KEY && key_open)
		{
			key_open = false;
			correct = true;
			if (s.getRoot().empty())
				throw std::invalid_argument("Missing Root, IS OBLIGATORY!");
			if (s.getClientMaxBodySize() == -1)
				s.setClientMaxBodySize(10000);
			if (s.getListen().empty())
				s.setDefLis();
			if (s.getLocations().empty())
				s.setDefLoc();
			s.preparePages();
			servers.push_back(s);
			s.clean();
		}
		i++;
	} while (!file.eof());
	return (0);
}

Input::~Input( void )
{
	if (file.is_open())
		file.close();
}
