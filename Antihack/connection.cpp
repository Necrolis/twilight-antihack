#include "connection.h"

// -------------------------------------------------------------------------------
// CONSTRUCTOR / DESTRUCTOR:
// -------------------------------------------------------------------------------

CConnection::CConnection()
{
	connected = false;
	hSocket = 0;
}

CConnection::~CConnection()
{

}

// -------------------------------------------------------------------------------
// COMMUNICATION:
// -------------------------------------------------------------------------------

int CConnection::Init(SOCKET s)
{
	if (connected) return 0;

	hSocket = s;
	connected = true;

	return 1;
}

int CConnection::Shutdown(SOCKET s)
{
	if (!connected || hSocket != s) return 0;

	hSocket = 0;
	connected = false;

	return 1;
}

int CConnection::Connected()
{
	return connected;
}

int CConnection::SendBuffer(void* buffer, unsigned int size)
{
	if (!connected) return 0;

	int ret = 0;
	int pos = 0;
	do {
		ret = send(hSocket, (const char*)((int)buffer + pos), size-pos, MSG_PARTIAL);
		if (ret == -1) {
			Shutdown(hSocket);
			break;
		} else {
			pos += ret;
		}
	} while (ret < size);

	return ret;
}

int CConnection::ReceiveBuffer(void* buffer, unsigned int size)
{
	if (!connected) return 0;

	return 1;
}

int CConnection::SendPacket(PACKET* packet)
{
	if (!connected) return 0;

	return 1;
}