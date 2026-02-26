#include "starmove_client.h"
#include "Rendering.h"
#include <Winsock2.h> // 소켓 함수 사용
#include <stdio.h>

void Render()
{
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