#pragma once

#include <iostream>
#include <Box2D/Box2D.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>

#define BOX2D_SCALE 30.f

class SFMLDebugDraw : public b2Draw
{
private:
	sf::RenderWindow* m_window;
	b2World* world;
	sf::Vector2f cameraPosition;
	bool isDebugDrawEnabled = false;
	sf::Color color;

public:
	SFMLDebugDraw(sf::RenderWindow* window, b2World* world, sf::Color color);
	void Render(sf::Vector2f cameraPosition = sf::Vector2f(0, 0));
	sf::Vector2f B2VecToSFVec(const b2Vec2 &vector, bool scaleToPixels = true);
	sf::Color GLColorToSFML(const b2Color &color, sf::Uint8 alpha = 255);
	void setEnabled(bool state);

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color);
	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color);
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	void DrawTransform(const b2Transform& xf);
	void DrawPoint(const b2Vec2& p, float size, const b2Color& color);
	bool IsDebugDrawEnabled();

};
