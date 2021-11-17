#pragma once

class Client {
	SOCKET sock;
	int id;
public:
	Client() {
		//array 기본생성자
		
	}
	Client(SOCKET socket, int clientId) : id(clientId) {
		sock = socket;
	}
	void Send(void* Packet) const;
	void Recv();
};