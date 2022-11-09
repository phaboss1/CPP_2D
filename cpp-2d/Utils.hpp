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

class Box2DUtils {
public:
	static b2Body* CreateRectangle(b2World* world, float xPos, float yPos, float width, float height, b2BodyType type)
	{
		// Create Ground
		b2BodyDef bodyDef;
		bodyDef.position.Set(xPos, yPos);
		bodyDef.type = type;

		b2Body* rectangleBody = world->CreateBody(&bodyDef);

		b2PolygonShape polygonShape;
		polygonShape.SetAsBox(width, height);
		rectangleBody->CreateFixture(&polygonShape, 1.0f);

		return rectangleBody;
	}
};