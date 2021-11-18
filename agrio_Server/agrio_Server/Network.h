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
		for (int i = 0; i < 100;++i) {
			GameObjects.push_back(new GameObject);
		}
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

		for (auto go : GameObjects) {
			delete go;
		}
	}
	void disconnect(int id) {
		Player* p = reinterpret_cast<Player*>(GameObjects[id]);
		closesocket(p->sock);
		GameObjects[id]->isActive = false;
		GameObjects[id]->isMove = false;
	}
	char get_id() {
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) return i;
		}
		return -1;
	}
	bool is_player(int id) {
		return (id >= 0) && (id < 4);
	}
	void start_accept() {
		threads.emplace_back(&Network::AcceptThread, this);
	}

	void send_put_obj(int id,int target);
	void send_move_obj(int id, int mover);
	void send_change_state(int id, int target);

	void update(float elapsedTime);



	void ProcessClient(int id);
	void AcceptThread();
};

