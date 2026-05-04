#pragma once

#include <winsock2.h>

int InitWinsock();

int ConnecttoServer(SOCKET& sock, const wchar_t* ip, int port);

int Non_blocking_tran(SOCKET sock);