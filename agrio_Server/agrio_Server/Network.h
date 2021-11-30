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
//상수
const short BULLET_WIDTH = 11;
const short BULLET_HEIGHT = 11;
const short BLOCK_WIDTH = 30;
const short BLOCK_HEIGHT = 30;
const short WINDOW_WIDTH = 900 * 2;      //윈도우 x사이즈
const short WINDOW_HEIGHT = 800 * 2;
const float VELOCITY = 300.0f;

enum Scene {
	GAMESTART, GAMEEND, PLAYERDIE
};
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
		instance = this;

		for (int i = 0; i < MAX_USER; ++i) {
			GameObjects.push_back(new Player);
		}
		for (int i = MAX_USER; i < MAX_OBJECT; ++i) {
			GameObjects.push_back(new GameObject);
		}
		{
			short objlength = 90;
			GameObjects[4]->isActive = true;
			GameObjects[4]->pos = Coordinate{ WINDOW_WIDTH / 2,0 };
			GameObjects[4]->width = WINDOW_WIDTH;
			GameObjects[4]->height = objlength;
			GameObjects[4]->sprite = (int)SPRITE::box;
			GameObjects[4]->id = 4;

			GameObjects[5]->isActive = true;
			GameObjects[5]->pos = Coordinate{ WINDOW_WIDTH / 2, short(WINDOW_HEIGHT - objlength * 2) };
			GameObjects[5]->width = WINDOW_WIDTH;
			GameObjects[5]->height = objlength;
			GameObjects[5]->sprite = (int)SPRITE::box;
			GameObjects[5]->id = 5;

			GameObjects[6]->isActive = true;
			GameObjects[6]->pos = Coordinate{ objlength,WINDOW_HEIGHT / 2 };
			GameObjects[6]->width = objlength;
			GameObjects[6]->height = WINDOW_HEIGHT;
			GameObjects[6]->sprite = (int)SPRITE::box;
			GameObjects[6]->id = 6;

			GameObjects[7]->isActive = true;
			GameObjects[7]->pos = Coordinate{ short(WINDOW_WIDTH - objlength),WINDOW_HEIGHT / 2 };
			GameObjects[7]->width = objlength;
			GameObjects[7]->height = WINDOW_HEIGHT;
			GameObjects[7]->sprite = (int)SPRITE::box;
			GameObjects[7]->id = 7;
		}
		for (int i = 8; i < 20; ++i) {
			GameObjects[i]->isActive = true;
			GameObjects[i]->pos = Coordinate{ short(BLOCK_WIDTH * rand()% WINDOW_WIDTH),  short(BLOCK_HEIGHT* rand() % WINDOW_HEIGHT) };
			GameObjects[i]->width = BLOCK_WIDTH;
			GameObjects[i]->height = BLOCK_HEIGHT;
			GameObjects[i]->sprite = (int)SPRITE::box;
			GameObjects[i]->id = i;
		}
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
	void GameStart();
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

		RECT aRect{ a->pos.x - a->width / 2, a->pos.y - a->height / 2,a->pos.x + a->width / 2,  a->pos.y + a->height };
		RECT bRect{ b->pos.x - b->width / 2, b->pos.y - b->height / 2,b->pos.x + b->width / 2,  b->pos.y + b->height };

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

