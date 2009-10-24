#include "message.h"

CMessage::CMessage(std::string msg, int duration, DWORD color)
{
	this->msg = msg;
	this->duration = duration;
	oldTime = GetTickCount();

	this->color = color;
}

bool CMessage::HasExpired()
{
	if (duration == INFINITE) return false;

	int deltaTime = GetTickCount() - oldTime;
	int threshhold = duration / 2; // how long it should wait before fading

	DWORD alpha = 0xFF; // default
						// 0x000000AA

	if (deltaTime >= threshhold) {
		// I cant remember how I came up with this formula but it gives good results
		alpha = static_cast<unsigned char>(0xFF - sin((double)((deltaTime - threshhold) / (double)(duration - threshhold))*1.5)*0xFF);
	}

	alpha <<= 24; // set to 0xAA000000
	color = color&0x00FFFFFF; // filter out alpha channel
							  // 0x00RRGGBB
	color |= alpha; // blend in new alpha
					// 0xAARRGGBB
	
	return (deltaTime >= duration);
}