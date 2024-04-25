
#include "Signals.hpp"

void sigHandler(int sig)
{
	(void) sig;
	signaled = false;
}