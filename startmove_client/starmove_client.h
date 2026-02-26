#pragma once

#define MAX_XLENGTH 80
#define MAX_YLENGTH 23

// 플레이어 구조체
struct Player
{
    int p_id;
    int p_x;
    int p_y;
};

// 다른 플레이어 정보를 저장할 배열
extern Player PlayerList[10];
extern char screenbuf[MAX_YLENGTH][MAX_XLENGTH];

extern int g_MyID;
extern Player mydata;
extern int usercount;

// ID 할당 구조체
struct IDALLOCATE
{
    int Type;
    int ID;
    int temp[2];
};

// 별 생성 구조체
struct STARCREATE
{
    int Type;
    int id;
    int x;
    int y;
};

// 별 삭제 구조체
struct STARREMOVE
{
    int Type;
    int ID;
    int temp[2];
};

// 별 이동 구조체
struct STARMOVE
{
    int Type;
    int ID;
    int x;
    int y;
};

