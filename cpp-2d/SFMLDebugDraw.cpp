#include "SFMLDebugDraw.hpp"

SFMLDebugDraw::SFMLDebugDraw(sf::RenderWindow* window, b2World* world, sf::Color color) {
	this->m_window = window;
	this->world = world;
	this->cameraPosition = sf::Vector2f(0, 0);
	this->color = color;

	this->SetFlags(b2Draw::e_shapeBit);
	world->SetDebugDraw(this);
}

void SFMLDebugDraw::Render(sf::Vector2f cameraPosition) {
	this->cameraPosition = cameraPosition;

	if (isDebugDrawEnabled)
	{
		//world->DrawDebugData();
		world->DebugDraw();
	}
}

sf::Vector2f SFMLDebugDraw::B2VecToSFVec(const b2Vec2 &vector, bool scaleToPixels)
{
	return sf::Vector2f(vector.x * (scaleToPixels ? BOX2D_SCALE : 1.f) - (scaleToPixels ? cameraPosition.x : 0), vector.y * (scaleToPixels ? BOX2D_SCALE : 1.f) - (scaleToPixels ? cameraPosition.y : 0));
}

sf::Color SFMLDebugDraw::GLColorToSFML(const b2Color &color, sf::Uint8 alpha)
{
	return this->color;
	//return sf::Color(static_cast<sf::Uint8>(color.r * 255), static_cast<sf::Uint8>(color.g * 255), static_cast<sf::Uint8>(color.b * 255), alpha);
}

void SFMLDebugDraw::setEnabled(bool state){
	isDebugDrawEnabled = state;
}

void SFMLDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	sf::ConvexShape polygon(vertexCount);
	sf::Vector2f center;
	for (int i = 0; i < vertexCount; i++)
	{
		sf::Vector2f transformedVec = B2VecToSFVec(vertices[i]);
		polygon.setPoint(i, sf::Vector2f(std::floor(transformedVec.x), std::floor(transformedVec.y)));
	}
	polygon.setOutlineThickness(-1.f);
	polygon.setFillColor(sf::Color::Transparent);
	polygon.setOutlineColor(SFMLDebugDraw::GLColorToSFML(color, 60));

	m_window->draw(polygon);
}

void SFMLDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	sf::ConvexShape polygon(vertexCount);
	for (int i = 0; i < vertexCount; i++)
	{
		sf::Vector2f transformedVec = B2VecToSFVec(vertices[i]);
		polygon.setPoint(i, sf::Vector2f(std::floor(transformedVec.x), std::floor(transformedVec.y)));
	}
	polygon.setOutlineThickness(-1.f);
	polygon.setFillColor(sf::Color::Transparent);
	polygon.setOutlineColor(SFMLDebugDraw::GLColorToSFML(color, 60));

	m_window->draw(polygon);
}

void SFMLDebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color) {
	sf::CircleShape circle(radius * BOX2D_SCALE);
	circle.setOrigin(radius * BOX2D_SCALE, radius * BOX2D_SCALE);
	circle.setPosition(B2VecToSFVec(center));
	circle.setFillColor(sf::Color::Transparent);
	circle.setOutlineThickness(-1.f);
	circle.setOutlineColor(SFMLDebugDraw::GLColorToSFML(color, 60));

	// This is disabled because it is rendering loop chain fixture's loop circle!
	//m_window->draw(circle);
}

void SFMLDebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) {
	sf::CircleShape circle(radius * BOX2D_SCALE);
	circle.setOrigin(radius * BOX2D_SCALE, radius * BOX2D_SCALE);
	circle.setPosition(B2VecToSFVec(center));
	circle.setFillColor(sf::Color::Transparent);
	circle.setOutlineThickness(1.f);
	circle.setOutlineColor(SFMLDebugDraw::GLColorToSFML(color, 60));

	b2Vec2 endPoint = center + radius * axis;
	sf::Vertex line[2] =
	{
		sf::Vertex(B2VecToSFVec(center), SFMLDebugDraw::GLColorToSFML(color)),
		sf::Vertex(B2VecToSFVec(endPoint), SFMLDebugDraw::GLColorToSFML(color)),
	};

	m_window->draw(circle);
	m_window->draw(line, 2, sf::Lines);
}

void SFMLDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
	sf::Vertex line[] =
	{
		sf::Vertex(B2VecToSFVec(p1), sf::Color::Red),
		sf::Vertex(B2VecToSFVec(p2), sf::Color::Red)
	};

	m_window->draw(line, 2, sf::Lines);
}

void SFMLDebugDraw::DrawTransform(const b2Transform& xf) {
	float lineLength = 0.4f;

	b2Vec2 xAxis = xf.p + lineLength * xf.q.GetXAxis();
	sf::Vertex redLine[] =
	{
		sf::Vertex(B2VecToSFVec(xf.p), sf::Color::Red),
		sf::Vertex(B2VecToSFVec(xAxis), sf::Color::Red)
	};

	b2Vec2 yAxis = xf.p + lineLength * xf.q.GetYAxis();
	sf::Vertex greenLine[] =
	{
		sf::Vertex(B2VecToSFVec(xf.p), sf::Color::Green),
		sf::Vertex(B2VecToSFVec(yAxis), sf::Color::Green)
	};

	m_window->draw(redLine, 2, sf::Lines);
	m_window->draw(greenLine, 2, sf::Lines);
}

// TODO : Implement this
void SFMLDebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color) {

}

bool SFMLDebugDraw::IsDebugDrawEnabled()
{
	return isDebugDrawEnabled;
}