#pragma comment(lib, "ws2_32")
#include "starmove_client.h"
#include "KeyInput.h"
#include "InitNetwork.h"
#include <stdio.h>
#include <Winsock2.h> // 소켓 함수 사용
#include <WS2tcpip.h> // InetPton, InetNtop 함수 사용

#define SERVERPORT 3000
char screenbuf[MAX_YLENGTH][MAX_XLENGTH] = { 0 , };

// 전역에 들어갈 것들
SOCKET g_socket;
int g_MyID;
Player PlayerList[10];

int main()
{
    // 윈속 초기화
    InitWinsock();

    // 소켓 생성 및 연결
    ConnecttoServer(g_socket, L"192.168.219.102", SERVERPORT);
   
    // 논블라킹 소켓으로 전환
    Non_blocking_tran(g_socket);

    char buf[16];
    int recv_ret;
    int usercount = 0;

    // ID가 없음을 -1로 표시
    Player mydata = { 0, };
    mydata.p_id = -1;
    g_MyID = -1;

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

        // 소켓 셋 초기화
        FD_SET rset;
        FD_ZERO(&rset);

        // rset에 g_socket 을 넣음
        FD_SET(g_socket, &rset);

        // rset에 소켓이 없어도 로직이 돌아야하기 때문에 0, 0을 집어넣음
        timeval timeval;
        timeval.tv_sec = 0;
        timeval.tv_usec = 0;

        // 읽기 셋에 있는 소켓
        int select_ret;
        select_ret = select(0, &rset, nullptr, nullptr, &timeval);
        if (select_ret == SOCKET_ERROR)
        {
            wprintf(L"select socket create failed error_code : %d\n", WSAGetLastError());
            return 0;
        }

        // 조건에 만족하는 소켓의 갯수가 0이상
        if (select_ret > 0)
        {
            //rset에 소켓이 들어있으면 recv 함수 호출하고 메시지 처리
            if (FD_ISSET(g_socket, &rset))
            {
                char message[160] = {0,};
                int select_recv_ret;
                select_recv_ret = recv(g_socket, message, 160, 0);
                if (select_recv_ret == SOCKET_ERROR)
                {
                    wprintf(L"select_socket recv failed error_code :%d\n", WSAGetLastError());
                    return 0;
                }

                int packet_count = select_recv_ret / 16;

                // 160 바이트를 16바이트씩 반복해서 메시지 처리
                for (int i = 0; i < packet_count; i++)
                {
                    // 현재 메시지 주소에서 16바이트씩 증가
                    char* current_message = message + (i * 16);

                    // 16바이트씩 잘라서 type 확인하고 메시지 처리
                    int* type = (int*)current_message;
                    switch (*type)
                    {
                    case 0:
                    {
                        IDALLOCATE* ID_ALLOC = (IDALLOCATE*)current_message;
                        g_MyID = ID_ALLOC->ID;
                        break;
                    }
                    case 1: // 별 생성 ( 새 유저의 별 생성 )
                    {
                        STARCREATE* STAR_C = (STARCREATE*)current_message;
                        // 내 아이디
                        if (g_MyID == STAR_C->id)
                        {
                            mydata.p_id = STAR_C->id;
                            mydata.p_x = STAR_C->x;
                            mydata.p_y = STAR_C->y;
                        }
                        else // 새로운 유저를 리스트에 추가
                        {
                            PlayerList[usercount].p_id = STAR_C->id;
                            PlayerList[usercount].p_x = STAR_C->x;
                            PlayerList[usercount].p_y = STAR_C->y;
                            usercount++;
                        }
                        break;
                    }
                    case 2: // 별 제거 ( 접속 종료한 유저 제거 )
                    {
                        STARREMOVE* STAR_R = (STARREMOVE*)current_message;
                        for (int k = 0; k < usercount; k++)
                        {
                            if (PlayerList[k].p_id == STAR_R->ID)
                            {
                                // 동적할당이 아니기 때문에 마지막 유저의 정보로 제거하려는 유저의 정보를 덮어쓴다.
                                PlayerList[k] = PlayerList[usercount - 1];
                                usercount--;
                                break;
                            }
                        }
                        break;

                    }
                    case 3: // 별 이동
                    {
                        STARMOVE* STAR_M = (STARMOVE*)current_message;
                        // 내 아이디라면 서버의 좌표로 저장
                        if (g_MyID == STAR_M->ID)
                        {
                            mydata.p_x = STAR_M->x;
                            mydata.p_y = STAR_M->y;
                        }
                        else // 아니면 다른 유저의 좌표에 서버의 좌표로 저장
                        {
                            for (int j = 0; j < usercount; j++)
                            {
                                if (PlayerList[j].p_id == STAR_M->ID)
                                {
                                    PlayerList[j].p_x = STAR_M->x;
                                    PlayerList[j].p_y = STAR_M->y;
                                    break;
                                }
                            }
                        }
                        break;

                    }
                    }
                }
            }
        }

        //랜더링

        COORD pos = { 0, 0 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

        for (int y = 0; y < MAX_YLENGTH; y++)
        {
            for (int x = 0; x < MAX_XLENGTH; x++)
            {
                screenbuf[y][x] = ' ';
            }
            screenbuf[y][MAX_XLENGTH - 1] = '\0';
        }
       
        if (g_MyID != -1)
        {
            screenbuf[mydata.p_y][mydata.p_x] = '*';
        }

        for (int i = 0; i < usercount; i++)
        {
            screenbuf[PlayerList[i].p_y][PlayerList[i].p_x] = '*';
        }

        for (int y = 0; y < MAX_YLENGTH; y++)
        {
            printf("%s\n", screenbuf[y]);
        }
        Sleep(20);
    }


    WSACleanup();
    return 0;
}

