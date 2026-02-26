#pragma once

#include <Winsock2.h> // 소켓 함수 사용

int Selectfunc(SOCKET sock);

int RecvMessageAndProcess(SOCKET sock);

void ProcessPacket(char * current_message);
