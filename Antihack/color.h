#ifndef INC_COLOR
#define INC_COLOR

#include <windows.h>

struct ARGB {
	unsigned char a;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct Color {
	Color() : color(0) {}
	Color(DWORD color) : color(color) {}
	operator DWORD() { return color; }
	void operator =(DWORD dw) { color = dw;}

	union {
		ARGB argb;
		DWORD color;
	};
};

#endif