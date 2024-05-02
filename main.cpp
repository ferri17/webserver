
#include <iostream>
#include "Input.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Colors.hpp"
#include "NAMETMP.hpp"
#include "Signals.hpp"
#include <signal.h>
// int main(int ac, char **av)
// {
// 	(void)ac;
// 	(void)av;

// 	for (int i = 0; i < 15; i++)
// 	{
// 		std::cout << "REQUEST " << i << std::endl;
// 		std::string	fileName = "files/request" + std::to_string(i) + ".http";
// 		std::ifstream	reqFile(fileName.c_str());

// 		// Determine the size of the file
// 		reqFile.seekg(0, std::ios::end);
// 		std::streamoff size = reqFile.tellg();
// 		reqFile.seekg(0, std::ios::beg);

// 		// Allocate memory for the buffer
// 		char *	buffer = new char[size + 1];
// 		reqFile.read(buffer, size);

// 		// Null terminate string
// 		buffer[size] = '\0';

// 		std::cout << buffer << std::endl;
// 		std::cout << "----------------------------------" << std::endl;
// 		// Close input file
// 		reqFile.close();

// 		//std::cout << "File content:" << std::endl << buffer;

// 		//Parse request in Request class
// 		Request	requestMssg(buffer);
// 		if (!requestMssg.getErrorCode())
// 			std::cout << GREEN << requestMssg << NC<< std::endl;
// 		else
// 		{
// 			std::cout << RED << "Error code: " << NC << requestMssg.getErrorCode() << std::endl;
// 			std::cout << RED << "Error message: " << NC << requestMssg.getErrorMessage() << std::endl;
// 		}
// 		std::cout << "==================================" << std::endl;
// 		std::cout << "==================================" << std::endl;
// 		std::cout << "==================================" << std::endl;
// 	}

// 	Response	testResponse;

// 	std::string	testBody("<h1>Adria dinar a les 14:00</h1>");
// 	testResponse.setBody(testBody);
// 	std::string	bodyLenght;
// 	std::stringstream	ss;
// 	ss << testBody.length();
// 	bodyLenght = ss.str();
	
// 	testResponse.addHeaderField(std::pair<std::string, std::string>("content-length", bodyLenght));	
// 	testResponse.addHeaderField(std::pair<std::string, std::string>("ads-lengtdash", "dassad"));	

// 	std::cout << "Reponse test:" << std::endl;
// 	std::cout << testResponse.generateResponse();
// 	return (0);
// }

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
			itS->preparePages();
			std::cout << GREEN "SERVER:" NC << std::endl;
			std::cout << *itS << std::endl;
			std::map<std::string, Location> &l = itS->getLocations();
			std::map<std::string, Location>::iterator it = l.begin();
			for (; it != l.end(); it++)
				std::cout << it->second << std::endl;

		}
		signal(SIGINT, sigHandler);
		startServ(s[0]);
	}
	catch(const std::exception& e)
	{
		std::cerr << "HULA" << std::endl;
		std::cerr << e.what() << '\n';
	}
	return (0);
}