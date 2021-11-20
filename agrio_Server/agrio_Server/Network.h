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

const int MAX_OBJECT = 100;

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
		for (int i = 0; i < MAX_USER; ++i) {
			GameObjects.push_back(new Player);
		}
		for (int i = MAX_USER; i < MAX_OBJECT; ++i) {
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
		p->isActive = false;
		p->isMove = false;

		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[id]->isActive) continue;
			if (id == i) continue;
			send_remove_obj(i, id);
		}
		closesocket(p->sock);
	}
	char get_player_id() {
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) return i;
		}
		std::cout << "can not return player id" << std::endl;
		return -1;
	}
	char get_obj_id() {
		for (int i = MAX_USER; i < MAX_OBJECT; ++i) {
			if (false == GameObjects[i]->isActive) return i;
		}
		std::cout << "can not return object id" << std::endl;
		return -1;
	}
	bool is_player(int id) {
		return (id >= 0) && (id < 4);
	}
	void start_accept() {
		threads.emplace_back(&Network::AcceptThread, this);
	}
	bool is_collision(int a_id, int b_id) {
		GameObject* a = GameObjects[a_id];
		GameObject* b = GameObjects[b_id];

		RECT aRect{ a->pos.x - a->width / 2, a->pos.y - a->width / 2,a->pos.x + a->width / 2,  a->pos.y + a->width };
		RECT bRect{ b->pos.x - b->width / 2, b->pos.y - b->width / 2,b->pos.x + b->width / 2,  b->pos.y + b->width };

		RECT tmp;
		if (IntersectRect(&tmp,&aRect, &bRect))
			return true;
		else
			return false;
	}
	void send_put_obj(int id, int target);
	void send_move_obj(int id, int mover);
	void send_change_state(int id, int target);
	void send_remove_obj(int id, int victm);
	void update(float elapsedTime);



	void ProcessClient(int id);
	void AcceptThread();
};

