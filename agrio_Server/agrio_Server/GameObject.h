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
	char curEquip;
	char state;
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
	void Recv();

	void ChangeState(void* pk);
	void UseItem(void* pk);

	void ChangeHP(short hp);
	void SendLogIn() {
		sc_packet_login_ok sendPacket;
		sendPacket.packetSize = sizeof(sendPacket);
		sendPacket.packetType = SC_PACKET_LOGIN_OK;
		id = sendPacket.playerID = id;
		pos.x = sendPacket.x = (short)800;
		pos.y = sendPacket.y = (short)900;
		width = sendPacket.width = PLAYER_WIDTH;
		height = sendPacket.height = PLAYER_HEIGHT;

		Send(&sendPacket);
	}


};