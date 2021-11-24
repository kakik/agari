#include "stdfx.h"
#include "Network.h"

using namespace std;
using namespace chrono;
//	서버 주소
const short SEVER_PORT = 8000;
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
		net->StartAccept();

		int nClient = 0;
		nClient++;
		
		auto pre_t = system_clock::now();
		while (1) {
			auto cur_t = system_clock::now();
			float elapsed = duration_cast<chrono::milliseconds>(cur_t - pre_t).count()/float(1000);
			net->Update(elapsed);
			
			pre_t = cur_t;
			if (system_clock::now() - pre_t < 32ms) {
				this_thread::sleep_for(32ms - (system_clock::now() - cur_t));
				//cout << "update " << chrono::duration_cast<chrono::milliseconds>(end_t - start_t).count() << "ms\n";
			}
		}
	}



}