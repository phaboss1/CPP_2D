#pragma once

#include <Windows.h>
#include <stdlib.h>
#include <time.h>



class Utils {
public:
	static std::string getRandomString()
	{
		srand((unsigned int)time(NULL));

		std::string retVal;
		for (int i = 0; i < 5; i++)
			retVal += (char)(rand() % 25 + 65);
		return retVal;
	}

	static std::string getCurrentDirectory()
	{
		// Get current path
		TCHAR buffer[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, buffer, MAX_PATH);
		std::string currentDir = buffer;
		currentDir = currentDir.substr(0, currentDir.find_last_of('\\'));
		return currentDir;
	}
};