#pragma once

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include "Location.hpp"
#include "Utils.hpp"

class Cgi
{
	private:
	public:
		Cgi();
		~Cgi();
		t_cgi_type findExtension(std::string str, std::vector<t_cgi_type> cgi);
		bool validExtension(std::string str, std::vector<t_cgi_type> cgi);
		int generateCgi(std::vector<t_cgi_type> cgi, std::string file, std::string &s, std::vector<std::string> cookies);
		char **generateEnv(std::vector<std::string> cookies);
};
