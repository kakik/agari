#pragma once


#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>

#include "Protocol.h"
#include "Client.h"
#include "GameObject.h"


#define BUFSIZE		512


void err_quit(const char* msg);
void err_display(const char* msg);



class Network
{
	static Network* instance;
	char buf[BUFSIZE];
public:
	
	std::vector<std::thread> threads;
	std::vector<GameObject> GameObjects;
	std::vector<Client> Clients;
	SOCKET listen_sock;
	static Network* GetInstance();
	Network() {
		assert(instance == nullptr);

		instance = this;
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return;

		Clients.reserve(3);
	}
	~Network() {
		for (auto& th : threads)
			th.join();
		closesocket(listen_sock);
		WSACleanup();
	}

	void send_login_ok(int id);

	void send_put_obj(int id);
	void send_move_obj(int id);

	void update();
	void start_accept() {

		threads.emplace_back(&Network::AcceptThread, this);
	}


	void ProcessClient(int id);
	void AcceptThread();
};

