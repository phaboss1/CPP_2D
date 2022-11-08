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

	static void PushBodyAndFixtureCreation(b2Body* bodyIter, sf::Packet& worldInit)
	{
		// Push Body related data
		{
			b2Vec2 a = bodyIter->GetPosition();
			worldInit << (int)bodyIter->GetType();
			worldInit << (float)bodyIter->GetPosition().x;
			worldInit << (float)bodyIter->GetPosition().y;
			worldInit << (float)bodyIter->GetAngle();
			worldInit << (float)bodyIter->GetLinearVelocity().x;
			worldInit << (float)bodyIter->GetLinearVelocity().y;
			worldInit << (float)bodyIter->GetAngularVelocity();
			worldInit << (float)bodyIter->GetLinearDamping();
			worldInit << (float)bodyIter->GetAngularDamping();
			worldInit << (int)bodyIter->IsFixedRotation();
		}

		// Push fixture related data
		{
			b2Fixture* absoluteFix = bodyIter->GetFixtureList();

			worldInit << (float)absoluteFix->GetFriction();
			worldInit << (float)absoluteFix->GetRestitution();
			worldInit << (float)absoluteFix->GetDensity();

			b2Shape::Type type = absoluteFix->GetType();
			worldInit << (int)type;

			if (type == b2Shape::e_polygon) {
				b2PolygonShape* poly = (b2PolygonShape*)absoluteFix->GetShape();
				worldInit << (int)poly->m_count;
				for (int i = 0; i < poly->m_count; i++)
				{
					worldInit << (float)poly->m_vertices[i].x;
					worldInit << (float)poly->m_vertices[i].y;
				}
			}
			else
			{
				std::cout << "Couldn't push fixture because no else for it." << std::endl;
			}
		}
	}

	static void PopBodyAndFixtureCreation(b2World* world, sf::Packet& worldInit)
	{
		b2Body* body;
		// Pop Body related data and create body
		{
			int type, isFixedRot;
			float x, y, angle, linVelX, linVelY, angVel, linDamp, angDamp;

			worldInit >> type;
			worldInit >> x;
			worldInit >> y;
			worldInit >> angle;
			worldInit >> linVelX;
			worldInit >> linVelY;
			worldInit >> angVel;
			worldInit >> linDamp;
			worldInit >> angDamp;
			worldInit >> isFixedRot;

			b2BodyDef bodyDef;
			bodyDef.type = (b2BodyType)type;
			bodyDef.position = b2Vec2(x, y);
			bodyDef.angle = angle;
			bodyDef.linearVelocity = b2Vec2(linVelX, linVelY);
			bodyDef.angularVelocity = angVel;
			bodyDef.linearDamping = linDamp;
			bodyDef.angularDamping = angDamp;
			bodyDef.fixedRotation = isFixedRot;
			body = world->CreateBody(&bodyDef);
		}

		b2Fixture* fixture;
		// Pop fixture related data and create fixture
		{
			int type, v_count;
			float friction, restitution, density;

			worldInit >> friction;
			worldInit >> restitution;
			worldInit >> density;
			worldInit >> type;

			if (type == b2Shape::e_polygon)
			{
				worldInit >> v_count;

				b2Vec2* vertices = (b2Vec2*)malloc(sizeof(b2Vec2) * v_count);
				for (int i = 0; i < v_count; i++) {
					float x, y;
					worldInit >> x >> y;
					*(vertices + i) = b2Vec2(x, y);
				}

				b2FixtureDef fixtureDef;
				fixtureDef.friction = friction;
				fixtureDef.restitution = restitution;
				fixtureDef.density = density;

				b2PolygonShape polygon;
				polygon.Set(vertices, v_count);

				fixtureDef.shape = &polygon;

				fixture = body->CreateFixture(&fixtureDef);
			}
			else 
			{
				std::cout << "Couldn't pop fixture because no else for it." << std::endl;
			}
		}
	}
};