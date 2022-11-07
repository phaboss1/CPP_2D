#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>

#define DEINIT 0
#define RUNNING 1

#define PACKET_TYPE_LOGIN_REQUEST 0
#define PACKET_TYPE_LOGIN_RESPONSE 1
#define PACKET_TYPE_BROADCAST 2