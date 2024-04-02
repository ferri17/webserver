
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
		int checkValidNames(std::vector<std::string> lineSplit);
		int checkValidSize(std::string str);
		int checkValidDir(std::string str);
		int checkServSplit(std::string str, int flag, bool key_open, int i, Server &s);
		int checkVarServer(std::vector<std::string> lineSplit, Server &s);
	public:
		Input( void );
		Input( char *fileToOpen );
		bool checkFormat( Server &s );
		void reopenFile ( void );
		~Input( void );
};
