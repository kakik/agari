#include "stdfx.h"

#include "Network.h"


Network* Network::instance = nullptr;

Network* Network::GetInstance()
{
	return instance;
}
void Network::ProcessClient(int id)
{
	Player* client = reinterpret_cast<Player*>(GameObjects[id]);
	
	while (client->Recv());

	return;
}
void Network::GameStart() {
	sc_packet_change_scene pk;
	pk.packetSize = sizeof(pk);
	pk.packetType = SC_PACKET_CHANGE_SCENE;
	pk.sceneNum = GAMESTART;// 게임 시작에 해당하는 번호
	for (int i = 0; i < MAX_USER; ++i) {
		if (false == GameObjects[i]->isActive) continue;
		reinterpret_cast<Player*>(GameObjects[i])->Send(buf, pk.packetSize);
	}
	for (int i = 8; i < 20; ++i) {
		GameObjects[i]->isActive = true;
		GameObjects[i]->pos = Coordinate{ short(BLOCK_WIDTH * rand() % WINDOW_WIDTH),  short(BLOCK_HEIGHT * rand() % WINDOW_HEIGHT) };
		GameObjects[i]->width = BLOCK_WIDTH;
		GameObjects[i]->height = BLOCK_HEIGHT;
		GameObjects[i]->sprite = (int)SPRITE::box;
		GameObjects[i]->id = i;
	}
}
void Network::AcceptThread() {
	SOCKADDR_IN ClientAddr;
	int addrlen;
	int id = 0;
	while (true) {
		
		id = GetPlayerId();
		if (id == -1) {
			
			std::this_thread::sleep_for(std::chrono::seconds(1));
			// 게임 시작 패킷 보내고
			// 게임시작
			// 게임 중에는 accept를 받지 않음, 게임이 시작하면 accpet를 다시 시작하자
			// return;
			continue;
		}
		addrlen = sizeof(ClientAddr);
		SOCKET client_sock = accept(listen_sock, (SOCKADDR*)&ClientAddr, &addrlen);

		GameObjects[id]->isActive = true;
		GameObjects[id]->id = id;
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			return;
		}
		std::cout << "\n[TCP 서버] 클라이언트 접속: IP 주소=" << inet_ntoa(ClientAddr.sin_addr)
			<< "포트 번호=" << ntohs(ClientAddr.sin_port) << std::endl;

		reinterpret_cast<Player*> (GameObjects[id])->SetSockId(client_sock, id);


		threads.emplace_back(&Network::ProcessClient, Network::GetInstance(), id);

	}
}
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL
	);
	MessageBoxA(NULL, (LPCSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL
	);
	MessageBoxA(NULL, (LPCSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
	
	LocalFree(lpMsgBuf);
}
void Network::SendLoginOk(int id) {
	sc_packet_login_ok sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_LOGIN_OK;
	sendPacket.playerID = id;
	sendPacket.x = (short)800;
	sendPacket.y = (short)900;
	sendPacket.width = PLAYER_WIDTH;
	sendPacket.height = PLAYER_HEIGHT;
	
	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendPutObj(int id, const int target) {
	sc_packet_put_obj sendPutPacket;
	sendPutPacket.packetSize = sizeof(sendPutPacket);
	sendPutPacket.packetType = SC_PACKET_PUT_OBJ;
	sendPutPacket.x = GameObjects[target]->pos.x;
	sendPutPacket.y = GameObjects[target]->pos.y;
	sendPutPacket.width = GameObjects[target]->width;
	sendPutPacket.height = GameObjects[target]->height;
	sendPutPacket.objectID = target;
	sendPutPacket.sprite = GameObjects[target]->sprite;
	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPutPacket, sendPutPacket.packetSize);
}

void Network::SendMoveObj(int id, int mover) {
	sc_packet_move_obj sendMovePacket;
	sendMovePacket.packetSize = sizeof(sendMovePacket);
	sendMovePacket.packetType = SC_PACKET_MOVE_OBJ;
	sendMovePacket.objectID = mover;
	sendMovePacket.lookDir = GameObjects[mover]->direction;

	sendMovePacket.x = GameObjects[mover]->pos.x;
	sendMovePacket.y = GameObjects[mover]->pos.y;
	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendMovePacket, sendMovePacket.packetSize);
}

void Network::SendChangeState(int id, int target) {
	sc_packet_player_state sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_PLAYER_STATE;
	sendPacket.objectID = target;
	sendPacket.playerState = (char)(reinterpret_cast<Player*>(GameObjects[target])->state);

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}

void Network::SendChangeHp(int id, int target) {
	sc_packet_change_hp sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_CHANGE_HP;
	sendPacket.playerID = target;
	sendPacket.hp = reinterpret_cast<Player*>(GameObjects[target])->hp;

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}

void Network::SendRemoveObj(int id, int victm) {
	sc_packet_remove_obj sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_REMOVE_OBJ;
	sendPacket.objectID = victm;
	GameObjects[victm]->collisionCount = 0;
	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendGetItem(int id, int itemtype) {
	sc_packet_get_item sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_GET_ITEM;
	sendPacket.playerID = id;
	sendPacket.itemID = itemtype;

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}

void Network::Update(float elapsedTime) {
	int bufstart = 0;
	for (auto obj : GameObjects) {
		//if (!obj)continue;
		if (false == obj->isActive) continue;
		obj->Update(elapsedTime, buf, bufstart);
		
	}

	if (std::chrono::system_clock::now() - preItemSpawnTime > std::chrono::seconds(ItemSpawnTime))
	{
		preItemSpawnTime = std::chrono::system_clock::now();

		for (int i = 0; i < MAX_USER; ++i) {
			Player* p = reinterpret_cast<Player*>(GameObjects[i]);
			if (false == p->isActive) continue;

			int obj_id = GetObjID();
			GameObject* pistol = GameObjects[obj_id];
			pistol->direction = rand() % 8;
			pistol->velocity = VELOCITY;
			pistol->width = BULLET_WIDTH;
			pistol->height = BULLET_HEIGHT;
			pistol->id = obj_id;
			pistol->sprite = (char)SPRITE::uiPistol + rand()%5;
			pistol->type = ITEM;
			pistol->isActive = true;
			pistol->isMove = false;
			pistol->pos = Coordinate{ short(rand() % WINDOW_WIDTH), short(rand() % WINDOW_HEIGHT) };

			SendPutObj(i, obj_id);
		}

	}

	//플레이어의 이벤트 버퍼에 있는 내용을 전송버퍼로 옮김
	for (int i = 0; i < MAX_USER; ++i) {
		Player* p = reinterpret_cast<Player*>(GameObjects[i]);
		if (false == p->isActive) continue;
		if (0 >= p->bufSize) continue;
		
		memcpy(buf + bufstart, p->eventPacketBuf, p->bufSize);
		bufstart += p->bufSize;
		p->bufSize = 0;
	}
	


	
	if (0 < bufstart)
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) continue;
			reinterpret_cast<Player*>(GameObjects[i])->Send(buf, bufstart);
		}
	//플레이어가 한 프레임 마다 생성된 버퍼 전송
}