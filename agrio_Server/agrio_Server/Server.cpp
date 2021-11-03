#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <cstdlib>
#include <iostream>
#include<vector>
#include<Windows.h>

#include"Protocol.h"
#define SERVERPORT	9000
#define BUFSIZE		512
using namespace std;

CRITICAL_SECTION cs;

COORD CurPos;
CONSOLE_SCREEN_BUFFER_INFO CurPosInfo;

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
class Client;
vector<Client> Clients;



class Client {
	SOCKET sock;
	int id;
public:

	Client(SOCKET socket, int clientId) : id(clientId) {
		sock = socket;
	}
	void Send(void* Packet) const
	{

		int retval = send(sock, reinterpret_cast<char*>(Packet), reinterpret_cast<packet*>(Packet)->packetSize, 0);
		cout << "[TCP 서버]" << retval << "바이트 보냈습니다\n";

	}
	void Recv() {

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
		case CS_PACKET_LOGIN:
		{
			cs_packet_login recvPecket;
			retval = recv(sock, reinterpret_cast<char*>((&recvPecket + 2)), pkSize.packetSize - 2, MSG_WAITALL);

			cout << "cs_packet_login : " << recvPecket.packetSize << " " << recvPecket.packetType << " " << recvPecket.playerSkin << endl;

			sc_packet_login_ok sendPacket;

			sendPacket.packetSize = sizeof(sendPacket);
			sendPacket.packetType = SC_PACKET_LOGIN_OK;
			sendPacket.playerID = id;
			sendPacket.x = 0;
			sendPacket.y = 0;
			Send(&sendPacket);

			/*
			* 새로 접속한 클라이언트에게 현재 그려야할 플레이어를 알려줌
			*/
			for (const auto& Client : Clients) {
				if (Client.id != id) {
					sc_packet_put_obj sendPutPacket;
					sendPutPacket.packetSize = sizeof(sendPutPacket);
					sendPutPacket.packetType = SC_PACKET_PUT_OBJ;
					sendPutPacket.x = 1;
					sendPutPacket.objectID = Client.id;
					Send(&sendPutPacket);
				}
			}
			/*
			* 각 클라이언트들한테 새로운 플레이어가 접속했으니 플레이어 오브젝트를 생성하라고함
			*/
			for (const auto& Client : Clients) {
				if (Client.id != id) {
					sc_packet_put_obj sendPutPacket;
					sendPutPacket.packetSize = sizeof(sendPutPacket);
					sendPutPacket.packetType = SC_PACKET_PUT_OBJ;
					sendPutPacket.x = 1;

					Client.Send(&sendPutPacket);
				}
			}
		}
		break;
		case CS_PACKET_PLAYER_MOVE:
		{
			cs_packet_player_move recvPecket;
			retval = recv(sock, reinterpret_cast<char*>((&recvPecket + 2)), pkSize.packetSize - 2, MSG_WAITALL);

			cout << "cs_packet_player_move : " << (int)recvPecket.dir << endl;
			/*
			* 각 클라이언트들한테 플레이어가 이동했으니 해당 플레이어 오브젝트를 이동 하라고함
			*/
			for (const auto& Client : Clients) {
				if (Client.id != id) {
					sc_packet_obj_move sendPutPacket;
					sendPutPacket.packetSize = sizeof(sendPutPacket);
					sendPutPacket.packetType = SC_PACKET_PUT_OBJ;
					sendPutPacket.x = 1;

					Client.Send(&sendPutPacket);
				}
			}
		}
		break;
		default:
			cout << "잘못된 패킷 전송";
			break;
		}
	}
};

DWORD WINAPI ProcessClient(LPVOID arg)
{
	Client* client = (Client*)arg;

	int retval;

	int addrlen;
	char buf[BUFSIZE + 1];

	while (true)
		client->Recv();



	return 0;
}
int main()
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	//대기 소캣
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	//bind
	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	//listen
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	//통신에 사용할 변수

	SOCKADDR_IN ClientAddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	HANDLE hThread;

	int nClient = 0;
	Clients.reserve(4);
	while (1) {
		//accept()
		addrlen = sizeof(ClientAddr);
		// accept할 쓰레드 있어야함 그래야 로비 게임 루프를 돌 수 있음

		SOCKET client_sock = accept(listen_sock, (SOCKADDR*)&ClientAddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		cout << "\n[TCP 서버] 클라이언트 접속: IP 주소=" << inet_ntoa(ClientAddr.sin_addr)
			<< "포트 번호=" << ntohs(ClientAddr.sin_port) << endl;

		Clients.emplace_back(client_sock, nClient);
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)&Clients[nClient], 0, NULL);

		if (hThread == NULL)closesocket(client_sock);


		nClient++;


	}
	closesocket(listen_sock);
	WSACleanup();
}