#pragma once

#include "Network.hpp"



class Server {
public:
	static bool isInit;
	static bool isLogging;
	static sf::Clock serverClock;
	static gServer_cbk_interface* callbackInterface;

	static sf::TcpListener tcpAccepter;
	static std::thread tcpAccepterThread;

	static std::thread tcpListenerThread;

	static std::vector<RemoteClient*> remoteClients;
	static std::mutex remoteClientsMutex;

	static bool Init(gServer_cbk_interface* callback, const int tcpPort)
	{
		if (isInit)
			return false;

		sf::Socket::Status tcpStatus = tcpAccepter.listen(tcpPort);
		if (tcpStatus == sf::Socket::Done)
		{
			isInit = true;
			callbackInterface = callback;
			Log("Starting tcp accepter and tcp listener threads.");
			callbackInterface->OnServerInit();
			tcpAccepterThread = std::thread(Server::TCPAccepter);
			tcpListenerThread = std::thread(Server::TCPListener);
			return true;
		}
		else // [TODO] Maybe can return NotReady
		{
			Log("TCP port already in bound!", true);
			return false;
		}
	}
	
	// This is big, I mean "BIG", but handles all cases smoothly
	static void TCPListener()
	{
		while (isInit)
		{
			remoteClientsMutex.lock();
			sf::Time currentTime = serverClock.getElapsedTime();
			for (std::vector<RemoteClient*>::iterator iter = remoteClients.begin(); iter != remoteClients.end(); )
			{
				RemoteClient* client = *iter;

				sf::Packet receivedPacket;
				sf::Socket::Status receiveStatus = client->socket->receive(receivedPacket);
				if (receiveStatus == sf::Socket::Status::Done || receiveStatus == sf::Socket::Status::NotReady)
				{
					if (receiveStatus == sf::Socket::Status::Done)
					{
						client->lastReceive = serverClock.getElapsedTime();
						if (client->isAuth)
						{
							Log("A packet received from authenticated client.");
							callbackInterface->OnClientReceive(client, receivedPacket);
							iter++;
						}
						else 
						{
							std::string username, password;
							if (CommunicationUtils::PopLoginRequest(receivedPacket, username, password))
							{
								if (1)
								{
									Log("An Authentication request accepted.");
									client->isAuth = true;
									client->username = username;
									CommunicationUtils::SendLoginResponse(client, true);
									callbackInterface->OnClientLogin(client);
									iter++;
								}
								else 
								{
									Log("An Authentication request rejected.");
									CommunicationUtils::SendLoginResponse(client, false);
									delete client;
									iter = remoteClients.erase(iter);
								}
							}
							else 
							{
								Log("Expected LoginRequest packet from a not authenticated client but it was not!", true);
								iter++;
							}
						}
					}
					else if(receiveStatus == sf::Socket::Status::NotReady)
					{
						if (client->isAuth)
						{
							// Nothing to be done except waiting for new packet
							iter++;
						}
						else 
						{
							if (currentTime.asMilliseconds() >= 1000 + client->lastReceive.asMilliseconds()) // 1 second for timeout the client
							{
								Log("Authentication with a client timed-out.");
								delete client;
								iter = remoteClients.erase(iter);
							}
							else
							{
								//Log(std::string("Authentication timeout event running for a client ").append(std::to_string(1000 + client->lastReceive.asMilliseconds() - currentTime.asMilliseconds())).append(" ms left.").c_str());
								iter++;
							}
						}
					}
					else 
					{
						// Not possible to reachable at all
					}
				}
				else if (receiveStatus == sf::Socket::Status::Error || receiveStatus == sf::Socket::Status::Partial) // Don't know how to handle both [TODO]
				{
					Log("Unknown receive status! Discarding the packet!", true);
					iter++;
				}
				else if (receiveStatus == sf::Socket::Status::Disconnected)
				{
					if (client->isAuth)
					{
						Log("Connection lost with a authenticated client.");
						callbackInterface->OnClientDisconnect(client);
					}
					else
					{
						Log("Connection lost with a not authenticated client.");
					}
					delete client;
					iter = remoteClients.erase(iter);
				}
				else
				{
					// Not possible to reachable at all
				}
			}
			remoteClientsMutex.unlock();			
		}
	}

	static void TCPAccepter()
	{
		while (isInit)
		{
			sf::TcpSocket* socket = new sf::TcpSocket();
			sf::Socket::Status acceptStatus = tcpAccepter.accept(*socket);
			if (acceptStatus == sf::Socket::Done)
			{
				Log("A new client is accepted...");
				socket->setBlocking(false);
				remoteClientsMutex.lock();
				remoteClients.push_back(new RemoteClient(socket, serverClock.getElapsedTime()));
				remoteClientsMutex.unlock();
			}
			else //if(acceptStatus == sf::Socket::Status::NotReady || acceptStatus == sf::Socket::Status::Error || acceptStatus == sf::Socket::Status::Partial || acceptStatus == sf::Socket::Status::Disconnected) // [TODO] Not sure how to handle rest, maybe only not_ready can be received, maybe none
			{
				delete socket;
			}
		}
	}

	static bool DeInit()
	{
		if (!isInit)
			return false;

		isInit = false;
		tcpAccepter.setBlocking(false);
		tcpAccepter.close();
		tcpAccepterThread.join();
		tcpListenerThread.join();
		callbackInterface = nullptr;
		return true;
	}

	static void Log(const char* log, bool isError = false)
	{
		if (isLogging)
			std::cout << "[SERVER]" << (isError ? " [ERROR] " : " [INFO] ") << log << std::endl;
	}
};

bool Server::isInit = false;
bool Server::isLogging = true;

sf::Clock Server::serverClock;
gServer_cbk_interface* Server::callbackInterface;

sf::TcpListener Server::tcpAccepter;
std::thread Server::tcpAccepterThread;

std::thread Server::tcpListenerThread;

std::vector<RemoteClient*> Server::remoteClients;
std::mutex Server::remoteClientsMutex;