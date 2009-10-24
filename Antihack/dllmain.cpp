#include <windows.h>
#include <algorithm>
#include <vector>
#include "anticheat.h"
#include "hooks.h"

//Antihack needs the following features:
//- hash the code setion to check for any modifications		[x]
//- check for any extra loaded modules (whitelist?)			[x]
//- communication with server								[-]
//
//Protection:
//- checking for debugger									[ ]
//- module hiding?											[-]
//- run warcraft with loader and inject						[x]
//- maybe pack the code										[ ]
//- inject into explorer.exe and monitor ah?				[ ]

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			{
				// ------------------------------------------------------------------------
				// Load up the antihack for the first time (its a singleton)
				// ------------------------------------------------------------------------

				try {
					CAntiCheat::Get();
				}
				catch (std::exception e) {
					MessageBoxA(0, e.what(), "Anticheat", MB_ICONERROR | MB_OK);

					return FALSE;
				}

				// ------------------------------------------------------------------------
				// Hook all the relevant API calls
				// ------------------------------------------------------------------------

				orig_LoadLibrary = (LoadLibrary_t)DetourFunction((LPBYTE) LoadLibraryA, (LPBYTE) hook_LoadLibrary );
				orig_Module32Next = (Module32Next_t)DetourFunction((LPBYTE) Module32Next, (LPBYTE) hook_Module32Next );
				//orig_send = (send_t)DetourFunction((LPBYTE) send, (LPBYTE) hook_send );
				//orig_recv = (recv_t)DetourFunction((LPBYTE) recv, (LPBYTE) hook_recv );
				//orig_socket = (socket_t)DetourFunction((LPBYTE) socket, (LPBYTE) hook_socket );
				//orig_closesocket = (closesocket_t)DetourFunction((LPBYTE) closesocket, (LPBYTE) hook_closesocket );

				break;
			}

		case DLL_PROCESS_DETACH:
			{
				// Nah, this baby isnt supposed to get unloaded
				// Destructor of AntiCheat takes care of all cleanup

				break;
			}
	}

	return TRUE;
}