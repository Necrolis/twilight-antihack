#include "hooks.h"

// -------------------------------------------------------------------------------
// Uses a pointer to a class with virtual methods and changes the items in the
// vtable to point to the hooked function
// -------------------------------------------------------------------------------

int DetourFunctionWithVtable(void* pObject, int offset, DWORD& pOriginalFunction, void* pDetourFunction)
{
	if (!pDetourFunction || !pObject || offset < 0) return 0;

	DWORD* pVtable = (DWORD*)*((DWORD*)pObject); // get 1st 4 bytes of object : pointer to vtable

	// set memory protection
	DWORD dwOldProtect = 0;
	if ( !VirtualProtect((void*)&pVtable[offset], sizeof(DWORD), PAGE_EXECUTE_READWRITE, &dwOldProtect) )
		return 0;

	pOriginalFunction = pVtable[offset]; // store original address
	pVtable[offset] = (DWORD)pDetourFunction; // overwrite pointer in vtable

	// restore protection
	if ( !VirtualProtect((void*)&pVtable[offset], sizeof(DWORD), dwOldProtect, &dwOldProtect) )
		return 0;

	return 1;
}

// -------------------------------------------------------------------------------
// Removes any file path junk before the name of the actual file
// -------------------------------------------------------------------------------

void StripFilePath(std::string& path)
{
	std::string::size_type pos = 0;

	for (unsigned int k = 0; k < path.length(); ++k) {
		if (path[k] == '\\') pos = k;
	}

	if (pos) {
		path = path.substr(pos+1, path.length());
	}
}

// -------------------------------------------------------------------------------
// LoadLibrary hook
// Intercepts all modules that get loaded dynamically, specifically d3d8.dll
// -------------------------------------------------------------------------------

LoadLibrary_t orig_LoadLibrary;
HMODULE WINAPI hook_LoadLibrary ( LPCSTR lpFileName )
{		
	std::string sModule = lpFileName;
	StripFilePath(sModule);

	// -------------------------------------------------------------------------------
	// Check unsafe (not whitelisted) modules and don't load them
	// -------------------------------------------------------------------------------

	CAntiCheat::Get()->ThreadRequestResources();
	CD3DManager::Get()->ThreadRequestResources();

	if (!CAntiCheat::Get()->ModuleIsSafe(sModule)) {
		if (CAntiCheat::Get()->ModuleIsBlack(sModule)) {
			CD3DManager::Get()->PrintMessage(sModule + " is hax you haxor!", 15000);
			CAntiCheat::Get()->ReportViolation(VIOLATION_BLACKLIST_MODULE);
		} else {
			CD3DManager::Get()->PrintMessage(sModule + " not loaded, unsafe!", 15000);
		}

		// prevent modules that get loaded more than once to be recorded more than once

		static std::list<std::string> loaded;

		std::list<std::string>::iterator i;
		i = std::find(loaded.begin(), loaded.end(), sModule);

		if (i == loaded.end()) {
			loaded.push_back(sModule);
			std::ofstream of;
			of.open("anticheat.txt", std::ios::app);
			of << "Unsafe module: \"" << sModule << "\"," << std::endl;
			of.close();
		}

		CD3DManager::Get()->ThreadReleaseResources();
		CAntiCheat::Get()->ThreadReleaseResources();

		return 0;
	}

	CD3DManager::Get()->ThreadReleaseResources();
	CAntiCheat::Get()->ThreadReleaseResources();

	// -------------------------------------------------------------------------------
	// Wait for d3d8.dll to be loaded and hook Direct3DCreate8
	// -------------------------------------------------------------------------------

    HMODULE hM = orig_LoadLibrary( lpFileName );

	static int hooked = 0;
    if (sModule == "d3d8.dll") 
    {
        hooked++;

        if (hooked == 3) {
            // get address of function to hook
            pDirect3DCreate8 = (PBYTE)GetProcAddress(hM, "Direct3DCreate8");
			orig_Direct3DCreate8 = (Direct3DCreate8_t)DetourFunction(pDirect3DCreate8, (PBYTE)hook_Direct3DCreate8);
        }
    }

    return hM;
}

// -------------------------------------------------------------------------------
// Direct3DCreate8 hook
// Use created IDirect3D interface to hook CreateDevice
// -------------------------------------------------------------------------------

PBYTE pDirect3DCreate8 = 0;
Direct3DCreate8_t orig_Direct3DCreate8;
IDirect3D8* __stdcall hook_Direct3DCreate8(UINT SDKVersion)
{
	IDirect3D8* d3d = orig_Direct3DCreate8(SDKVersion);
	DetourFunctionWithVtable((void*)d3d, 15, (DWORD&)orig_CreateDevice, (void*)hook_CreateDevice);

	return d3d;
}

// -------------------------------------------------------------------------------
// CreateDevice hook
// Used to get a pointer to the created IDirect3DDevice8 interface
// Stores screen resolution passed in the PresentationParameters
// -------------------------------------------------------------------------------

CreateDevice_t orig_CreateDevice;
HRESULT APIENTRY hook_CreateDevice(IDirect3D8* pInterface, UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice8** ppReturnedDeviceInterface)
{
    HRESULT ret = orig_CreateDevice(pInterface, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	DetourFunctionWithVtable((void*)(*ppReturnedDeviceInterface), 35, (DWORD&)orig_EndScene, (void*)hook_EndScene);
	DetourFunctionWithVtable((void*)(*ppReturnedDeviceInterface), 14, (DWORD&)orig_Reset, (void*)hook_Reset);
	
	CD3DManager::Get()->SetDevice(*ppReturnedDeviceInterface);
	
	if (pPresentationParameters->Windowed) {
		CD3DManager::Get()->SetScreenDimensions(800, 600, false);
	} else {
		CD3DManager::Get()->SetScreenDimensions(pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, true);
	}

    return ret;
}

// -------------------------------------------------------------------------------
// Endscene hook
// All rendering calls to be done here
// -------------------------------------------------------------------------------

EndScene_t orig_EndScene;
HRESULT APIENTRY hook_EndScene(IDirect3DDevice8* pInterface)
{
	CD3DManager::Get()->Update();

	return orig_EndScene(pInterface);
}

// -------------------------------------------------------------------------------
// Reset hook
// Ensures that all created textures etc are reset to make alt-tab not break
// -------------------------------------------------------------------------------

Reset_t orig_Reset;
HRESULT APIENTRY hook_Reset(IDirect3DDevice8* pInterface, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	CD3DManager::Get()->OnLostDevice();
	HRESULT ret = orig_Reset(pInterface, pPresentationParameters);
	CD3DManager::Get()->OnResetDevice();
	CD3DManager::Get()->ReloadFont();

	if (pPresentationParameters->Windowed) {
		CD3DManager::Get()->SetScreenDimensions(800, 600, false);
	} else {
		CD3DManager::Get()->SetScreenDimensions(pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight, true);
	}

	return ret;
}

// -------------------------------------------------------------------------------
// Module32Next hook
// A (weak) form of module hiding that only protects from other modules scanning
// for our dll from within the current process, using toolhelp32 API's :(
// -------------------------------------------------------------------------------

Module32Next_t orig_Module32Next;
BOOL WINAPI hook_Module32Next(HANDLE hSnapshot, LPMODULEENTRY32 lppe)
{
	BOOL res = orig_Module32Next(hSnapshot, lppe);

	std::stringstream ss;
	std::string s;

	ss << lppe->szModule;
	ss >> s;

	if (s == "antihack.dll") {
		return orig_Module32Next(hSnapshot, lppe);
	} else {
		return res;
	}
}

// -------------------------------------------------------------------------------
// socket hook
// grabs a handle to warcraft's socket
// Edit: Redundant for now, war3 creates 10 billion sockets. Send() is used to
// find the correct one.
// This is currently unhooked, add an entry in dllmain if you want to hook it.
// Note that a simple hook seems to get optimised and breaks, add some more
// meat to the body of the hook if you want it to work
// -------------------------------------------------------------------------------

socket_t orig_socket;
SOCKET __stdcall hook_socket(int af, int type, int protocol)
{
	return orig_socket(af, type, protocol);
}

// -------------------------------------------------------------------------------
// closesocket hook
// passes on the socket to the anticheat
// -------------------------------------------------------------------------------

closesocket_t orig_closesocket;
int __stdcall hook_closesocket(SOCKET s)
{
	if (CAntiCheat::Get()->ShutDownWinsock(s)) {
		CD3DManager::Get()->PrintMessage("Connection closed");
	}

	return orig_closesocket(s);
}

// -------------------------------------------------------------------------------
// send hook
// intercepts packets
// -------------------------------------------------------------------------------

send_t orig_send;
int __stdcall hook_send(SOCKET s, const char* buf, int len, int flags)
{
	t_bnet_header* h = (t_bnet_header*)buf;
	t_client_antihack_handshake* handshake = (t_client_antihack_handshake*)buf;

	// bnet_logged_in message, so its the correct socket to use for antihack stuff
	if (h->type == (short)0x53ff) {
		if (CAntiCheat::Get()->InitialiseWinsock(s)) {
			CD3DManager::Get()->PrintMessage("Connection established");
		}
	}

	return orig_send(s, buf, len, flags);
}

// -------------------------------------------------------------------------------
// recv hook
// intercepts packets
// -------------------------------------------------------------------------------

recv_t orig_recv;
int __stdcall hook_recv(SOCKET s, const char* buf, int len, int flags)
{
	std::stringstream ss;
	std::string msg;

	t_bnet_header* h = (t_bnet_header*)buf;
	if (h->type = CLIENT_ANTIHACK_HANDSHAKE) {
		// whatever
	}

	return orig_recv(s, buf, len, flags);
}