#pragma once

#include <winsock2.h>
#include "starmove_client.h"

bool KeyInput(Player& p);

void SendPacket(SOCKET sock, STARMOVE& move);