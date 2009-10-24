#ifndef INC_MESSAGE
#define INC_MESSAGE

#include <windows.h>
#include <string>
#include <math.h>
#include "color.h"

class CMessage {
	public:
		CMessage(std::string msg, int duration, DWORD color);
		bool HasExpired();
		bool operator==(const CMessage& a) {
			return (duration == a.duration && oldTime == a.oldTime && msg == a.msg);
		}

	private:
		int duration;
		int oldTime;

	public:
		std::string msg;
		DWORD color;
};

#endif