#include <windows.h>
#include <string>
#include "patch.h"
#include "inject.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);

	// add in something random so the next parameter is the actual switch
	std::string sCmd = "AnthIste-Loader ";
	// pass on the parameters to the warcraft exe
	sCmd += lpCmdLine;

	try {

		// ------------------------------------------------------------------------
		// Create the process
		// ------------------------------------------------------------------------

		if (!CreateProcessA("war3.exe",
			(LPSTR)sCmd.c_str(),
			0,
			0,
			false,
			CREATE_SUSPENDED,
			0,
			0,
			&si,
			&pi)) throw std::exception("Could not launch Warcraft III");

		// ------------------------------------------------------------------------
		// Patch warcraft for pvpgn and inject the antihack
		// ------------------------------------------------------------------------

		PatchWarcraft(pi.hProcess);
		InjectAntihack(pi.hProcess);

	}
	catch (std::exception e) {
		std::string s = e.what();

		char lpMsgBuf[500] = {0};
		FormatMessageA( 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			lpMsgBuf,
			sizeof(lpMsgBuf),
			NULL 
		);

		s += " \n\nThe error returned by the system was:\n";
		s += (char*)lpMsgBuf;

		MessageBoxA(0, s.c_str(), "Antihack", MB_ICONERROR | MB_OK);
		LocalFree(lpMsgBuf);

		TerminateProcess(pi.hProcess, 0);
	
		return 0;
	}

	if (pi.hThread) {
		ResumeThread(pi.hThread);
	}

	return 0;
}