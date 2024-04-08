
#include <iostream>
#include "Input.hpp"
#include "Server.hpp"
#include "Colors.hpp"

int main(int ac, char **av)
{
	if (ac > 2)
		return(1);
	try
	{
		std::vector<Server> s;

		if (ac == 1)
		{
			Server def;

			def.initDef();
			s.push_back(def);
		}
		else
		{
			Input test(av[1]);
			test.checkFormat(s);
		}
		std::vector<Server>::iterator itS = s.begin();
		for (; itS != s.end(); itS++)
		{
			std::cout << GREEN "SERVER:" NC << std::endl;
			std::cout << *itS << std::endl;
			std::map<std::string, Location> &l = itS->getLocations();
			std::map<std::string, Location>::iterator it = l.begin();
			for (; it != l.end(); it++)
				std::cout << it->second << std::endl;

		}
		s[0].startServ();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}