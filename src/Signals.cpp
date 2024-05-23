
#include "Signals.hpp"
#include <stdlib.h>

void sigHandler(int sig)
{
	if (sig == SIGINT)
		signaled = false;
}