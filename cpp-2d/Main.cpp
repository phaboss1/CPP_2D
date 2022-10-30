#include <iostream>
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <Windows.h>

#include "SFMLDebugDraw.hpp"

using namespace std;

#define CAMERA_SPEED 0.1f * 10

b2World* physicsWorld;
sf::RenderWindow* window;
SFMLDebugDraw* debugDraw;
sf::Vector2f cameraPosition;
sf::Sprite playerSprite;
b2Body* playerBody;

bool qKeyFlag = true;

string getCurrentDirectory()
{
	// Get current path
	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string currentDir = buffer;
	currentDir = currentDir.substr(0, currentDir.find_last_of('\\'));
	return currentDir;
}

b2Body* CreateRectangle(float xPos, float yPos, float width, float height, b2BodyType type)
{
	// Create Ground
	b2BodyDef bodyDef;
	bodyDef.position.Set(xPos, yPos);
	bodyDef.type = type;

	b2Body* rectangleBody = physicsWorld->CreateBody(&bodyDef);

	b2PolygonShape polygonShape;
	polygonShape.SetAsBox(width, height);
	rectangleBody->CreateFixture(&polygonShape, 1.0f);

	return rectangleBody;
}

b2Body* CreateCircle(float xPos, float yPos, float radius, b2BodyType type)
{
	// Create Ground
	b2BodyDef bodyDef;
	bodyDef.position.Set(xPos, yPos);
	bodyDef.type = type;

	b2Body* circleBody = physicsWorld->CreateBody(&bodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = radius;
	circleBody->CreateFixture(&circleShape, 1.0f);

	return circleBody;
}

void HandleEvents()
{
	// Enable/Disable Debug Draw
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) && qKeyFlag)
	{
		qKeyFlag = false;
		debugDraw->setEnabled(!debugDraw->IsDebugDrawEnabled());
	}
	else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
	{
		qKeyFlag = true;
	}

	// Create Circle on Mousepress
	{
		static bool createFlag = true;
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && createFlag)
		{
			createFlag = false;
			
			sf::Vector2i mousePosition = sf::Mouse::getPosition() - window->getPosition();
			CreateCircle(mousePosition.x / BOX2D_SCALE, mousePosition.y / BOX2D_SCALE, 0.5f, b2_dynamicBody);
		}
		else if(!sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			createFlag = true;
		}
	}

	// Move Player
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			playerBody->SetLinearVelocity(b2Vec2(playerBody->GetLinearVelocity().x  -1, playerBody->GetLinearVelocity().y));
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			playerBody->SetLinearVelocity(b2Vec2(playerBody->GetLinearVelocity().x + 1, playerBody->GetLinearVelocity().y));
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			playerBody->SetLinearVelocity(b2Vec2(playerBody->GetLinearVelocity().x, playerBody->GetLinearVelocity().y - 5));
	}

	// Camera movement
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			cameraPosition.x -= CAMERA_SPEED;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			cameraPosition.x += CAMERA_SPEED;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			cameraPosition.y += CAMERA_SPEED;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			cameraPosition.y -= CAMERA_SPEED;
	}

	// Iterate all event
	sf::Event event;
	while (window->pollEvent(event))
	{
		// Get close window event
		if (event.type == sf::Event::Closed)
			window->close();
	}
}

int main()
{
	// Set camera to 0,0
	cameraPosition.x = 0;
	cameraPosition.y = 0;

	// Create Box2D world
	physicsWorld = new b2World(b2Vec2(0,9.807f));

	// Create Ground
	CreateRectangle(15, 18, 7, 1.f, b2_staticBody);

	// Create Window
	window = new sf::RenderWindow(sf::VideoMode(800, 600), "Hello World");

	// Create Debug Draw
	debugDraw = new SFMLDebugDraw(window, physicsWorld, sf::Color::Red);
	debugDraw->setEnabled(true);

	// Create Player
	sf::Texture playerTexture;
	playerTexture.loadFromFile(getCurrentDirectory() + "/player.png");
	playerSprite.setTexture(playerTexture);
	playerBody = CreateRectangle(300 / BOX2D_SCALE, 100 / BOX2D_SCALE, 31 / BOX2D_SCALE / 2, 49 / BOX2D_SCALE / 2, b2_dynamicBody);
	
	while (window->isOpen())
	{
		// Update Physics World
		physicsWorld->Step(1.f / 60.f, 6, 2);	

		// Update Player sprite position with physics
		playerSprite.setPosition(playerBody->GetPosition().x * BOX2D_SCALE - 31 / 2.f, playerBody->GetPosition().y * BOX2D_SCALE - 49 / 2.f);

		// Clear Screen
		window->clear();

		// Render DebugDraw
		debugDraw->Render(cameraPosition);

		// Render Player
		window->draw(playerSprite);

		// Render Everything
		window->display();

		// Handle All Events + Keyboard + Mouse
		HandleEvents();

		// Wait 16ms for 60 fps
		sf::sleep(sf::milliseconds(16));
	}

	return 0;
}