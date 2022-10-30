#include <iostream>
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

using namespace std;

int main()
{
	b2World* world = new b2World(b2Vec2(0,0));

	sf::RenderWindow window(sf::VideoMode(800, 600), "Hello World");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

	sf::Texture texture;
	if (!texture.loadFromFile("C:/Users/baris.yavas/Desktop/CPP_2D/Debug/image.png"))
	{
		cout << "error" << endl;
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(shape);
		window.display();
	}

	return 0;
}