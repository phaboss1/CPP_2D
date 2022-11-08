#include <iostream>

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "Utils.hpp"

#include "SFMLDebugDraw.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Box2DUtils.hpp"




class ServerGame : public gServer_cbk_interface {
public:
	
	ServerGame()
	{
		Server::Init(53000, this);

		if (Server::state == RUNNING)
		{
			Box2DUtils::CreateRectangle(Server::world, 15, 18, 7, 1.f, b2_staticBody);
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
		while (Server::world->IsLocked());
		client->body = Box2DUtils::CreateRectangle(Server::world, 300 / BOX2D_SCALE, 100 / BOX2D_SCALE, 31 / BOX2D_SCALE / 2, 49 / BOX2D_SCALE / 2, b2_dynamicBody);
		for (RemoteClient* c : Server::authenticatedClients)
		{
			/*sf::Packet p;
			c->socket->send();*/
		}

		sf::Packet worldInit;
		worldInit << PACKET_TYPE_WORLD_INIT;
		worldInit << 0.f << 9.8f; // gravity

		worldInit << Server::world->GetBodyCount();

		b2Body* bodyIter = Server::world->GetBodyList();
		while (bodyIter != nullptr)
		{
			Box2DUtils::PushBodyAndFixtureCreation(bodyIter, worldInit);

			bodyIter = bodyIter->GetNext();
		}
		client->socket->send(worldInit);
	}

	void OnPacket(RemoteClient* client, sf::Packet& packet)
	{
		//std::cout << "a client packet to server" << std::endl;
	}
};

class ClientGame : public gClient_cbk_interface {
public:
	SFMLDebugDraw* debugDraw;
	sf::RenderWindow* window;

	ClientGame(sf::RenderWindow* window)
	{
		this->window = window;
		Client::Init(this, Utils::getRandomString(), "123321", "127.0.0.1", 53000);
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

	void OnAuth(sf::Packet& packet)
	{
		int packet_type, body_count;
		float x, y;
		packet >> packet_type >> x >> y >> body_count;
		Client::world = new b2World(b2Vec2(x, y));

		// Create Debug Draw
		debugDraw = new SFMLDebugDraw(window, Client::world, sf::Color::Red);
		debugDraw->setEnabled(true);

		// Create bodies and fixtures
		for(int i = 0 ; i < body_count; i++)
			Box2DUtils::PopBodyAndFixtureCreation(Client::world, packet);
	}

	void OnPacket(sf::Packet& packet)
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