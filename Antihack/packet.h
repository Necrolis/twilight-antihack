#ifndef INC_PACKET
#define INC_PACKET

#include <windows.h>

class PACKET { 
	enum { PING = 0,
		   PONG = PING << 1 };
};

// -----------------------------------------------------------------------

typedef struct
{
    short type;
    short size;
} t_bnet_header;

// Twilight
#define CLIENT_ANTIHACK_HANDSHAKE (short)0x80ff
typedef struct{
	t_bnet_header		h;
	DWORD				something_special; // special identifier for security ? (4 bytes) 
} t_client_antihack_handshake;

#define CLIENT_ANTIHACK_VIOLATION (short)0x81ff
enum {
	VIOLATION_BAD_CHECKSUM = 0,
	VIOLATION_BLACKLIST_MODULE,
	VIOLATION_DEBUGGER_PRESENT
};
typedef struct{
	t_bnet_header		h;
	DWORD				violation; // violation code
} t_client_antihack_violation;

// -----------------------------------------------------------------------

#endif