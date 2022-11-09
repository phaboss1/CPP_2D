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

	void OnServerInit()
	{
		std::cout << "[ServerGame][Info] OnServerInit!" << std::endl;
		world = new b2World(b2Vec2(0, 0));
	}

	void OnClientLogin(RemoteClient* client)
	{
		std::cout << "[ServerGame][Info] OnClientLogin!" << std::endl;
	}

	void OnClientDisconnect(RemoteClient* client)
	{
		std::cout << "[ServerGame][Info] OnClientDisconnect!." << std::endl;
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