#pragma once

#include "starmove_client.h"
#include <winsock2.h>

bool KeyInput(Player& p);

void SendPacket(SOCKET sock, STARMOVE& move);