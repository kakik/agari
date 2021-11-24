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
	void Disconnect(int id) {
		Player* p = reinterpret_cast<Player*>(GameObjects[id]);
		p->isActive = false;
		p->isMove = false;

		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) continue;
			SendRemoveObj(i, id);
		}

		closesocket(p->sock);
		threads[id].join();
	}
	char GetPlayerId() {
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) return i;
		}
		std::cout << "can not return player id" << std::endl;
		return -1;
	}
	char GetObjID() {
		for (int i = MAX_USER; i < MAX_OBJECT; ++i) {
			if (false == GameObjects[i]->isActive) return i;
		}
		std::cout << "can not return object id" << std::endl;
		return -1;
	}
	bool IsPlayer(int id) {
		return (id >= 0) && (id < 3);
	}
	void StartAccept() {
		threads.emplace_back(&Network::AcceptThread, this);
	}
	bool IsCollision(int a_id, int b_id) {
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
	void SendPutObj(int id, int target);
	void SendMoveObj(int id, int mover);
	void SendChangeState(int id, int target);
	void SendChangeHp(int id, int target);
	void SendRemoveObj(int id, int victm);
	void Update(float elapsedTime);



	void ProcessClient(int id);
	void AcceptThread();
};

