#ifndef INC_GAMEINFO
#define INC_GAMEINFO

#include <string>

namespace GameInfo {

extern const std::string colorStrings[];

const int MAX_PLAYERS = 12;
struct CPlayer {
	private:
		unsigned char filler[0x78];
	public:
		int gold;
};

struct CHero {
	private:
		unsigned char filler[0x8C];
	public:
		int xp;
};

struct CHeroPos {
	unsigned char filler[0x78];
	float x;
	float y;
};

}

#endif