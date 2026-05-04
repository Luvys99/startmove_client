#include "starmove_client.h"
#include "KeyInput.h"
#include "InitNetwork.h"
#include "NetworkProcess.h"
#include "Rendering.h"
#include <stdio.h>
#include <Winsock2.h> // 소켓 함수 사용
#include <WS2tcpip.h> // InetPton, InetNtop 함수 사용

#pragma comment(lib, "ws2_32") 

#define SERVERPORT 3000
char screenbuf[MAX_YLENGTH][MAX_XLENGTH] = { 0 , };

// 전역에 들어갈 것들 ( 프로그램 종료까지 데이터가 유지되어야 하기 때문에 전역으로 선언 )
// 클라이언트는 소켓이 한 개만 필요하므로 전역으로 선언 ( 개발의 편의성을 위함 )
// 플레이어 리스트는 다른 접속한 클라이언트 정보를 저장하기 위함
// mydata는 키입력을 받을 때 내 캐릭터 정보를 업데이트할 때 인자로 넣기 위해 선언 
SOCKET g_socket;
Player PlayerList[20];
Player mydata; 
int g_MyID = -1;
int usercount = 0;

int main()
{
    // 윈속 초기화
    int Init_ret = InitWinsock();
    if (Init_ret == -1)
    {
        return -1;
    }

    // 소켓 생성 및 연결
    ConnecttoServer(g_socket, L"192.168.219.102", SERVERPORT);
   
    // 논블라킹 소켓으로 전환
    int non_ret = Non_blocking_tran(g_socket);
    if (non_ret == -1)
    {
        return -1;
    }

    // 서버에 접속한 이후에 프로그램 로직
    // 프레임
    while (1)
    {
        // 서버에 접속하고 서버에서 부여하는 ID, 별 생성되기 전까지
        // 키보드 입력을 받지 않도록 함
        if (g_MyID != -1)
        {
            // 1. 키보드 처리
            if (KeyInput(mydata) == true)
            {
                STARMOVE move;
                move.Type = 3;
                move.ID = mydata.p_id;
                move.x = mydata.p_x;
                move.y = mydata.p_y;

                // 클라이언트에서 키입력받아서 변경된 좌표를 먼저 저장하고
                // 해당 데이터를 서버로 이동했다고 이동 패킷을 보내는 구조
                // 원래는 select를 거쳐서 wset셋에 넣어서 송신해야 하지만 송신버퍼가 필요하기 때문에
                // 그냥 send 하는 방식을 선택
                int send_ret = SendPacket(g_socket, move);
                if (send_ret == -1)
                {
                    // 에러나면 루프 탈출
                    break;
                }
            }
        }
        
        // 2. 네트워크 처리
        int select_ret = 0;
        select_ret = Selectfunc(g_socket);
        if (select_ret > 0)
        {
            // 서버에 접속하면 ID 할당과 별 생성 패킷이 전달됨
            // 키보드 처리 전에 먼저 Selectfun에서 rset에 등록된 소켓으로 별생성, ID 할당 프로토콜이 전달되고 해당 데이터를 저장할게 됨
            // 반환 값이 -1이거나 0이면 비정상 종료, 정상 종료이므로 break로 탈출하고 클라 종료
            if (RecvMessageAndProcess(g_socket) <= 0)
            {
                // 에러나면 루프 탈출
                break;
            }
        }
        // select_ret = 0 이면 그냥 리턴

        // 3. 랜더링
        memset(screenbuf, ' ', sizeof(screenbuf));
        Render();
    }

    closesocket(g_socket);
    WSACleanup();
    return 0;
}

