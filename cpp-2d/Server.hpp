#pragma once

#include "Network.hpp"



class RemoteClient {
public:
	sf::TcpSocket* socket;
	std::string username;

	RemoteClient()
	{
		this->socket = new sf::TcpSocket();
	}

	~RemoteClient()
	{
		delete this->socket;
	}
};


class Server {
public:
	static int state;
	static sf::TcpListener tcpAuthenticator;
	static std::thread tcpAuthenticatorTh;
	static std::thread tcpListenerTh;
	static std::vector<RemoteClient*> authenticatedClients;
	static std::mutex authenticatedClientsMutex;

	static bool Init(const int tcpPort)
	{
		if (state != DEINIT)
			return false;

		std::cout << "[Server][Info] Starting server..." << std::endl;

		sf::Socket::Status tcpStatus = tcpAuthenticator.listen(tcpPort);

		if (tcpStatus == sf::Socket::Done)
		{
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
				RemoteClient* rClient = *iter;
				sf::Packet receivedPacket;
				sf::Socket::Status receiveStatus = rClient->socket->receive(receivedPacket);

				if(broadcast)
				{
					sf::Packet broadcastMsg;
					broadcastMsg << PACKET_TYPE_BROADCAST;
					broadcastMsg << "aq cocu";
					rClient->socket->send(broadcastMsg);
				}

				if (receiveStatus == sf::Socket::Done)
				{
					int packetType;
					receivedPacket >> packetType;
					if (packetType == PACKET_TYPE_BROADCAST)
					{
						std::string msg;
						receivedPacket >> msg;
						std::cout << "[Server][Info] A broadcast packet received from the client " << rClient->username << " with message " << msg << "!" << std::endl;
					}
					else 
					{
						std::cout << "[Server][Info] unkown packet!" << std::endl;
					}
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
					delete rClient;
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
					client->socket->setBlocking(false);
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
