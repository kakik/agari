#include "stdfx.h"
#include "Network.h"

using namespace std;

//	서버 주소
const short SEVER_PORT = 4000;
const char* SEVER_ADDR = "127.0.0.1";

Network network;

int main()
{
	int retval;

	{
		Network* net = Network::GetInstance();
		//대기 소캣
		net->listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (net->listen_sock == INVALID_SOCKET) err_quit("socket()");

		//bind
		SOCKADDR_IN ServerAddr;
		ZeroMemory(&ServerAddr, sizeof(ServerAddr));
		ServerAddr.sin_family = AF_INET;
		ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		ServerAddr.sin_port = htons(SEVER_PORT);
		retval = bind(net->listen_sock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
		if (retval == SOCKET_ERROR) err_quit("bind()");

		//listen
		retval = listen(net->listen_sock, SOMAXCONN);
		if (retval == SOCKET_ERROR) err_quit("listen()");

		//접속 받기 시작
		net->start_accept();

		int nClient = 0;
		nClient++;
		while (1) {
	

			net->update();
			
		}
	}



}