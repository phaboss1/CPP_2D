#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include <box2d/box2d.h>

#define PACKET_TYPE_LOGIN_REQUEST 0
#define PACKET_TYPE_LOGIN_RESPONSE 1
#define PACKET_TYPE_WORLD_INIT 2
#define PACKET_TYPE_BODY_CREATE 3

class RemoteClient {
public:
	sf::TcpSocket* socket;
	sf::Time lastReceive;
	bool isAuth;
	std::string username;
	b2Body* body;

	RemoteClient(sf::TcpSocket* socket, sf::Time lastReceive)
	{
		this->socket = socket;
		this->lastReceive = lastReceive;
		this->isAuth = false;
		this->username = "";
		this->body = nullptr;
	}

	~RemoteClient()
	{

	}
};

class gClient_cbk_interface {
public:
	virtual void OnConnect() = 0;
	virtual void OnDisconnect() = 0;
	virtual void OnReceive(sf::Packet& packet) = 0;

};

class gServer_cbk_interface {
public:
	virtual void OnServerInit() = 0;
	virtual void OnClientLogin(RemoteClient* client) = 0;
	virtual void OnClientDisconnect(RemoteClient* client) = 0;
	virtual void OnClientReceive(RemoteClient* client, sf::Packet& packet) = 0;

};

class CommunicationUtils {
	// Common
private:
	static int ntohs32(const int *input)
	{
		int rval;
		byte *data = (byte *)&rval;

		data[0] = *input >> 24;
		data[1] = *input >> 16;
		data[2] = *input >> 8;
		data[3] = *input >> 0;

		return rval;
	}
public:
	static int GetPacketType(sf::Packet& packet)
	{
		return ntohs32((const int*)packet.getData());
	}

	// Server Side
public:
	static sf::Socket::Status SendLoginResponse(RemoteClient* client, bool response)
	{
		sf::Packet loginResponse;
		loginResponse << (int)PACKET_TYPE_LOGIN_RESPONSE << (bool)response;
		return client->socket->send(loginResponse);
	}

	static bool PopLoginRequest(sf::Packet& packet, std::string& username, std::string& password)
	{
		int packetType = GetPacketType(packet);
		bool isPacketExpectedType = packetType == PACKET_TYPE_LOGIN_REQUEST;
		if (isPacketExpectedType)
			packet >> packetType >> username >> password;

		return isPacketExpectedType;
	}

	// Client Side
public:
	static sf::Socket::Status SendLoginRequest(sf::TcpSocket& socket, std::string username, std::string password)
	{
		sf::Packet loginRequest;
		loginRequest << (int)PACKET_TYPE_LOGIN_REQUEST << username << password;
		return socket.send(loginRequest);
	}

	static bool PopLoginResponse(sf::Packet& packet, bool& response)
	{
		int packetType = GetPacketType(packet);
		bool isPacketExpectedType = packetType == PACKET_TYPE_LOGIN_RESPONSE;
		if (isPacketExpectedType)
			packet >> packetType >> response;

		return isPacketExpectedType;
	}
	
	// Others
public:
	static void PushBodyAndFixtureCreation(b2Body* bodyIter, sf::Packet& packet)
	{
		// Push Body related data
		{
			b2Vec2 a = bodyIter->GetPosition();
			packet << (int)bodyIter->GetType();
			packet << (float)bodyIter->GetPosition().x;
			packet << (float)bodyIter->GetPosition().y;
			packet << (float)bodyIter->GetAngle();
			packet << (float)bodyIter->GetLinearVelocity().x;
			packet << (float)bodyIter->GetLinearVelocity().y;
			packet << (float)bodyIter->GetAngularVelocity();
			packet << (float)bodyIter->GetLinearDamping();
			packet << (float)bodyIter->GetAngularDamping();
			packet << (int)bodyIter->IsFixedRotation();
		}

		// Push fixture related data
		{
			b2Fixture* absoluteFix = bodyIter->GetFixtureList();

			packet << (float)absoluteFix->GetFriction();
			packet << (float)absoluteFix->GetRestitution();
			packet << (float)absoluteFix->GetDensity();

			b2Shape::Type type = absoluteFix->GetType();
			packet << (int)type;

			if (type == b2Shape::e_polygon) {
				b2PolygonShape* poly = (b2PolygonShape*)absoluteFix->GetShape();
				packet << (int)poly->m_count;
				for (int i = 0; i < poly->m_count; i++)
				{
					packet << (float)poly->m_vertices[i].x;
					packet << (float)poly->m_vertices[i].y;
				}
			}
			else
			{
				std::cout << "Couldn't push fixture because no else for it." << std::endl;
			}
		}
	}

	static void PopBodyAndFixtureCreation(b2World* world, sf::Packet& packet)
	{
		b2Body* body;
		// Pop Body related data and create body
		{
			int type, isFixedRot;
			float x, y, angle, linVelX, linVelY, angVel, linDamp, angDamp;

			packet >> type;
			packet >> x;
			packet >> y;
			packet >> angle;
			packet >> linVelX;
			packet >> linVelY;
			packet >> angVel;
			packet >> linDamp;
			packet >> angDamp;
			packet >> isFixedRot;

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

			packet >> friction;
			packet >> restitution;
			packet >> density;
			packet >> type;

			if (type == b2Shape::e_polygon)
			{
				packet >> v_count;

				b2Vec2* vertices = (b2Vec2*)malloc(sizeof(b2Vec2) * v_count);
				for (int i = 0; i < v_count; i++) {
					float x, y;
					packet >> x >> y;
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
