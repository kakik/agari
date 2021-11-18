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
	std::vector<GameObject*> GameObjects;

	SOCKET listen_sock;
	static Network* GetInstance();
	Network() {
		assert(instance == nullptr);
		GameObjects.resize(100);
		instance = this;
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return;
	}
	~Network() {
		for (auto& th : threads)
			th.join();
		closesocket(listen_sock);
		WSACleanup();
	}
	bool is_player(int id) {
		return (id >= 0) && (id < 4);
	}
	void send_login_ok(int id);

	void send_put_obj(int id,int target);
	void send_move_obj(int id, int mover);

	void update();
	void start_accept() {
		threads.emplace_back(&Network::AcceptThread, this);
	}


	void ProcessClient(int id);
	void AcceptThread();
};

