
#include <iostream>
#include "Input.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Colors.hpp"

int main(int ac, char **av)
{
	(void)ac;
	(void)av;

	for (int i = 0; i < 14; i++)
	{
		std::cout << "REQUEST " << i << std::endl;
		std::string	fileName = "files/request" + std::to_string(i) + ".http";
		std::ifstream	reqFile(fileName.c_str());

		// Determine the size of the file
		reqFile.seekg(0, std::ios::end);
		std::streamoff size = reqFile.tellg();
		reqFile.seekg(0, std::ios::beg);

		// Allocate memory for the buffer
		char *	buffer = new char[size + 1];
		reqFile.read(buffer, size);

		// Null terminate string
		buffer[size] = '\0';

		std::cout << buffer << std::endl;
		std::cout << "----------------------------------" << std::endl;
		// Close input file
		reqFile.close();

		//std::cout << "File content:" << std::endl << buffer;

		//Parse request in Request class
		Request	requestMssg(buffer);

		if (!requestMssg._errorCode)
			std::cout << GREEN << requestMssg << NC<< std::endl;
		else
			std::cout << RED << "Error code: " << NC << requestMssg._errorCode << std::endl;


		std::cout << "==================================" << std::endl;
		std::cout << "==================================" << std::endl;
		std::cout << "==================================" << std::endl;
	}

	return (0);
}