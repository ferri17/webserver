
#include <iostream>
#include "Input.hpp"
#include "Server.hpp"

int main(int ac, char **av)
{
	if (ac > 2)
		return(1);
	try
	{
		Server s;

		if (ac == 1)
			s.initDef();
		else
		{
			Input test(av[1]);
			test.checkFormat(s);
		}
		std::cout << s << std::endl << std::endl;
		Location &l = s.getLocations("/a");
		std::cout  << l << std::endl;
		//s.startServ();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}