#include "error.h"

bool ErrorClose::closeApp = 0;

void reportError(char *str, bool severe)
{
	std::cout << str << std::endl;
	if (severe)
		ErrorClose::closeApp = 1;
}

void reportError(std::string str, bool severe)
{
	std::cout << str << std::endl;
	if (severe)
		ErrorClose::closeApp = 1;
}