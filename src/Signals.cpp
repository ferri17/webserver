
#include "Signals.hpp"
#include <stdlib.h>

void sigHandler(int sig)
{
	(void) sig;
	signaled = false;
}