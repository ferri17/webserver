
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>

#include "Server.hpp"

class Input
{
	private:
		std::string fileOpen;
		std::fstream file;
		bool correct;
		void eraseAllTabs(std::string& str, const std::string& token);
		void throwExeptionFormat(std::string str, int i);
		int	limitsNum(std::string num, int min, int max);
		int checkValidAutoIndex(std::string lineSplit);
		int checkValidNames(std::vector<std::string> &lineSplit);
		int checkValidSize(std::string str);
		int checkValidDir(std::string str);
		int checkValidDirSemiColon(std::string &str);
		int checkServSplit(std::string str, int flag, bool key_serv, int i, Server &s);
		int checkVarServer(std::vector<std::string> lineSplit, int flag, Server &s);
		int checkLocation(std::vector<std::string> lineSplit ,int flag, Server &s);
		int checkLocationVar(std::vector<std::string> lineSplit, Location &loc);
		int checkValidPag(std::string &lineSplit);
		int checkValidCgi(std::string str);
		int checkIp(std::string &str);
		int checkValidListen(std::string &str);
		int checkValidMethods(std::vector<std::string> &lineSplit);

	public:
		Input( void );
		Input( char *fileToOpen );
		bool checkFormat( std::vector<Server> &s );
		void reopenFile ( void );
		~Input( void );
};
