#pragma comment(lib, "ws2_32")
#include "starmove_client.h"
#include "KeyInput.h"
#include <stdio.h>
#include <Winsock2.h> // 소켓 함수 사용
#include <WS2tcpip.h> // InetPton, InetNtop 함수 사용

#define SERVERPORT 3000
char screenbuf[MAX_YLENGTH][MAX_XLENGTH];

// 전역에 들어갈 것들
SOCKET g_socket;
int g_MyID;
Player PlayerList[10];

int main()
{
    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        wprintf(L"윈속 초기화 실패! error_code : %d\n", WSAGetLastError());
        return 1;
    }

    g_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_socket == INVALID_SOCKET)
    {
        wprintf(L"socket create failed!! error_code : %d\n", WSAGetLastError());
        return 0;
    }

    // Ipv4 소켓 주소 구조체
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    // 문자열 IP 정보를 in_addr 형태로 변환
    InetPton(AF_INET, L"127.0.0.1", &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    int connect_ret = connect(g_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (connect_ret == SOCKET_ERROR)
    {
        wprintf(L"connect failed error_code : %d\n", WSAGetLastError());
        return 0;
    }
   
    // 이후 논블라킹 소켓으로 전환
    u_long on = 1;
    int non_blocking_ret;
    non_blocking_ret = ioctlsocket(g_socket, FIONBIO, &on);
    if (non_blocking_ret == SOCKET_ERROR)
    {
        wprintf(L"non_blocking socket failed error_code : %d\n", WSAGetLastError());
        return 0;
    }

    char buf[16];
    int recv_ret;
    int usercount = 0;

    // ID가 없음을 -1로 표시
    Player mydata;
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

                // 160 바이트를 16바이트씩 반복해서 메시지 처리
                for (int i = 0; i < 10; i++)
                {
                    // 현재 메시지 주소에서 16바이트씩 증가
                    char* current_message = message + (i * 16);

                    // 16바이트씩 잘라서 type 확인하고 메시지 처리
                    int* type = (int*)current_message;
                    switch (*type)
                    {
                    case 0:
                    {
                        IDALLOCATE* ID_ALLOC = (IDALLOCATE*)message;
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

        for (int i = 0; usercount; i++)
        {
            screenbuf[PlayerList[i].p_y][PlayerList[i].p_x] = '@';
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

