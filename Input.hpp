
#include <iostream>
#include <fstream>

class Input
{
	private:
		std::string fileOpen;
		std::fstream file;
		bool correct;
	public:
		Input( void );
		Input( char *fileToOpen );
		bool checkFormat( void );
		void reopenFile ( void );
		~Input( void );
};
