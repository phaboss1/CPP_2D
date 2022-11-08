#pragma once

#include <box2d/box2d.h>

class Box2DUserData {
public:
	b2Body* body;
	Box2DUserData(b2Body* body)
	{
		this->body = body;
	}
};


class Box2DUtils {
public:
	static Box2DUserData* CreateRectangleBody(b2World* world, float xPos, float yPos, float width, float height, b2BodyType type)
	{
		// Create Ground
		b2BodyDef bodyDef;
		bodyDef.position.Set(xPos, yPos);
		bodyDef.type = type;

		b2Body* rectangleBody = world->CreateBody(&bodyDef);

		b2PolygonShape polygonShape;
		polygonShape.SetAsBox(width, height);
		rectangleBody->CreateFixture(&polygonShape, 1.0f);

		Box2DUserData * uD = new Box2DUserData(rectangleBody);

		rectangleBody->SetUserData(uD);

		return uD;
	}
};