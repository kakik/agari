#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>

#include "Protocol.h"
#include "Client.h"
#include "GameObject.h"


void err_quit(const char* msg);
void err_display(const char* msg);

//상수
const short BULLET_WIDTH = 11;
const short BULLET_HEIGHT = 11;
const short BLOCK_WIDTH = 30;
const short BLOCK_HEIGHT = 30;

const short WINDOW_WIDTH = 900 * 2;      //윈도우 x사이즈
const short WINDOW_HEIGHT = 800 * 2;
const float VELOCITY = 300.0f;
extern short CUR_WINDOW_WIDTH;      //윈도우 x사이즈
extern short CUR_WINDOW_HEIGHT;
extern short CUR_WINDOW_START_X;
extern short CUR_WINDOW_START_Y;
const int WALL_ID_UP = 4;
const int WALL_ID_DOWN = 5;
const int WALL_ID_LEFT = 6;
const int WALL_ID_RIGHT = 7;
enum class Scene {
	title, lobby, stage1, gameover, winner
};

class Network
{
	static Network* instance;
	char buf[BUFSIZE*4];

	int ItemSpawnTime;
	std::chrono::system_clock::time_point preItemSpawnTime;
public:

	std::vector<std::thread> threads;
	std::vector<GameObject*> GameObjects;

	SOCKET listen_sock;
	static Network* GetInstance();
	Scene MyScene = Scene::lobby;

	Network() {
		assert(instance == nullptr);
		instance = this;
		ItemSpawnTime = 1;
		for (int i = 0; i < MAX_USER; ++i) {
			GameObjects.push_back(new Player);
		}
		for (int i = MAX_USER; i < MAX_OBJECT; ++i) {
			GameObjects.push_back(new GameObject);
		}
		{
			short objlength = 100;
			// 위
			GameObjects[WALL_ID_UP]->isActive = true;
			GameObjects[WALL_ID_UP]->pos = Coordinate{ WINDOW_WIDTH / 2,0 };
			GameObjects[WALL_ID_UP]->width = WINDOW_WIDTH;
			GameObjects[WALL_ID_UP]->height = objlength;
			GameObjects[WALL_ID_UP]->sprite = (int)SPRITE::wallRow;
			GameObjects[WALL_ID_UP]->type = WALL;
			GameObjects[WALL_ID_UP]->id = 4;

			GameObjects[WALL_ID_DOWN]->isActive = true;
			GameObjects[WALL_ID_DOWN]->pos = Coordinate{ WINDOW_WIDTH / 2, WINDOW_HEIGHT };
			GameObjects[WALL_ID_DOWN]->width = WINDOW_WIDTH;
			GameObjects[WALL_ID_DOWN]->height = objlength;
			GameObjects[WALL_ID_DOWN]->sprite = (int)SPRITE::wallRow;
			GameObjects[WALL_ID_DOWN]->type = WALL;
			GameObjects[WALL_ID_DOWN]->id = 5;

			GameObjects[WALL_ID_LEFT]->isActive = true;
			GameObjects[WALL_ID_LEFT]->pos = Coordinate{ 0, WINDOW_HEIGHT / 2 };
			GameObjects[WALL_ID_LEFT]->width = objlength;
			GameObjects[WALL_ID_LEFT]->height = WINDOW_HEIGHT;
			GameObjects[WALL_ID_LEFT]->sprite = (int)SPRITE::wallCol;
			GameObjects[WALL_ID_LEFT]->type = WALL;
			GameObjects[WALL_ID_LEFT]->id = 6;

			GameObjects[WALL_ID_RIGHT]->isActive = true;
			GameObjects[WALL_ID_RIGHT]->pos = Coordinate{ WINDOW_WIDTH, WINDOW_HEIGHT / 2 };
			GameObjects[WALL_ID_RIGHT]->width = objlength;
			GameObjects[WALL_ID_RIGHT]->height = WINDOW_HEIGHT;
			GameObjects[WALL_ID_RIGHT]->sprite = (int)SPRITE::wallCol;
			GameObjects[WALL_ID_RIGHT]->type = WALL;
			GameObjects[WALL_ID_RIGHT]->id = 7;
		}
		for (int i = 8; i < 20; ++i) {
			GameObjects[i]->isActive = true;
			GameObjects[i]->pos = Coordinate{ short(BLOCK_WIDTH * rand()% WINDOW_WIDTH),  short(BLOCK_HEIGHT* rand() % WINDOW_HEIGHT) };
			GameObjects[i]->width = BLOCK_WIDTH;
			GameObjects[i]->height = BLOCK_HEIGHT;
			GameObjects[i]->sprite = (int)SPRITE::box;
			GameObjects[i]->type = BOX;
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
		threads.emplace_back(&Network::LobbyThread, this);
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

	void SendLoginOk(int id);
	void SendPutObj(int id, int target);
	void SendMoveObj(int id, int mover);
	void SendChangeState(int id, int target);
	void SendChangeHp(int id, int target);
	void SendChangeScene(int id, char snum);
	void SendRemoveObj(int id, int victm);
	void SendGetItem(int id, int victm);
	void SendChangeWeapon(int id, int target);
	void Update(float elapsedTime);



	void ProcessClient(int id);
	void LobbyThread();
};

