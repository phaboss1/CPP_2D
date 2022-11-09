#include <iostream>

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

#include "Utils.hpp"

#include "SFMLDebugDraw.hpp"
#include "Server.hpp"
#include "Client.hpp"




class ServerGame : public gServer_cbk_interface {
public:
	b2World* world;

	ServerGame()
	{
		
	}

	~ServerGame()
	{
		Server::DeInit();
	}

	void Update()
	{
		world->Step(1.f / 60.f, 6, 2);
	}

	void OnServerInit() // Only this is called from main thread
	{
		std::cout << "[ServerGame][Info] OnServerInit!" << std::endl;
		world = new b2World(b2Vec2(0, 9.807f));
		Box2DUtils::CreateRectangle(world, 15, 18, 7, 1.f, b2_staticBody);
	}

	void OnClientLogin(RemoteClient* client)
	{
		std::cout << "[ServerGame][Info] OnClientLogin!" << std::endl;

		sf::Packet worldInit;
		worldInit << PACKET_TYPE_WORLD_INIT;
		worldInit << world->GetGravity().x << world->GetGravity().y;

		worldInit << world->GetBodyCount();

		b2Body* bodyIter = world->GetBodyList();
		while (bodyIter != nullptr)
		{
			CommunicationUtils::PushBodyAndFixtureCreation(bodyIter, worldInit);

			bodyIter = bodyIter->GetNext();
		}
		while (client->socket->send(worldInit) != sf::Socket::Status::Done);

		while (world->IsLocked());
		client->body = Box2DUtils::CreateRectangle(world, 300 / BOX2D_SCALE, 100 / BOX2D_SCALE, 31 / BOX2D_SCALE / 2, 49 / BOX2D_SCALE / 2, b2_dynamicBody);
		sf::Packet bodyCreation;
		bodyCreation << PACKET_TYPE_BODY_CREATE;
		CommunicationUtils::PushBodyAndFixtureCreation(client->body, bodyCreation);
		for (RemoteClient* c : Server::remoteClients)
			while (c->socket->send(bodyCreation) != sf::Socket::Status::Done);
	}

	void OnClientDisconnect(RemoteClient* client)
	{
		std::cout << "[ServerGame][Info] OnClientDisconnect!" << std::endl;
	}

	void OnClientReceive(RemoteClient* client, sf::Packet& packet)
	{
		std::cout << "[ServerGame][Info] OnClientReceive!" << std::endl;
	}
};

class ClientGame : public gClient_cbk_interface {
public:
	b2World* world;
	SFMLDebugDraw* debugDraw;
	sf::RenderWindow* window;
	bool isPlaying;
	bool isGameEnded;

	ClientGame(sf::RenderWindow* window)
	{
		this->window = window;
		this->isPlaying = false;
		this->isGameEnded = false;
	}

	~ClientGame()
	{
		
	}

	void Update()
	{
		if (isPlaying)
		{
			world->Step(1.f / 60.f, 6, 2);
		}
	}

	void Render()
	{
		if (isPlaying)
		{
			// Render DebugDraw
			debugDraw->Render();
		}
	}

	void OnConnect() // There is not use case I can think of right now
	{
		std::cout << "[ClientGame][Info] OnConnect!" << std::endl;
	}

	void OnDisconnect()
	{
		std::cout << "[ClientGame][Info] OnDisconnect!" << std::endl;
		isPlaying = false;
	}

	void OnReceive(sf::Packet& packet)
	{
		std::cout << "[ClientGame][Info] OnReceive!" << std::endl;

		int packetType = CommunicationUtils::GetPacketType(packet);
		if (packetType == PACKET_TYPE_WORLD_INIT)
		{
			int packet_type, body_count;
			float x, y;
			packet >> packet_type >> x >> y >> body_count;
			world = new b2World(b2Vec2(x, y));

			// Create Debug Draw
			debugDraw = new SFMLDebugDraw(window, world, sf::Color::Red);
			debugDraw->setEnabled(true);

			// Create bodies and fixtures
			for (int i = 0; i < body_count; i++)
				CommunicationUtils::PopBodyAndFixtureCreation(world, packet);

			isPlaying = true;
		}
		else if (packetType == PACKET_TYPE_BODY_CREATE)
		{
			int packet_type;
			packet >> packet_type;

			CommunicationUtils::PopBodyAndFixtureCreation(world, packet);
		}
		
	}
};

sf::RenderWindow* window;
int main()
{
	// Create Window
	window = new sf::RenderWindow(sf::VideoMode(800, 600), "Hello World");

	// Create Server
	ServerGame * serverGame = new ServerGame();
	Server::Init(serverGame, 53000);
	
	// Create Client
	ClientGame* clientGame = new ClientGame(window);
	Client::Init(clientGame, Utils::getRandomString(), "123321", "127.0.0.1", 53000);

	while (window->isOpen() && (clientGame->isPlaying || Client::isInit))
	{
		// Update
		{
			if(Server::isInit)
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
	Server::DeInit();

	Client::DeInit();
	delete clientGame;

	return 0;
}