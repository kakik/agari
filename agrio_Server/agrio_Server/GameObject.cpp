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