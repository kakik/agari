#include "stdfx.h"
#include "Network.h"
#include "GameObject.h"

void GameObject::Update(float elapsedTime, char* buf, int& bufStart)
{
	if (isMove) {
		sc_packet_move_obj& pk = *reinterpret_cast<sc_packet_move_obj*>(buf + bufStart);
		pk.packetSize = sizeof(pk);
		pk.packetType = SC_PACKET_MOVE_OBJ;
		pk.lookDir = direction;
		pk.objectID = id;

		short speed = static_cast<short>(velocity * elapsedTime);
		int x = pos.x;
		int y = pos.y;

		switch (direction)
		{
		case (char)DIR::N:
			//pos.x += 1;
			y -= speed;
			break;
		case (char)DIR::NE:
			x += speed;
			y -= speed;
			break;

		case (char)DIR::NW:
			x -= speed;
			y -= speed;
			break;

		case (char)DIR::S:
			//pos.x += 1;
			y += speed;
			break;

		case (char)DIR::SE:
			x += speed;
			y += speed;
			break;

		case (char)DIR::SW:
			x -= speed;
			y += speed;
			break;

		case (char)DIR::E:
			x += speed;
			break;

		case (char)DIR::W:
			x -= speed;
			break;

		default:
			break;
		}
		//충돌일 경우 원래 위치로 돌리기위해 잠깐 사용
		pk.x = pos.x;
		pk.y = pos.y;

		pos.x = x;
		pos.y = y;
		Network* net = Network::GetInstance();

		if (type == PLAYER && net->MyScene == SCENE::lobby) {
			if ((830 < pk.x && 970 > pk.x) && (650 < pk.y && 780 > pk.y))
				reinterpret_cast<Player*>(this)->isReady = true;
			else
				reinterpret_cast<Player*>(this)->isReady = false;
		}

		for (auto* obj : net->GameObjects) {
			if (false == obj->isActive)continue;
			if (id == obj->id)continue;
			if (net->IsCollision(id, obj->id)) {
				//충돌처리 해줄 것
				if (type == PLAYER) {

					switch (obj->type)
					{
					case BULLET:
					{
						reinterpret_cast<Player*>(this)->ChangeHp(ATTACKHP);

						GameObject* Object = (net->GameObjects[obj->id]);
						Object->isActive = false;
						Object->isMove = false;
						for (int i = 0; i < MAX_USER; ++i) {
							if (false == Network::GetInstance()->GameObjects[i]->isActive) continue;
							net->SendChangeHp(i, id);
							net->SendRemoveObj(i, obj->id);
							// 체력이 0이 되면
							if (reinterpret_cast<Player*>(this)->hp <= 0) {
								CUR_WINDOW_START_X = WINDOW_WIDTH / 4 + 15;
								CUR_WINDOW_START_Y = WINDOW_HEIGHT / 4 + 15;
								CUR_WINDOW_WIDTH *= 0.75;
								CUR_WINDOW_HEIGHT *= 0.75;
								net->GameObjects[WALL_ID_UP]->pos.y = WINDOW_HEIGHT / 4;
								net->SendMoveObj(i, WALL_ID_UP);
								net->GameObjects[WALL_ID_DOWN]->pos.y = WINDOW_HEIGHT / 4 * 3;
								net->SendMoveObj(i, WALL_ID_DOWN);

								net->GameObjects[WALL_ID_LEFT]->pos.x = WINDOW_WIDTH / 4 * 3;
								net->SendMoveObj(i, WALL_ID_LEFT);
								net->GameObjects[WALL_ID_RIGHT]->pos.x = WINDOW_WIDTH / 4;
								net->SendMoveObj(i, WALL_ID_RIGHT);
								for (int j = WALL_ID_RIGHT; j < MAX_OBJECT; ++j) {
									//net->SendRemoveObj(i, j);
								}
								net->SendRemoveObj(i, id);
								net->SendChangeScene(id, (char)SCENE::gameover);
							}

						}
					}
					break;
					case ITEM:
					{
						int item = (net->GameObjects[obj->id])->sprite - (int)SPRITE::uiPistol;
						if (item <= shotgun)
							reinterpret_cast<Player*>(net->GameObjects[id])->items[item] += 5;
						else
							reinterpret_cast<Player*>(net->GameObjects[id])->items[item] += 1;
						net->GameObjects[obj->id]->isActive = false;
						for (int i = 0; i < MAX_USER; ++i) {
							if (false == net->GameObjects[i]->isActive) continue;
							net->SendRemoveObj(i, obj->id);
						}
						net->SendGetItem(id, item);
					}
					break;
					case WALL:
					case BOX:
						pos.x = pk.x;
						pos.y = pk.y;
						break;
					default:
						break;
					}

				}
				else {// 플레이어가 아닌(총알, 아이템, 벽)오브젝트가 충돌했을 때 충돌타입이(obj->type)이라면
					switch (type) {
					case BOX:
					case WALL:
						break;
					case BULLET:
						if (obj->type == PLAYER) {
							reinterpret_cast<Player*>(net->GameObjects[obj->id])->ChangeHp(ATTACKHP);

							GameObject* Object = (net->GameObjects[id]);
							Object->isActive = false;
							Object->isMove = false;
							for (int i = 0; i < MAX_USER; ++i) {
								if (false == net->GameObjects[i]->isActive) continue;
								net->SendChangeHp(i, obj->id);
								net->SendRemoveObj(i, id);
								// 체력이 0이 되면
								if (reinterpret_cast<Player*>(net->GameObjects[obj->id])->hp <= 0) {
									CUR_WINDOW_START_X = WINDOW_WIDTH / 4 + 15;
									CUR_WINDOW_START_Y = WINDOW_HEIGHT / 4 + 15;
									CUR_WINDOW_WIDTH *= 0.75;
									CUR_WINDOW_HEIGHT *= 0.75;
									net->GameObjects[WALL_ID_UP]->pos.y = WINDOW_HEIGHT / 4;
									net->SendMoveObj(i, WALL_ID_UP);
									net->GameObjects[WALL_ID_DOWN]->pos.y = WINDOW_HEIGHT / 4 * 3;
									net->SendMoveObj(i, WALL_ID_DOWN);

									net->GameObjects[WALL_ID_LEFT]->pos.x = WINDOW_WIDTH / 4 * 3;
									net->SendMoveObj(i, WALL_ID_LEFT);
									net->GameObjects[WALL_ID_RIGHT]->pos.x = WINDOW_WIDTH / 4;
									net->SendMoveObj(i, WALL_ID_RIGHT);


									net->SendRemoveObj(i, obj->id);
									net->SendChangeScene(obj->id, (char)Scene::gameover);
								}
							}
						}
						else if (obj->type == WALL) {

							if (collisionCount > 2) {
								for (int i = 0; i < MAX_USER; ++i) {
									if (false == net->GameObjects[i]->isActive) continue;
									net->SendRemoveObj(i, id);
								}
							}
							else
							{
								collisionCount++;
								switch (direction)
								{
								case (int)DIR::N:
								case (int)DIR::E:
								case (int)DIR::S:
								case (int)DIR::W:
									direction = (direction + 4) % 8;
									break;
								default:
									direction = (direction + 2) % 8;
									break;
								}
							}


						}
						else if (obj->type == BOX) {
							if (collisionCount > 1) {
								for (int i = 0; i < MAX_USER; ++i) {
									if (false == net->GameObjects[i]->isActive) continue;
									net->SendRemoveObj(i, id);
								}
							}
							else
							{
								collisionCount++;
								switch (direction)
								{
								case (int)DIR::N:
								case (int)DIR::E:
								case (int)DIR::S:
								case (int)DIR::W:
									direction = (direction + 4) % 8;
									break;
								default:
									direction = (direction + 2) % 8;
									break;
								}
							}
							if (obj->collisionCount > 1) {
								for (int i = 0; i < MAX_USER; ++i) {
									if (false == net->GameObjects[i]->isActive) continue;
									net->SendRemoveObj(i, obj->id);
								}
							}
							else
							{
 								obj->collisionCount++;

							}
						}
						break;
					}
					
				}
			}
		}
		pk.x = pos.x;
		pk.y = pos.y;
		bufStart += sizeof(pk);
	}
}


void Player::UpdateBuf(void* Packet, int packSize) {
	buf_lock.lock();
	memcpy(eventPacketBuf + bufSize, Packet, packSize);
	bufSize += packSize;
	buf_lock.unlock();
}
void Player::Send(void* buf, int bufSize)
{
	int retval = send(sock, reinterpret_cast<char*>(buf), bufSize, 0);
	if (retval == SOCKET_ERROR) {
		std::cout << "오류 발생" << (int)id << std::endl;
	}
	std::cout << "[TCP 서버]" << (int)id << " : " << retval << "바이트 보냈습니다\n";
}

void SetBulletPos(DIR direction, Coordinate& pos, short dist)
{
	switch (direction)
	{
	case DIR::N:
		pos.y -= dist;
		break;
	case DIR::NE:
		pos.x += dist;
		pos.y -= dist;
		break;

	case DIR::NW:
		pos.x -= dist;
		pos.y -= dist;
		break;

	case DIR::S:
		pos.y += dist;
		break;

	case DIR::SE:
		pos.x += dist;
		pos.y += dist;
		break;

	case DIR::SW:
		pos.x -= dist;
		pos.y += dist;
		break;

	case DIR::E:
		pos.x += dist;
		break;

	case DIR::W:
		pos.x -= dist;
		break;

	default:
		break;
	}
}

void SetPistol(int obj_id, char direction, Coordinate pos) {
	if (direction == -1) direction = 7;
	GameObject* pistol = Network::GetInstance()->GameObjects[obj_id];
	pistol->direction = direction;
	pistol->velocity = VELOCITY;
	pistol->width = BULLET_WIDTH;
	pistol->height = BULLET_HEIGHT;
	pistol->id = obj_id;
	pistol->sprite = (char)SPRITE::bulletN + direction;
	pistol->type = BULLET;
	pistol->isActive = true;
	pistol->isMove = true;
	pistol->pos = pos;
	SetBulletPos((DIR)pistol->direction, pistol->pos, 50);
}
void SetBox(int obj_id, char direction, Coordinate pos) {
	if (direction == -1) direction = 7;
	GameObject* pistol = Network::GetInstance()->GameObjects[obj_id];
	pistol->direction = direction;
	pistol->velocity = VELOCITY;
	pistol->width = BLOCK_WIDTH;
	pistol->height = BLOCK_HEIGHT;
	pistol->id = obj_id;
	pistol->sprite = (char)SPRITE::box;
	pistol->type = BOX;
	pistol->isActive = true;
	pistol->isMove = false;
	pistol->pos = pos;
	SetBulletPos((DIR)pistol->direction, pistol->pos, 50);
}
bool Player::Recv() {
	Network* net = Network::GetInstance();
	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		net->Disconnect(id);
		err_display("recv()");
		return false;
	}
	std::cout << "Size : " << (int)pkSize.packetSize << std::endl;
	std::cout << "Type : " << (int)pkSize.packetType << std::endl;

	switch (pkSize.packetType)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login recvPacket;
		retval = recv(sock, reinterpret_cast<char*>((&recvPacket)) + 2, pkSize.packetSize - 2, MSG_WAITALL);


		std::cout << "id : " << (int)id << std::endl;
		sprite = recvPacket.playerSkin;
		isActive = true;
		direction = (char)DIR::N;
		type = PLAYER;
		velocity = PLAYER_SPEED;
		pos.x = (short)800;
		pos.y = (short)900;
		width = PLAYER_WIDTH;
		height = PLAYER_HEIGHT;

		net->SendLoginOk(id);
		/*
		* 각 클라이언트들한테 새로운 플레이어가 접속했으니 플레이어 오브젝트를 생성하라고함
		*/
		for (int i = 0; i < MAX_USER; ++i) {
			Player* p = reinterpret_cast<Player*>(net->GameObjects[i]);
			if (false == p->isActive) continue;
			if (id == i) continue;
			net->SendPutObj(i, id);
		}

		/*
		* 새로 접속한 클라이언트에게 현재 그려야할 플레이어를 알려줌
		*/
		for (const auto Client : net->GameObjects) {
			if (false == Client->isActive) continue;
			if (id == Client->GetId()) continue;
			net->SendPutObj(id, Client->GetId());
		}

	}
	break;
	case CS_PACKET_PLAYER_MOVE:
	{
		cs_packet_player_move recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		/*
		* 각 클라이언트들한테 플레이어가 이동했으니 해당 플레이어 오브젝트를 이동 하라고함
		*/
		direction = (char)recvPacket.dir;
	}
	break;
	case CS_PACKET_PLAYER_STATE:
	{
		cs_packet_player_state recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		switch (recvPacket.playerState)
		{
		case (char)STATE::idle:
		{
			isMove = false;
			isAttack = false;
			state = STATE::idle;
		}
		break;

		case (char)STATE::move:
		{
			isMove = true;
			state = STATE::move;
		}
		break;
		case (char)STATE::attack:
		{
			state = STATE::attack;
			//nMagazine++;
		}
		break;
		default:
			break;
		}
		for (int i = 0; i < MAX_USER; ++i)
		{
			Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
			if (false == player->isActive) continue;
			net->SendChangeState(i, id);
		}
	}
	break;
	case CS_PACKET_SHOOT_BULLET:
	{
		cs_packet_shoot_bullet recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		if (items[curEquip-1] <= 0) break;
		//nMagazine++;
		items[curEquip - 1]--;

		if (curEquip == shotgun) {
			for (int i = -1; i < 2; ++i) {
				int obj_id = net->GetObjID();
				if (obj_id == -1) {
					std::cout << "모든 오브젝트를 사용하였습니다." << std::endl;
					break;
				}
				SetPistol(obj_id, (net->GameObjects[recvPacket.playerID]->direction+i)%8, net->GameObjects[recvPacket.playerID]->pos);
				for (int i = 0; i < MAX_USER; ++i) {
					Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
					if (false == player->isActive) continue;
					//if (id == i) continue;
					net->SendPutObj(i, obj_id);
				}
			}
		}
		else {
			int obj_id = net->GetObjID();
			if (obj_id == -1) {
				std::cout << "모든 오브젝트를 사용하였습니다." << std::endl;
				break;
			}
			SetPistol(obj_id, net->GameObjects[recvPacket.playerID]->direction, net->GameObjects[recvPacket.playerID]->pos);
			for (int i = 0; i < MAX_USER; ++i) {
				Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
				if (false == player->isActive) continue;
				//if (id == i) continue;
				net->SendPutObj(i, obj_id);
			}
		}
		
		/*
		* 각 클라이언트들한테 플레이어가 총을 발사 해당 플레이어 오브젝트를 Render 하라고함
		*/
		

	}
	break;
	case CS_PACKET_USED_ITEM:
	{
		cs_packet_used_item recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		switch (recvPacket.itemNum)
		{
		case (char)ITEM::pistol:
		case (char)ITEM::uzi:
		case (char)ITEM::shotgun:
			curEquip = (char)recvPacket.itemNum;
			for (int i = 0; i < MAX_USER; ++i) {
				if (false == net->GameObjects[i]->isActive) continue;
				net->SendChangeWeapon(i, id);
			}
			break;

		case (char)ITEM::potion:
		{
			if (items[potion-1] <= 0) break;
			ChangeHp(HEALING);
			/*
			* 각 클라이언트들한테 플레이어가 포션을 사용하였으므로 해당 플레이어의 체력을 Chage 하라고함
			*/
			for (int i = 0; i < MAX_USER; ++i) {
				if (false == net->GameObjects[i]->isActive) continue;
				net->SendChangeHp(i, id);
			}

		}
		break;
		case (char)ITEM::box:
		{
			if (items[box -1 ] <= 0) break;
			int obj_id = net->GetObjID();
			if (obj_id == -1) {
				std::cout << "모든 오브젝트를 사용하였습니다." << std::endl;
				break;
			}
			SetBox(obj_id, (net->GameObjects[id]->direction), net->GameObjects[id]->pos);
			for (int i = 0; i < MAX_USER; ++i) {
				Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
				if (false == player->isActive) continue;
				//if (id == i) continue;
				net->SendPutObj(i, obj_id);
			}
		}
		break;
		default:
			break;
		}
	}
	break;
	default:
		std::cout << "잘못된 패킷 전송";
		break;
	}
	return true;
}