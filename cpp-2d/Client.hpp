#pragma once

#include <box2d/box2d.h>

#include "Network.hpp"



class gClient_cbk_interface {
public:
	virtual void OnAuth(sf::Packet* packet) = 0;
	virtual void OnPacket(sf::Packet* packet) = 0;

};


class Client {
public:
	static int state;
	static sf::TcpSocket* tcpSocket;
	static std::thread tcpListenerTh;
	static gClient_cbk_interface* clientGameCallback;
	static b2World* world;

	static bool Init(gClient_cbk_interface* callback, const std::string username, const std::string passwd, const sf::IpAddress ip, const int port)
	{
		std::cout << "[Client][Info] Starting client..." << std::endl;
		tcpSocket = new sf::TcpSocket();
		sf::Socket::Status status = tcpSocket->connect(ip, port, sf::milliseconds(1000));

		if (status == sf::Socket::Done)
		{
			std::cout << "[Client][Info] Connected to server..." << std::endl;

			if (TryAuth(username, passwd))
			{
				clientGameCallback = callback;
				state = RUNNING;
				tcpListenerTh = std::thread(Client::TCPListener);

				sf::Packet gameInit;
				tcpSocket->receive(gameInit);

				clientGameCallback->OnAuth(&gameInit);

				return true;
			}
			else
			{
				DeInit();
				return false;
			}
		}
		else
		{
			std::cout << "[Client][Error] Can not bound to TCP port!" << std::endl;
			tcpSocket->disconnect();
			state = DEINIT;
			return false;
		}
	}

	static void Update()
	{

	}

	static void TCPListener()
	{
		while (state == RUNNING)
		{
			sf::Packet receivedPacket;
			if(tcpSocket->receive(receivedPacket) == sf::Socket::Done)
			{
				clientGameCallback->OnPacket(&receivedPacket);
			}
			else 
			{
				// Connection lost, packet corrupted???
				std::cout << "[Client][Error] Connection lost with server!" << std::endl;
				DeInit();
			}
		}
	}

	static bool TryAuth(const std::string username, const std::string passwd)
	{
		sf::Packet loginRequest;
		loginRequest << PACKET_TYPE_LOGIN_REQUEST;
		loginRequest << username;
		loginRequest << passwd;

		if (tcpSocket->send(loginRequest) == sf::Socket::Done)
		{
			std::cout << "[Client][Info] Login request sent..." << std::endl;

			sf::Packet receivedPacket;
			if (tcpSocket->receive(receivedPacket) == sf::Socket::Done)
			{
				int packetType;
				receivedPacket >> packetType;

				if (packetType == PACKET_TYPE_LOGIN_RESPONSE)
				{
					bool loginStatus;
					receivedPacket >> loginStatus;
					return loginStatus;
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
		tcpSocket->setBlocking(false);
		tcpSocket->disconnect();
		tcpListenerTh.join();

		return true;
	}
};

int Client::state = DEINIT;
sf::TcpSocket* Client::tcpSocket;
std::thread Client::tcpListenerTh;
gClient_cbk_interface* Client::clientGameCallback;
b2World* Client::world;