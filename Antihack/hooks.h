#ifndef INC_DETOURS_H
#define INC_DETOURS_H

#include <windows.h>
#include <tlhelp32.h>
#include <detours.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <winsock.h>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "directx.h"
#include "anticheat.h"
#include "connection.h"

#pragma comment(lib, "detours.lib")

// Helper Functions --------------------------------------------------------------------------------------------

int DetourFunctionWithVtable(void* pObject, int offset, DWORD& pOriginalFunction, void* pDetourFunction);
void StripFilePath(std::string& path);

// LoadLibrary Hook --------------------------------------------------------------------------------------------

typedef HMODULE (WINAPI *LoadLibrary_t)(LPCSTR);
extern LoadLibrary_t orig_LoadLibrary; // holds address of original non-detoured function
HMODULE WINAPI hook_LoadLibrary ( LPCSTR lpFileName );

// Direct3DCreate8 Hook ----------------------------------------------------------------------------------------

typedef IDirect3D8* (__stdcall *Direct3DCreate8_t)(UINT);
extern Direct3DCreate8_t orig_Direct3DCreate8;
IDirect3D8* __stdcall hook_Direct3DCreate8(UINT SDKVersion);
// Holds address that we get in our LoadLibrary hook (used for detour)
extern PBYTE pDirect3DCreate8;

// CreateDevice Hook (Offset: 15) ------------------------------------------------------------------------------

typedef HRESULT (APIENTRY *CreateDevice_t)(IDirect3D8*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice8**);
extern CreateDevice_t orig_CreateDevice;
HRESULT APIENTRY hook_CreateDevice(IDirect3D8* pInterface, UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice8** pg_pReturnedDeviceInterface);

// Endscene Hook (Offset: 35) ----------------------------------------------------------------------------------

typedef HRESULT (APIENTRY *EndScene_t)(IDirect3DDevice8*);
HRESULT APIENTRY hook_EndScene(IDirect3DDevice8* pInterface);
extern EndScene_t orig_EndScene;

// Reset Hook (Offset: 14) ----------------------------------------------------------------------------------

typedef HRESULT (APIENTRY *Reset_t)(IDirect3DDevice8* pInterface, D3DPRESENT_PARAMETERS* pPresentationParameters);
HRESULT APIENTRY hook_Reset(IDirect3DDevice8* pInterface, D3DPRESENT_PARAMETERS* pPresentationParameters);
extern Reset_t orig_Reset;

// Process32Next
typedef BOOL (WINAPI *Module32Next_t)(HANDLE hSnapshot, LPMODULEENTRY32 lppe);
BOOL WINAPI hook_Module32Next(HANDLE hSnapshot, LPMODULEENTRY32 lppe);
extern Module32Next_t orig_Module32Next;

// socket
typedef SOCKET (__stdcall *socket_t)(int af, int type, int protocol);
SOCKET __stdcall hook_socket(int af, int type, int protocol);
extern socket_t orig_socket;

// closesocket
typedef int (__stdcall *closesocket_t)(SOCKET s);
int __stdcall hook_closesocket(SOCKET s);
extern closesocket_t orig_closesocket;

// send
typedef int (__stdcall *send_t)(SOCKET s, const char* buf, int len, int flags);
int __stdcall hook_send(SOCKET s, const char* buf, int len, int flags);
extern send_t orig_send;

// recv
typedef int (__stdcall *recv_t)(SOCKET s, const char* buf, int len, int flags);
int __stdcall hook_recv(SOCKET s, const char* buf, int len, int flags);
extern recv_t orig_recv;

#endif