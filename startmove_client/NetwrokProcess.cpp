#include "NetworkProcess.h"
#include "starmove_client.h"
#include <WinSock2.h>
#include <stdio.h>

int Selectfunc(SOCKET sock)
{

    // 소켓 셋 초기화
    FD_SET rset;
    FD_ZERO(&rset);
    FD_SET(sock, &rset); // 소켓 rset에 소켓 등록

    // rset에 소켓이 없어도 키입력 로직이 돌아야하기 때문에 0, 0을 집어넣음 ( 대기시간 0 )
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    //select 함수 호출
    int select_ret;
    select_ret = select(0, &rset, nullptr, nullptr, &tv);
    if (select_ret == SOCKET_ERROR)
    {
        wprintf(L"select failed error_code : %d\n", WSAGetLastError());
        return -1;
    }
    // 조건에 만족하는 소켓의 갯수가 0이상
    else if (select_ret > 0)
    {
        ////rset에 소켓이 들어있으면 recv 함수 호출하고 메시지 처리
        if (FD_ISSET(sock, &rset))
            return 1;
    }
    // 0이면 타임아웃 ( 대기 시간 동안 recv, send 할 것 없음 )
    return 0;
    
}

// 랜더링에서 내 플레이어의 x,y가 너무 커지는 것에 대한 원인을 찾아야 됨
// 내 클라이언트 하나만 돌아다니면 값이 커지진 않은데 다른 클라이언트가 접속되어 있으면
// 내 플레이어의 데이터가 갑자기 개 커짐 확인해봐야 됨
int RecvMessageAndProcess(SOCKET sock)
{
    char message[160] = { 0, };

    int recv_ret;
    recv_ret = recv(sock, message, 160, 0);
    if (recv_ret == SOCKET_ERROR)
    {
        int errCode = WSAGetLastError();
        if (errCode == WSAEWOULDBLOCK)
        {
            return 1; 
        }
        else if (errCode == 10054 || errCode == 10053) // 정상  
        {
            // 서버( 상대방 )가 종료되서 연결이 끊김
            wprintf(L"[error] Lost connection to server : %d\n", errCode);
            return -1;
        }
        else if ( errCode == 10038 ) // 비정상
        {
            // 이미 닫힌 소켓을 읽으려고 함
            wprintf(L"[error] WSAENOTSOCK - Attempting to read from a closed socket : 10038\n");
            return -1;
        }
        else // 비정상 or 처리하지 않은 에러 처리
        {
            wprintf(L"[error] abnormal or untreated error handling : %d", errCode);
            return -1;
        }
    }
    else if (recv_ret == 0) // 정상적인 종료 
    {
        return -1;
    }

    int packet_count = recv_ret / 16;

    // 160 바이트를 16바이트씩 패킷 갯수 만큼 반복해서 메시지 처리
    for (int i = 0; i < packet_count; i++)
    {
        // 현재 메시지 주소에서 16바이트씩 증가
        char* current_message = message + (i * 16);
        ProcessPacket(current_message);
    }

    return 1;
}

void ProcessPacket(char* current_message)
{

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