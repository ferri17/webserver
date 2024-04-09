
#include <iostream>
#include "Input.hpp"
#include "Server.hpp"
#include "Request.hpp"

int main(int ac, char **av)
{
	(void)ac;
	(void)av;

	std::ifstream	reqFile("files/request.http");

	// Determine the size of the file
    reqFile.seekg(0, std::ios::end);
    std::streamoff size = reqFile.tellg();
    reqFile.seekg(0, std::ios::beg);

	// Allocate memory for the buffer
	char *	buffer = new char[size + 1];
	reqFile.read(buffer, size);

	// Null terminate string
	buffer[size] = '\0';

	// Close input file
	reqFile.close();

	//std::cout << "File content:" << std::endl << buffer;

	//Parse request in Request class
	Request	requestMssg(buffer);

	if (!requestMssg._errorCode)
		std::cout << requestMssg << std::endl;
	else
		std::cout << "Error code: " << requestMssg._errorCode << std::endl;

	return (0);
}