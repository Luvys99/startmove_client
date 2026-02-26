#include "InitNetwork.h"
#include <Winsock2.h> // 소켓 함수 사용
#include <stdio.h>
#include <WS2tcpip.h> // InetPton, InetNtop 함수 사용

void InitWinsock()
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        wprintf(L"윈속 초기화 실패! error_code : %d\n", WSAGetLastError());
        return;
    }
}

void ConnecttoServer(SOCKET& sock, const wchar_t* ip, int port)
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        wprintf(L"socket create failed!! error_code : %d\n", WSAGetLastError());
        return;
    }

    // Ipv4 소켓 주소 구조체
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    // 문자열 IP 정보를 in_addr 형태로 변환
    InetPton(AF_INET, ip, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(port);

    int connect_ret = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (connect_ret == SOCKET_ERROR)
    {
        wprintf(L"connect failed error_code : %d\n", WSAGetLastError());
        return;
    }
}

void Non_blocking_tran(SOCKET sock)
{
    u_long on = 1;
    int non_blocking_ret;
    non_blocking_ret = ioctlsocket(sock, FIONBIO, &on);
    if (non_blocking_ret == SOCKET_ERROR)
    {
        wprintf(L"non_blocking socket failed error_code : %d\n", WSAGetLastError());
        return;
    }

}