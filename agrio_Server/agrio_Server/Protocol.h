#pragma once
//	서버 주소
const short SEVER_PORT = 4000;
const char* SEVER_ADDR = "127.0.0.1";

//	최대 접속 가능 클라이언트
const char MAX_USER = 3;

//	총알 발사 방향
enum class DIR { N, NE, E, SE, S, SW, W, NW };

//	아이템 ID
enum ITEM { pistol, uzi, shotgun, potion, box };

//	패킷 타입(Client->Server)
const char CS_PACKET_LOGIN = 1;
const char CS_PACKET_PLAYER_MOVE = 2;
const char CS_PACKET_PLAYER_STATE = 3;
const char CS_PACKET_SHOOT_BULLET = 4;
const char CS_PACKET_USED_ITEM = 5;

//	패킷 타입(Server->Client)
const char SC_PACKET_LOGIN_OK = 1;
const char SC_PACKET_CHANGE_SCENE = 2;
const char SC_PACKET_OBJ_MOVE = 3;
const char SC_PACKET_PLAYER_STATE = 4;
const char SC_PACKET_PUT_OBJ = 5;
const char SC_PACKET_REMOVE_OBJ = 6;
const char SC_PACKET_CHANGE_HP = 7;
const char SC_PACKET_GET_ITEM = 8;
const char SC_PACKET_ITEM_COUNT = 9;
const char SC_PACKET_CHAGE_WEAPON = 10;

//	패킷 정의(Client->Server)
#pragma pack(push,1)
struct packet {
	unsigned char packetSize;
	char packetType;
};
struct cs_packet_login : packet {
	char playerSkin;
};
struct cs_packet_player_move : packet {
	char dir;
};
struct cs_packet_player_state : packet {
	char playerState;
};
struct cs_packet_shoot_bullet : packet {
	char shootX, shootY;
	char dir;
};
struct cs_packet_used_item : packet {
	char itemNum;
};

//	패킷 정의(Server->Client)
struct sc_packet_login_ok : packet {
	char playerID;
	char x, y;
};
struct sc_packet_change_scene : packet {
	char sceneNum;
};
struct sc_packet_obj_move : packet {
	char objectID;
	char lookDir;
	char x, y;
};
struct sc_packet_player_state : packet {
	char objectID;
	char playerState;
};
struct sc_packet_put_obj : packet {
	char objectID;
	char sprite;
	char x, y;
	unsigned char width, height;
};
struct sc_packet_remove_obj : packet {
	char objectID;
};
struct sc_packet_change_hp : packet {
	char playerID;
	unsigned char hp;
};
struct sc_packet_get_item : packet {
	char playerID;
	char itemID;
};

struct sc_packet_item_count : packet {
	char playerID;
	char itemID;
	char itemCount;
};
struct sc_packet_change_weapon : packet {
	char playerID;
	char gunID; // == itemID
};
#pragma pop()