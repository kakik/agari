#pragma once

const short PLAYER_WIDTH = 18;
const short PLAYER_HEIGHT = 30;

struct Coordinate
{
	short x, y;
};
class GameObject
{
public:
	char direction;
	float velocity;
	unsigned short width, height;
	unsigned char id;
	char sprite;

	bool isMove = false;
public:
	bool isActive = false;
	Coordinate pos;
	unsigned char GetId() const {
		return id;
	}

	void Move(void* pk);
	void SetBullet(void* pk);
	void SetBox(void* pk);
	void Update(float elapsedTime,char* buf, int& buf_start);
	void IsCollision(GameObject* other);
};


class Player : public GameObject
{
public:

	char curEquip;
	STATE state;
	short hp;
	short items[8];
	SOCKET sock;

public:
	Player();

	~Player();

	void SetSockId(SOCKET socket, int clientId) {
		id = clientId;
		sock = socket;
	};
	void Send(void* Packet) const;
	bool Recv();

	void ChangeState(void* pk);
	void UseItem(void* pk);

	void ChangeHP(short hp);
	void SendLogIn();


};