#pragma once
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib, "ws2_32")
#include "stdfx.h"
#include"../../agrio_Server/agrio_Server/Protocol.h"

/******************************************** ÇÔ¼ö ********************************************/
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL
	);
	MessageBoxA(NULL, (LPCSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL
	);
	MessageBoxA(NULL, (LPCSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ConnectServer();
void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime);

void Send(void* Packet);
void Recv(SOCKET sock);
DWORD WINAPI ProcessClient(LPVOID arg);

void SetBulletPos(DIR direction, Coordinate& pos, short dist);
