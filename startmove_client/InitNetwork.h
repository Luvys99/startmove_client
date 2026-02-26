#pragma once

#include <winsock2.h>

void InitWinsock();

void ConnecttoServer(SOCKET& sock, const wchar_t* ip, int port);

void Non_blocking_tran(SOCKET& sock);