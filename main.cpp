
#include <iostream>
#include "Input.hpp"

int main(int ac, char **av)
{
	if (ac != 2)
		return(1);
	try
	{
		Input test(av[1]);
		test.checkFormat();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}