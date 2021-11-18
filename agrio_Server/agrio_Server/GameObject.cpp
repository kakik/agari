#include "stdfx.h"
#include "Network.h"
#include "GameObject.h"


void GameObject::Update(char* buf, int& bufStart)
{
	sc_packet_move_obj pk;
	pk.lookDir = direction;
	pk.objectID = id;
	pk.packetSize = sizeof(pk);
	pk.packetType = SC_PACKET_MOVE_OBJ;
	pk.x = 1;
	pk.y = 1;

	memcpy(buf + bufStart, &pk, sizeof(pk));

	bufStart += sizeof(pk);
}


void Player::Send(void* Packet) const
{

	int retval = send(sock, reinterpret_cast<char*>(Packet), reinterpret_cast<packet*>(Packet)->packetSize, 0);
	std::cout << "[TCP 서버]" << retval << "바이트 보냈습니다\n";

}

void Player::Recv() {
	Network* net = Network::GetInstance();
	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}
	std::cout << "Size : " << (int)pkSize.packetSize << std::endl;
	std::cout << "Type : " << (int)pkSize.packetType << std::endl;

	switch (pkSize.packetType)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login recvPecket;
		retval = recv(sock, reinterpret_cast<char*>((&recvPecket)) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , Skin : " << (int)recvPecket.playerSkin << std::endl;

		net->send_login_ok(id);
		/*
		* 새로 접속한 클라이언트에게 현재 그려야할 플레이어를 알려줌
		*/
		for (const auto Client : net->GameObjects) {
			net->send_put_obj(id, Client->GetId());
		}
		/*
		* 각 클라이언트들한테 새로운 플레이어가 접속했으니 플레이어 오브젝트를 생성하라고함
		*/
		for (const auto& Client : net->GameObjects) {
			if(net->is_player(Client->GetId())) continue;

			net->send_put_obj(Client->GetId(), id);
			
		}
	}
	break;
	case CS_PACKET_PLAYER_MOVE:
	{
		cs_packet_player_move recvPecket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPecket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , cs_packet_player_move : " << (int)recvPecket.dir << std::endl;
		/*
		* 각 클라이언트들한테 플레이어가 이동했으니 해당 플레이어 오브젝트를 이동 하라고함
		*/
		direction = (char)recvPecket.dir;
		switch (direction)
		{
		case (char)DIR::N:
			//pos.x += 1;
			pos.y += 1;
			break;
		case (char)DIR::NE:
			pos.x += 1;
			pos.y -= 1;
			break;

		case (char)DIR::NW:
			pos.x -= 1;
			pos.y -= 1;
			break;

		case (char)DIR::S:
			//pos.x += 1;
			pos.y += 1;
			break;

		case (char)DIR::SE:
			pos.x += 1;
			pos.y -= 1;
			break;

		case (char)DIR::SW:
			pos.x -= 1;
			pos.y -= 1;
			break;

		case (char)DIR::E:
			pos.x += 1;
			break;

		case (char)DIR::W:
			pos.x -= 1;
			break;

		default:
			break;
		}

		for (int i = 0; i < MAX_USER; ++i) {
			net->send_move_obj(i, id);
		}
	}
	break;
	case CS_PACKET_PLAYER_STATE:
	{

	}
	break;
	case CS_PACKET_SHOOT_BULLET:
	{
		cs_packet_shoot_bullet recvPecket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPecket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , cs_packet_player_move : " << (int)recvPecket.shootX << " " << (int)recvPecket.shootY << " " << (int)recvPecket.dir << std::endl;
		/*
		* 각 클라이언트들한테 플레이어가 총을 발사 해당 플레이어 오브젝트를 Render 하라고함
		*/
		//for (const auto& Client : net->GameObjects) {
		//	if (Client.id != id) {//
		//		//net->send_move_obj(Client.id);
		//	}
		//}

	}
	break;
	case CS_PACKET_USED_ITEM:
	{

	}
	break;
	default:
		std::cout << "잘못된 패킷 전송";
		break;
	}
}