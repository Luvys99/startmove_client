#include "KeyInput.h"
#include "starmove_client.h"
#include <windows.h>
#include <WinUser.h>
#include <stdio.h>


// 플래그를 사용해서 좌표가 이동했는지 체크하고 send 보내기
bool KeyInput(Player& p)
{
	bool isMove = false;

	if ((GetAsyncKeyState(VK_LEFT) & 0x8000) && p.p_x > 0)
	{
		p.p_x--;
		isMove = true;
	}
	if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) && p.p_x < MAX_XLENGTH - 2)
	{
		p.p_x++;
		isMove = true;

	}
	if ((GetAsyncKeyState(VK_UP) & 0x8000) && p.p_y > 0)
	{
		p.p_y--;
		isMove = true;
	}
	if ((GetAsyncKeyState(VK_DOWN) & 0x8000) && p.p_y < MAX_YLENGTH - 1)
	{
		p.p_y++;
		isMove = true;
	}

	return isMove;

}

void SendPacket(SOCKET sock, STARMOVE& move)
{
	int send_ret;
	send_ret = send(sock, (char*)&move, 16, 0);
	if (send_ret == SOCKET_ERROR)
	{
		wprintf(L"move data send failed error_code :%d\n", WSAGetLastError());
		return;
	}

}