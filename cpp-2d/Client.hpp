#pragma once

#include "Network.hpp"



class Client {
public:
	static bool isInit;
	static bool isLogging;
	static bool isAuth;
	static bool isLoginRequested;
	static std::string username, password;
	static sf::Clock clientClock;
	static gClient_cbk_interface* callbackInterface;

	static sf::TcpSocket tcpSocket;
	static std::thread tcpListenerThread;
	
	static bool Init(gClient_cbk_interface* callback, const std::string uname, const std::string passwd, const sf::IpAddress ip, const int port)
	{
		if (isInit)
			return false;

		sf::Socket::Status status = tcpSocket.connect(ip, port, sf::milliseconds(1000));
		if (status == sf::Socket::Done)
		{
			isInit = true;
			isAuth = false;
			isLoginRequested = false;
			callbackInterface = callback;
			username = uname;
			password = passwd;
			Log("Starting tcp listener thread and authentication process.");
			tcpListenerThread = std::thread(Client::TCPListener);
			return true;
		}
		else // [TODO] Maybe can return NotReady
		{
			Log("Can not bound to TCP port!", true);
			return false;
		}
	}

	static void TCPListener()
	{
		while (isInit)
		{
			if (isAuth || isLoginRequested)
			{
				sf::Packet receivedPacket;
				sf::Socket::Status receiveStatus = tcpSocket.receive(receivedPacket);
				if (receiveStatus == sf::Socket::Done)
				{
					if (isAuth)
					{
						callbackInterface->OnReceive(receivedPacket);
					}
					else  if (isLoginRequested)
					{
						bool response;
						if (CommunicationUtils::PopLoginResponse(receivedPacket, response))
						{
							
							if (response)
							{
								Log("Authentication is successfull!");
								isAuth = true;
								callbackInterface->OnConnect();
							}
							else
							{
								Log("Authentication rejected by server!");
								isInit = false;
							}
						}
						else
						{
							Log("Expected LoginResponse packet from server but it was not!", true);
						}
					}

				}
				else if (receiveStatus == sf::Socket::Error || receiveStatus == sf::Socket::Partial || receiveStatus == sf::Socket::NotReady)
				{
					// [TODO] Don't know how to handle, not sure if all can happen
				}
				else if (receiveStatus == sf::Socket::Disconnected)
				{
					if(isAuth)
						callbackInterface->OnDisconnect();
					Log("Connection lost with server!", true);
					isInit = false;
				}
				else
				{
					// Not possible to reachable at all
				}
			}
			else 
			{
				if (!isLoginRequested)
				{
					isLoginRequested = CommunicationUtils::SendLoginRequest(tcpSocket, username, password) == sf::Socket::Status::Done;
				}
			}
		}
		std::cout << "";
	}
	
	static bool DeInit()
	{
		if (!isInit)
			return false;

		isInit = false;
		isAuth = false;
		isLoginRequested = false;
		username = "";
		password = "";
		tcpSocket.setBlocking(false);
		tcpSocket.disconnect();
		tcpListenerThread.join();
		callbackInterface = nullptr;
		Log("Client DeInitialized.");
		return true;
	}

	static void Log(const char* log, bool isError = false)
	{
		if (isLogging)
			std::cout << "[CLIENT]" << (isError ? " [ERROR] " : " [INFO] ") << log << std::endl;
	}
};

bool Client::isInit = false;
bool Client::isLogging = false;
bool Client::isAuth = false;
bool Client::isLoginRequested = false;
std::string Client::username, Client::password;
gClient_cbk_interface* Client::callbackInterface;

sf::TcpSocket Client::tcpSocket;
std::thread Client::tcpListenerThread;
