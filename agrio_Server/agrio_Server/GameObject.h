#pragma once

struct Coordinate
{
	short x, y;
}; 
class GameObject
{
	Coordinate pos;
	char direction;
	float velocity;
	unsigned char width, height;
	unsigned char id;
	bool isActive;

public:
	void SetBullet(void* pk);
	void SetBox(void* pk);
	void Update(char* buf, int& buf_start);
	void IsCollision(GameObject* other);
};

class Player : public GameObject
{
	char curEquip;
	char state;
	short hp;
	short items[8];
public:
	Player();
	~Player();

	void ChangeState(void* pk);
	void UseItem(void* pk);
	void Move(void* pk);
	void ChangeHP(short hp);
	void LogIn(void* pk);

};