#ifndef INC_ANTICHEAT
#define INC_ANTICHEAT

#include <windows.h>
#include <tlhelp32.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "singleton.h"
#include "connection.h"
#include "md5.h"
#include "hotspot.h"
#include "directx.h"
#include "whitelist.h"
#include "blacklist.h"
#include "checksums.h"
#include "gameinfo.h"

#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "d3dx8.lib")

class CD3DManager;
class CAntiCheat;

using GameInfo::CHero;
using GameInfo::CHeroPos;
using GameInfo::CPlayer;

// -------------------------------------------------------------------------------
// CAntiCheat
// Manages all anti-cheat methods / data
// Access using static Get() method
// -------------------------------------------------------------------------------

class CAntiCheat: public CSingleton<CAntiCheat> {
	public:
		// CONSTRUCTOR / DESTRUCTOR:
		CAntiCheat();
		~CAntiCheat();

		// THREADS:
		static DWORD WINAPI thread_CheckHotspots(LPVOID param);
		static DWORD WINAPI thread_ChecksumCode(LPVOID param);
		static DWORD WINAPI thread_CheckLoadedModules(LPVOID param);
		static DWORD WINAPI thread_StayAlivePackets(LPVOID param);
		static DWORD WINAPI thread_ReceiveCommands(LPVOID param);
		static DWORD WINAPI thread_AntiDebug(LPVOID param);

		// COMMUNICATION:
		int InitialiseWinsock(SOCKET s);
		int ShutDownWinsock(SOCKET s);

		// PROTECTION:
		int ShutDownWarcraft();
		int HideModule();
		int AntiDebug();

		// ANTI-CHEAT:
		int ScanHotspot(const HOTSPOT& hotspot);
		int CodeHasValidChecksum();
		MD5_CTX MD5_Section(int base, int size);
		int LogHash(int base, int size, unsigned char* hash);
		int TakeScreenshot();
		int ModuleIsSafe(std::string moduleName);
		int ModuleIsBlack(std::string moduleName);
		int EnumerateModules();
		int BuildWhitelist(std::string filename);
		int ReportViolation(int code);
		DWORD GetCurrentThreadId();
		int SuspendThreadById(DWORD threadId);
		int ResumeThreadById(DWORD threadId);
		int EnumerateThreads(int (CAntiCheat::*callback)(DWORD threadId));

		// GENERAL:
		int FreeAllResources();
		int LogBegin();
		int LogText(std::string s);
		int LogEnd();
		int GetDeepPointer(int base, int argc, ...);

	private:
		// THREADS:
		std::vector<HANDLE> vThreads; // Vector to keep track of vThreads

		// LOGGING:
		std::ofstream log;

		// WINSOCK
		CConnection connection;

		// DIRECTX
		CD3DManager* d3d;

		// HACK ?
		CPlayer* players[GameInfo::MAX_PLAYERS];
};

#endif // INC_ANTICHEAT
