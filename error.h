#ifndef Z_ERR
#define Z_ERR

#include <iostream>
#include <string>

//Error reporting file
class ErrorClose
{
public:
	static bool closeApp;
};

void reportError(char *str, bool severe);
void reportError(std::string str, bool severe);

#endif