
#include "Input.hpp"

#define KO 0
#define MISS_KEY 1
#define ALL_DONE 2
#define COMENT 3
#define NOT_INIT -1

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

int checkOpenKey(std::string str, size_t i)
{
	for (; str[i] && (str[i] == ' ' || str[i] == '\t') ; i++)
		;
	if (str[i] == '{')
	{
		i++;
		for (; str[i] && (str[i] == ' ' || str[i] == '\t') ; i++)
			;
		if (str[i] == '\0')
			return (ALL_DONE);
		return (KO);
	}
	return(KO);
}

int checkServ(std::string hola, int flag)
{
	if (hola.find("#") == 0)
	{
		if (flag == MISS_KEY)
			return (MISS_KEY);
		return(COMENT);
	}
	if (flag == MISS_KEY)
	{
		return (checkOpenKey(hola, 0));
	}
	if(hola.find("server") == 0)
	{
		size_t i = 6;

		for (; hola[i] && (hola[i] == ' ' || hola[i] == '\t') ; i++)
			;
		if (checkOpenKey(hola, i) == ALL_DONE)
			return (ALL_DONE);
		if (hola[i] == '\0')
			return (MISS_KEY);
	}
	return (false);
}

bool Input::checkFormat( void )
{
	std::string hola;
	int flag = NOT_INIT;

	do
	{
		std::getline(file, hola);
		flag = checkServ(hola, flag);
		if (flag == MISS_KEY)
			std::cout << "MISS" << std::endl;
		else if (flag == ALL_DONE)
			std::cout << "ALL" << std::endl;
		else if (flag == COMENT)
			std::cout << "COMENT" << std::endl;
		else
			std::cout << "KO" << std::endl;
	} while (!file.eof());

	return (1);
}

Input::~Input( void )
{
	if (file.is_open())
		file.close();
}