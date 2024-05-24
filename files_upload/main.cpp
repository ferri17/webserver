
#include <iostream>
#include "Input.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Colors.hpp"
#include "StartServer.hpp"
#include "Signals.hpp"
#include <signal.h>

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
			def.preparePages();
			s.push_back(def);
		}
		else
		{
			Input test(av[1]);
			test.checkFormat(s);
		}
		signal(SIGINT, sigHandler);
		startServers(s);
	}
	catch(const std::exception& e)
	{
		std::cerr << getTime() << BOLD RED << e.what() << NC << std::endl;
	}
	return (0);
}
------WebKitFormBoundarysn8JJ28CBBB637dq
Content-Disposition: form-data; name="submit"

Subir Archivo
------WebKitFormBoundarysn8JJ28CBBB637dq--
