#pragma once

#include "starmove_client.h"
#include <winsock2.h>

bool KeyInput(Player& p);

int SendPacket(SOCKET sock, STARMOVE& move);