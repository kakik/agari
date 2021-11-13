#include <cstdlib>
#include <iostream>
#include <thread>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32")

#include"../../agrio_Server/agrio_Server/Protocol.h"

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	4000
#define BUFSIZE		512

using namespace std;

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}


void Recv(SOCKET sock) {

	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}
	cout << "Size : " << (int)pkSize.packetSize << endl;
	cout << "Type : " << (int)pkSize.packetType << endl;

	switch (pkSize.packetType)
	{
	case SC_PACKET_LOGIN_OK:
	{
		sc_packet_login_ok recvPacket;
		retval += recv(sock, reinterpret_cast<char*>((&recvPacket + 2)), pkSize.packetSize - 2, MSG_WAITALL);

		cout << "playerID   ; " << (int)recvPacket.playerID << endl;
		cout << "x          ; " << (int)recvPacket.x << endl;
		cout << "y			; " << (int)recvPacket.y << endl;
		cout << "[TCP 클라이언트] " << retval << "바이트를 받았습니다.\n";
	}
	break;
	case SC_PACKET_PUT_OBJ:
	{
		sc_packet_put_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>((&recvPacket + 2)), pkSize.packetSize - 2, MSG_WAITALL);


		cout << "x: " << (int)recvPacket.x;
		cout << "y: " << (int)recvPacket.y << endl;
		cout << "[TCP 클라이언트] " << retval << "바이트를 받았습니다.\n";
	}
	break;
	case SC_PACKET_OBJ_MOVE:
	{
		sc_packet_obj_move recvPacket;
		retval += recv(sock, reinterpret_cast<char*>((&recvPacket + 2)), pkSize.packetSize - 2, MSG_WAITALL);

		cout << "x: " << (int)recvPacket.x;
		cout << "y: " << (int)recvPacket.y << endl;
		cout << "[TCP 클라이언트] " << retval << "바이트를 받았습니다.\n";
	}
	break;
	default:
		cout << "잘못된 패킷 전송";
		break;
	}
}

void ProcessClient(void* arg)
{
	SOCKET sock = reinterpret_cast<SOCKET>(arg);
	//서버와 데이터 통신
	int len;

	while (1) {

		Recv(sock);
	}

}

int main()
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	//socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket");

	//connect

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	ServerAddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");



	thread recvThread( ProcessClient, (void*)sock);


	char buf[BUFSIZE + 1];

	while (1) {
		SleepEx(100, true);
		//게임 메인루프
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;
		int retval = 0;
		switch (buf[0])
		{
		case 'l':
		{
			cs_packet_login sendPacket;
			sendPacket.packetSize = sizeof(cs_packet_login);
			sendPacket.packetType = CS_PACKET_LOGIN;
			sendPacket.playerSkin = 2;
			// 데이터 보내기
			retval = send(sock, reinterpret_cast<char*>(&sendPacket), sizeof(sendPacket), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}
		}
		break;
		case 'm':
		{
			cs_packet_player_move sendPacket;
			sendPacket.packetSize = sizeof(cs_packet_player_move);
			sendPacket.packetType = CS_PACKET_PLAYER_MOVE;
			sendPacket.dir = 2;
			// 데이터 보내기
			retval = send(sock, reinterpret_cast<char*>(&sendPacket), sizeof(sendPacket), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
			}
		}
		break;
		default:
			break;
		}



		cout << "[TCP 클라이언트]" << retval << "바이트 보냈습니다\n";
	}

	recvThread.join();

	WSACleanup();
}