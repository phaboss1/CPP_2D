#include <iostream>

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "Utils.hpp"

#include "SFMLDebugDraw.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Box2DUtils.hpp"

b2Body* CreateRectangle(b2World* world, float xPos, float yPos, float width, float height, b2BodyType type)
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


class ServerGame : public gServer_cbk_interface {
public:
	
	ServerGame()
	{
		Server::Init(53000, this);

		if (Server::state == RUNNING)
		{
			CreateRectangle(Server::world, 15, 18, 7, 1.f, b2_staticBody);
		}
	}

	~ServerGame()
	{
		Server::DeInit();
	}

	void Update()
	{
		Server::Update();
	}

	void OnAuth(RemoteClient* client)
	{
		std::cout << "a client auth to server" << std::endl;

		sf::Packet worldInit;
		worldInit << PACKET_TYPE_WORLD_INIT;
		worldInit << 0.f << 0.f; // gravity

		worldInit << Server::world->GetBodyCount();

		b2Body* bodyIter = Server::world->GetBodyList();
		while (bodyIter != nullptr)
		{
			// Push Body related data
			{
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

				worldInit << absoluteFix->GetFriction();
				worldInit << absoluteFix->GetRestitution();
				worldInit << absoluteFix->GetDensity();

				b2Shape::Type type = absoluteFix->GetType();
				worldInit << type;

				if (type == b2Shape::e_polygon) {
					b2PolygonShape* poly = (b2PolygonShape*)absoluteFix->GetShape();
					worldInit << poly->m_count;
					for (int i = 0; i < poly->m_count; i++)
					{
						worldInit << poly->m_vertices[i].x;
						worldInit << poly->m_vertices[i].y;
					}
				}
			}

			bodyIter = bodyIter->GetNext();
		}

		client->socket->send(worldInit);
	}

	void OnPacket(RemoteClient* client, sf::Packet* packet)
	{
		std::cout << "a client packet to server" << std::endl;
	}
};

class ClientGame : public gClient_cbk_interface {
public:
	SFMLDebugDraw* debugDraw;
	sf::RenderWindow* window;

	ClientGame(sf::RenderWindow* window)
	{
		Client::Init(this, Utils::getRandomString(), "123321", "127.0.0.1", 53000);
		this->window = window;
	}

	~ClientGame()
	{
		Client::DeInit();
	}

	void Update()
	{
		Client::Update();
	}

	void Render()
	{
		if (Client::state == RUNNING)
		{
			// Render DebugDraw
			debugDraw->Render();
		}
	}

	void OnAuth(sf::Packet* packet)
	{
		std::cout << "a client auth to client" << std::endl;

		Client::world = new b2World(b2Vec2());

		// Create Debug Draw
		debugDraw = new SFMLDebugDraw(window, Client::world, sf::Color::Red);
		debugDraw->setEnabled(true);
	}

	void OnPacket(sf::Packet* packet)
	{
		std::cout << "a client packet to client" << std::endl;
	}
};

sf::RenderWindow* window;
int main()
{
	// Server
	ServerGame * serverGame = new ServerGame();
	
	// Create Window
	window = new sf::RenderWindow(sf::VideoMode(800, 600), "Hello World");
	
	// Create Client
	ClientGame* clientGame = new ClientGame(window);

	while (window->isOpen() && Client::state == RUNNING)
	{
		// Update
		{
			serverGame->Update();

			clientGame->Update();
		}

		// Rendering
		{
			// Clear Screen
			window->clear();

			// Render Client Game
			clientGame->Render();

			// Flush Everything
			window->display();
		}

		// Iterate all event
		sf::Event event;
		while (window->pollEvent(event))
		{
			// Get close window event
			if (event.type == sf::Event::Closed)
				window->close();
		}

		// Wait 16ms for 60 fps
		sf::sleep(sf::milliseconds(16));
	}

	delete serverGame;
	delete clientGame;
	
	return 0;
}