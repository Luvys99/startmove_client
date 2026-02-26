#pragma comment(lib, "ws2_32")
#include "starmove_client.h"
#include "KeyInput.h"
#include "InitNetwork.h"
#include "NetworkProcess.h"
#include "Rendering.h"
#include <stdio.h>
#include <Winsock2.h> // 소켓 함수 사용
#include <WS2tcpip.h> // InetPton, InetNtop 함수 사용

#define SERVERPORT 3000
char screenbuf[MAX_YLENGTH][MAX_XLENGTH] = { 0 , };

// 전역에 들어갈 것들
SOCKET g_socket;
Player PlayerList[10];
Player mydata = { 0, };
int g_MyID = -1;
int usercount = 0;

int main()
{
    // 윈속 초기화
    InitWinsock();

    // 소켓 생성 및 연결
    ConnecttoServer(g_socket, L"192.168.219.102", SERVERPORT);
   
    // 논블라킹 소켓으로 전환
    Non_blocking_tran(g_socket);

    // 서버에 접속한 이후에 프로그램 로직
    while (1)
    {
        // 내 ID로 생성된 별이 존재할 때 이동 가능
        if (g_MyID != -1)
        {
            //키보드 처리
            if (KeyInput(mydata) == true)
            {
                STARMOVE move;
                move.Type = 3;
                move.ID = mydata.p_id;
                move.x = mydata.p_x;
                move.y = mydata.p_y;

                SendPacket(g_socket, move);
            }
        }
        
        //네트워크 처리

        int select_ret = 0;
        select_ret = Selectfunc(g_socket);

        if (select_ret)
        {
            if (RecvMessageAndProcess(g_socket) <= 0)
            {
                break;
            }
        }

        //랜더링
        Render();
    }

    WSACleanup();
    return 0;
}

