#pragma once

#include "Network.hpp"
#include <box2d/box2d.h>



class RemoteClient {
public:
	sf::TcpSocket* socket;
	std::string username;
	b2Body* body;

	RemoteClient()
	{
		this->socket = new sf::TcpSocket();
	}

	~RemoteClient()
	{
		delete this->socket;
	}
};

class gServer_cbk_interface {
public:
	virtual void OnAuth(RemoteClient* client) = 0;
	virtual void OnPacket(RemoteClient* client, sf::Packet& packet) = 0;

};


class Server {
public:
	static int state;
	static sf::TcpListener tcpAuthenticator;
	static std::thread tcpAuthenticatorTh;
	static std::thread tcpListenerTh;
	static std::vector<RemoteClient*> authenticatedClients;
	static std::mutex authenticatedClientsMutex;
	static gServer_cbk_interface* serverGameCallback;
	static b2World* world;

	static bool Init(const int tcpPort, gServer_cbk_interface* cbk)
	{
		if (state != DEINIT)
			return false;

		std::cout << "[Server][Info] Starting server..." << std::endl;

		sf::Socket::Status tcpStatus = tcpAuthenticator.listen(tcpPort);

		if (tcpStatus == sf::Socket::Done)
		{
			world = new b2World(b2Vec2(0, 9.8f));
			serverGameCallback = cbk;
			state = RUNNING;
			tcpAuthenticatorTh = std::thread(Server::TCPAuthenticator);
			tcpListenerTh = std::thread(Server::TCPListener);
			return true;
		}
		else
		{
			std::cout << "[Server][Error] TCP port already in bound!" << std::endl;
			state = DEINIT;
			return false;
		}
	}

	static void Update()
	{
		if (Server::state == RUNNING)
		{
			world->Step(1.f / 60.f, 6, 2);
		}
	}

	static void TCPListener()
	{
		std::cout << "[Server][Info] TCP Listener stating..." << std::endl;

		while (state == RUNNING)
		{
			bool broadcast = false;
			static sf::Clock c;
			if (c.getElapsedTime().asMilliseconds() >= 2500)
			{
				c.restart();
				broadcast = true;
			}

			authenticatedClientsMutex.lock();
			for (std::vector<RemoteClient*>::iterator iter = authenticatedClients.begin(); iter != authenticatedClients.end(); )
			{
				RemoteClient* client = *iter;
				sf::Packet receivedPacket;
				sf::Socket::Status receiveStatus = client->socket->receive(receivedPacket);

				if (receiveStatus == sf::Socket::Done)
				{
					serverGameCallback->OnPacket(client, receivedPacket);

					iter++;
				}
				else if (receiveStatus == sf::Socket::NotReady)
				{
					iter++;
				}
				else 
				{
					std::cout << "[Server][Info] Connection lost with a client..." << std::endl;
					// Disconnected probably
					delete client;
					iter = authenticatedClients.erase(iter);
				}

			}
			authenticatedClientsMutex.unlock();			
		}
	}

	static void TCPAuthenticator()
	{
		std::cout << "[Server][Info] TCP Authenticator stating..." << std::endl;

		while (state == RUNNING)
		{
			RemoteClient* client = new RemoteClient();
			if (tcpAuthenticator.accept(*client->socket) == sf::Socket::Done) // if tcp handshake
			{
				std::cout << "[Server][Info] A new client is accepted..." << std::endl;

				if (TryAuth(client))
				{
					authenticatedClientsMutex.lock();
					authenticatedClients.push_back(client);
					// Call cbk
					serverGameCallback->OnAuth(client);
					client->socket->setBlocking(false);
					authenticatedClientsMutex.unlock();
					std::cout << "[Server][Info] Successfully auth a client..." << std::endl;
				}
				else 
				{
					std::cout << "[Server][Info] Couldn't auth a client..." << std::endl;
					delete client;
				}
			}
			else
			{
				std::cout << "[Server][Info] Error while authenticating a client..." << std::endl;
				delete client;
			}
		}
	}

	static bool TryAuth(RemoteClient* client)
	{
		sf::Packet loginRequest;
		if (client->socket->receive(loginRequest) == sf::Socket::Done)
		{
			int packetType;
			loginRequest >> packetType;
			if (packetType == PACKET_TYPE_LOGIN_REQUEST)
			{
				std::string uname, passwd;
				loginRequest >> uname >> passwd;
				std::cout << "[Server][Info] Login request recived from " << uname << ":" << passwd << "..." << std::endl;

				sf::Packet responsePacket;
				responsePacket << PACKET_TYPE_LOGIN_RESPONSE;

				bool isAuth;
				if (1)
				{
					isAuth = true;
					client->username = uname;
				}
				else 
				{
					isAuth = false;
					std::cout << "[Server][Info] User not found..." << std::endl;
				}

				responsePacket << isAuth;
				client->socket->send(responsePacket);

				return isAuth;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	static bool DeInit()
	{
		if (state != RUNNING)
			return false;

		state = DEINIT;

		tcpAuthenticator.setBlocking(false);
		tcpAuthenticator.close();
		tcpAuthenticatorTh.join();

		tcpListenerTh.join();
		
		return true;
	}
};

int Server::state = DEINIT;
sf::TcpListener Server::tcpAuthenticator;
std::thread Server::tcpAuthenticatorTh;
std::vector<RemoteClient*> Server::authenticatedClients;
std::mutex Server::authenticatedClientsMutex;
std::thread Server::tcpListenerTh;
gServer_cbk_interface* Server::serverGameCallback;
b2World* Server::world;