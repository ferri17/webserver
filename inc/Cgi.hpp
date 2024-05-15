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
		t_cgi_type findExtension(std::string str, std::vector<t_cgi_type> cgi);
	public:
		Cgi();
		~Cgi();
		int generateCgi(std::vector<t_cgi_type> cgi, std::string file, std::string &s, std::vector<std::string> cookies);
		char **generateEnv(std::vector<std::string> cookies);
};
