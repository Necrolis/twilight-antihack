#ifndef INC_DIRECTX
#define INC_DIRECTX

#include <d3d8.h>
#include <d3dx8.h>
#include <string>
#include <fstream>
#include <tlhelp32.h>
#include <list>

#include "singleton.h"
#include "hooks.h"
#include "message.h"
#include "cvar.h"
#include "anticheat.h"
#include "color.h"

// -------------------------------------------------------------------------------
// CD3DManager
// Stores pointer to the direct3d device and manages all graphics calls
// Access using static Get() method
// -------------------------------------------------------------------------------

class CD3DManager: public CSingleton<CD3DManager> {
	public:
		enum PrintFlags_t { PF_MESSAGES = 1,
							PF_CVARS = PF_MESSAGES << 1 };

	public:
		void SetDevice(IDirect3DDevice8* device);
		int Update();
		int OnLostDevice();
		int OnResetDevice();
		int ReloadFont();
		int TakeScreenshot(std::string file_name);
		int PrintMessage(std::string msg, int duration = 5000, DWORD color = 0xFFDCF000);
		int TextOut(int x, int y, std::string text, DWORD color);
		template <typename T>
		int AddCvar(std::string name, T* pObject);
		template <typename T>
		int AddCvar(std::string name, T pObject);
		template <typename T>
		int RemoveCvar(std::string name);
		int CalculateFPS();
		int SetScreenDimensions(int x, int y, bool fullscreen);
		int GetScreenDimension(int* x, int* y, bool* fullscreen);
		int Fullscreen();
		int TogglePrintFlags(int flags);
	
	private:
		int PutPixel(int x, int y, D3DCOLOR color);

	private:
		friend class CSingleton<CD3DManager>;
		CD3DManager();	// private constructor, use accessor
		~CD3DManager();

		std::list<CMessage> messages;
		std::list<CCvarBase*> cvars;

		IDirect3DDevice8* pDevice;
		LPD3DXFONT m_font;

		int fps;
		int screenWidth, screenHeight;
		bool bFullscreen;

		int printFlags;
};

template <typename T>
int CD3DManager::AddCvar(std::string name, T* pObject)
{
	ThreadRequestResources();

	CCvar<T>* cvar = new CCvar<T>(name, pObject);
	cvars.push_back(cvar);

	ThreadReleaseResources();

	return 1;
}

template <typename T>
int CD3DManager::AddCvar(std::string name, T pObject)
{
	ThreadRequestResources();

	CCvar<T>* cvar = new CCvar<T>(name, pObject);
	cvars.push_back(cvar);

	ThreadReleaseResources();

	return 1;
}

template <typename T>
int CD3DManager::RemoveCvar(std::string name)
{
	ThreadRequestResources();

	std::list<CCvarBase*> dead;
	std::list<CCvarBase*>::iterator i;

	for (i = cvars.begin(); i != cvars.end(); ++i) {
		if ( ((CCvar<T>*)(*i))->name == name) {
			dead.push_back(*i);
		}
	}

	for (i = dead.begin(); i != dead.end(); ++i) {
		cvars.remove(*i);
	}

	ThreadReleaseResources();

	return 1;
}

#endif