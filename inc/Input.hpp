
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

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
		int checkServSplit(std::string str, int flag, bool key_open, int i);
	public:
		Input( void );
		Input( char *fileToOpen );
		bool checkFormat( void );
		void reopenFile ( void );
		~Input( void );
};
