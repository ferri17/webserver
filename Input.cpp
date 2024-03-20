
#include "Input.hpp"

Input::Input( void ) { correct = 0; }

Input::Input( char *fileToOpen )
{
	fileOpen = fileToOpen;
	reopenFile();
	correct = 0;
}

void Input::reopenFile ( void )
{
	file.open(fileOpen, std::ios::in);

	if (!file.is_open())
		throw std::invalid_argument("File inaccessible");
}

bool checkServ(std::string hola)
{
	if(hola.find("server") == 0)
		return (true);
	return (false);
}

bool Input::checkFormat( void )
{
	std::string hola;

	do
	{
		std::getline(file, hola);
		if (checkServ(hola))
			std::cout << "HOLA" << std::endl;
	}while (!file.eof());

	return (1);
}

Input::~Input( void )
{
	if (file.is_open())
		file.close();
}