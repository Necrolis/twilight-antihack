#include "patch.h"

void PatchWarcraft(HANDLE hProc)
{
	const unsigned char patch1[] = {0x77, 0x33, 0x6C, 0x68, 0x2E, 0x64, 0x6C, 0x6C};
	const unsigned char patch2[] = {0x00};
	DWORD dwOldProtect[2] = {0, 0};

	// Remove memory protection

	if (!VirtualProtectEx(hProc, (void*)0x00456B64, sizeof(patch1), PAGE_EXECUTE_READWRITE, &dwOldProtect[0]) ||
		!VirtualProtectEx(hProc, (void*)0x0040016F, sizeof(patch2), PAGE_EXECUTE_READWRITE, &dwOldProtect[1]))
			throw std::exception("Could not remove page protection");

	// Write patch

	if (!WriteProcessMemory(hProc, (void*)0x00456B64, patch1, sizeof(patch1), 0) ||
		!WriteProcessMemory(hProc, (void*)0x0040016F, patch2, sizeof(patch2), 0))
			throw std::exception("Could not patch warcraft for pvpgn use");
			 
	// Restore protection

	VirtualProtectEx(hProc, (void*)0x00456B64, sizeof(patch1), dwOldProtect[0], &dwOldProtect[0]);
	VirtualProtectEx(hProc, (void*)0x0040016F, sizeof(patch2), dwOldProtect[1], &dwOldProtect[1]);

}