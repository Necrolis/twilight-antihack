#include "inject.h"

void InjectAntihack(HANDLE hProc)
{
	if (!hProc) throw std::exception("Invalid process handle");

	HMODULE hModule = GetModuleHandleA("kernel32.dll");
	if (!hModule) throw std::exception("Could not acquire module of kernel32.dll");

	FARPROC pLoadLibrary = GetProcAddress(hModule, "LoadLibraryA");
	if (!pLoadLibrary) throw std::exception("Could not acquire address of LoadLibraryA");

	const std::string dllName = "antihack.dll";
	int size = dllName.length()+1;

	void* pRemoteAddress = VirtualAllocEx(hProc, 0, size, MEM_COMMIT, PAGE_READWRITE);
	if (!pRemoteAddress) throw std::exception("Could not allocate remote memory");

	if ( !WriteProcessMemory(hProc, pRemoteAddress, (void*)dllName.c_str(), size, 0) )
		throw std::exception("Could not write remote string");

	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteAddress, 0, 0);
	if (!hThread) throw std::exception("Could not start remote thread");

	WaitForSingleObject(hThread, INFINITE);
	DWORD dwExit = 0;
	GetExitCodeThread(hThread, &dwExit);

	if (!dwExit) throw std::exception("Error while loading remote dll");

	if ( !VirtualFreeEx(hProc, pRemoteAddress, size, MEM_DECOMMIT) )
		throw std::exception("Could not free remote memory");
}