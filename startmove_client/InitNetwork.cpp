#include "InitNetwork.h"
#include <Winsock2.h> // 소켓 함수 사용
#include <stdio.h>
#include <WS2tcpip.h> // InetPton, InetNtop 함수 사용

int InitWinsock()
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        wprintf(L"윈속 초기화 실패! error_code : %d\n", WSAGetLastError());
        return -1;
    }

    return 1;
}

int ConnecttoServer(SOCKET& sock, const wchar_t* ip, int port)
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        wprintf(L"socket create failed!! error_code : %d\n", WSAGetLastError());
        return -1;
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
        // 클라이언트 코드의 문제가 아닌 외부 환경, 서버의 상태에 대한 에러코드 ( 유저 인식용 )
        // 내 코드 문제 ( 에러 코드만 출력해서 코드 수정 )
        // 내 PC의 리소스 문제 ( NOBUFS 같은 코드, 로그 남기고 종료시켜야 함 )
        // 논 블로킹 전용 에러 ( 넘기면 됨 )
        
        int errCode = WSAGetLastError();
        
        switch (errCode)
        {
        case WSAECONNREFUSED: // 10061 에러 ( 서버가 안 켜져 있으면 )
            wprintf(L"[error] The server refused the connection : %d\n", errCode);
            break;
        case WSAETIMEDOUT: // 10060 에러 ( IP가 다르거나, 방화벽에 막혔다면 )
            wprintf(L"[error] The connection timed out : %d \n", errCode);
            break;
        default: // 알 수 없는 에러들 로그 남기기
            wprintf(L"[Log] Unknown connect error occurred : %d\n", errCode);
            break;
        }
        return -1;
    }

    return 1; // 연결 성공
}

int Non_blocking_tran(SOCKET sock)
{
    u_long on = 1;
    int non_blocking_ret;
    non_blocking_ret = ioctlsocket(sock, FIONBIO, &on);
    if (non_blocking_ret == SOCKET_ERROR)
    {
        wprintf(L"non_blocking socket failed error_code : %d\n", WSAGetLastError());
        return -1;
    }

}