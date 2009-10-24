#ifndef INC_CONNECTION
#define INC_CONNECTION

#include <windows.h>
#include <winsock.h>
#include <string>
#include <list>
#include "packet.h"

#pragma comment(lib, "wsock32.lib")

class CConnection {
	public:
		CConnection();
		~CConnection();

		int Init(SOCKET s);
		int Shutdown(SOCKET s);
		int Connected();

		int SendBuffer(void* buffer, unsigned int size);
		int ReceiveBuffer(void* buffer, unsigned int size);
		int SendPacket(PACKET* packet);

	private:
		SOCKET hSocket;
		int connected;
};

#endif