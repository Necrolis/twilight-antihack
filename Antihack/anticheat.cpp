#include "anticheat.h"

// -------------------------------------------------------------------------------
// CONSTRUCTOR / DESTRUCTOR:
// -------------------------------------------------------------------------------

CAntiCheat::CAntiCheat()
{
	// Set up rendering
	d3d = CD3DManager::Get(); // access it for the firt time: constructor is called
	d3d->PrintMessage("Antihack loading...", 15000, D3DCOLOR_XRGB(0, 0, 255));

	// Create vThreads

	vThreads.push_back(CreateThread(0, 0, thread_CheckHotspots, (void*)this, 0, 0));
	vThreads.push_back(CreateThread(0, 0, thread_ChecksumCode, (void*)this, 0, 0));
	vThreads.push_back(CreateThread(0, 0, thread_CheckLoadedModules, (void*)this, 0, 0));
	vThreads.push_back(CreateThread(0, 0, thread_ReceiveCommands, (void*)this, 0, 0));
	vThreads.push_back(CreateThread(0, 0, thread_StayAlivePackets, (void*)this, 0, 0));
	vThreads.push_back(CreateThread(0, 0, thread_AntiDebug, (void*)this, 0, 0));

	d3d->AddCvar("Info", std::string("[F5] Take screenshot"));
	d3d->AddCvar("Info", std::string("[F8] Toggle message display"));
	d3d->AddCvar("Info", std::string("[F9] Toggle this display"));
}

CAntiCheat::~CAntiCheat()
{
	FreeAllResources();
}

// -------------------------------------------------------------------------------
// THREADS:
// -------------------------------------------------------------------------------

DWORD WINAPI CAntiCheat::thread_CheckHotspots(LPVOID param)
{
	CAntiCheat* p = (CAntiCheat*)param;

	while (1) {
		p->ThreadRequestResources();

		if ( p->ScanHotspot(HOTSPOT((char*)0x455FFF, 1, "\x00")) ) {
			p->LogBegin();
			p->log << "Illegal code detected!" << std::endl;
			p->LogEnd();

			p->d3d->PrintMessage("Illegal code detected!", 5000, D3DCOLOR_XRGB(255, 0, 0));
		}

		p->ThreadReleaseResources();
		Sleep(2000);
	}

	return TRUE;
}

DWORD WINAPI CAntiCheat::thread_ChecksumCode(LPVOID param)
{
	CAntiCheat* p = (CAntiCheat*)param;

	// wait for all modules to get loaded
	// Fixme: Check if a module is loaded before scanning it
	Sleep(5000);

	std::string message;
	DWORD color;

	while (1) {
		p->ThreadRequestResources();
		p->EnumerateThreads(&CAntiCheat::SuspendThreadById);

		if ( !p->CodeHasValidChecksum() ) {
			message = "Illegal code detected!";
			color = 0xFFFF0000;

			p->LogBegin();
			p->log << message << std::endl << std::endl;
			p->LogEnd();

			p->ReportViolation(VIOLATION_BAD_CHECKSUM);
		} else {
			message = "Code seems clean!";
			color = 0xFF00FF00;
		}

		p->d3d->PrintMessage(message, 5000, color);

		p->EnumerateThreads(&CAntiCheat::ResumeThreadById);
		p->ThreadReleaseResources();
		Sleep(10000);
	}

	return TRUE;
}

DWORD WINAPI CAntiCheat::thread_CheckLoadedModules(LPVOID param)
{
	CAntiCheat* p = (CAntiCheat*)param;
	std::string s;

	while (1) {
		p->ThreadRequestResources();

		p->d3d->PrintMessage("Checking loaded modules...", 1000);
		p->EnumerateModules();

		p->ThreadReleaseResources();
		Sleep(8000);
	}

	return TRUE;
}

DWORD WINAPI CAntiCheat::thread_StayAlivePackets(LPVOID param)
{
	CAntiCheat* p = (CAntiCheat*)param;

	while (1) {
		if (p->connection.Connected()) {
			p->ThreadRequestResources();

			// build a packet
			t_client_antihack_handshake packet;
			memset((void*)&packet, 0, sizeof(packet));
			packet.h.size = sizeof(packet);
			packet.h.type = (short)CLIENT_ANTIHACK_HANDSHAKE;
			packet.something_special = 666;

			// send it
			p->connection.SendBuffer((void*)&packet, sizeof(packet));

			p->ThreadReleaseResources();
		}

		Sleep(5000);
	}

	return TRUE;
}

DWORD WINAPI CAntiCheat::thread_ReceiveCommands(LPVOID param)
{
	static bool goldOn = false;

	CAntiCheat* p = (CAntiCheat*)param;

	while (1) {
		p->ThreadRequestResources();

		if (GetAsyncKeyState('P')) {
			// fixme : check if in-game == true
			goldOn = !goldOn;

			if (!goldOn) {
				for (int k = 0; k < GameInfo::MAX_PLAYERS; k++) {
					p->d3d->RemoveCvar<int>("Gold (" + GameInfo::colorStrings[k] + ")");
				}
			} else {
				for (int k = 0; k < GameInfo::MAX_PLAYERS; k++) {
					p->players[k] = (CPlayer*)p->GetDeepPointer(0x6FACE5E0, 3, 0xC, 0x14 + k*0x140, 0x00);
					p->d3d->AddCvar("Gold (" + GameInfo::colorStrings[k] + ")", &p->players[k]->gold);
				}
				p->d3d->AddCvar<void*>("Address", static_cast<void*>(&p->players[1]->gold));
				p->d3d->AddCvar<int>("Value", &p->players[1]->gold);
			}

			Sleep(100);
		}
		
		if (GetAsyncKeyState(VK_F8)) {
			p->d3d->TogglePrintFlags(CD3DManager::PF_MESSAGES);
			p->d3d->PrintMessage("message display toggled");

			Sleep(100);
		}

		if (GetAsyncKeyState(VK_F9)) {
			p->d3d->TogglePrintFlags(CD3DManager::PF_CVARS);
			p->d3d->PrintMessage("cvar display toggled");

			Sleep(100);
		}

		if (GetAsyncKeyState(VK_F5)) {
			p->d3d->TakeScreenshot("C:\\warcraft.bmp");
			p->d3d->PrintMessage("Taking screenshot...");

			Sleep(100);
		}

		p->ThreadReleaseResources();
		Sleep(10);
	}

	return TRUE;
}

DWORD WINAPI CAntiCheat::thread_AntiDebug(LPVOID param)
{
	CAntiCheat* p = (CAntiCheat*)param;

	while (1) {
		p->ThreadRequestResources();

		p->AntiDebug();

		p->ThreadReleaseResources();
		Sleep(100);
	}

	return TRUE;
}

// -------------------------------------------------------------------------------
// COMMUNICATION:
// -------------------------------------------------------------------------------

int CAntiCheat::InitialiseWinsock(SOCKET s)
{
	return connection.Init(s);
}

int CAntiCheat::ShutDownWinsock(SOCKET s)
{
	return connection.Shutdown(s);
}

// -------------------------------------------------------------------------------
// PROTECTION:
// -------------------------------------------------------------------------------

int CAntiCheat::ShutDownWarcraft()
{
	// wait for all vThreads to stop working and then stop them going at it again

	FreeAllResources();
	ExitProcess(0); // Should be possible, is it? Will destructor be called?

	return 1;
}

int CAntiCheat::HideModule()
{
	// Remove module from PEB
	__asm {

	}

	return 1;
}

int CAntiCheat::AntiDebug()
{
	if (IsDebuggerPresent()) {
		ReportViolation(VIOLATION_DEBUGGER_PRESENT);
		//ShutDownWarcraft();
	}

	return 1;
}

// -------------------------------------------------------------------------------
// ANTI-CHEAT:
// -------------------------------------------------------------------------------

int CAntiCheat::ScanHotspot(const HOTSPOT& hotspot)
{
	// return true if invalid code is found

	for (unsigned int k = 0; k < hotspot.size; ++k)
		if (hotspot.address[k] != hotspot.real_data[k])
			return 1;

	return 0;
}

int CAntiCheat::CodeHasValidChecksum()
{
	int ret = 1;

	// war3.exe ------------------------------------------------------
	{
		MD5_CTX mdContext = MD5_Section(war3_base + war3_code, war3_size);

		for (int k = 0; k < 16; ++k) {
			if (mdContext.digest[k] != war3_checksum[k]) {
				LogHash(war3_base + war3_code, war3_size, mdContext.digest);
				LogText("war3.exe contains modified code!");
				ret = 0;
				break;
			}
		}
	}

	// game.dlll ------------------------------------------------------
	{
		MD5_CTX mdContext = MD5_Section(game_base + game_code, game_size);

		for (int k = 0; k < 16; ++k) {
			if (mdContext.digest[k] != game_checksum[k]) {
				LogHash(game_base + game_code, game_size, mdContext.digest);
				LogText("game.dll contains modified code!");
				ret = 0;
				break;
			}
		}
	}

	// storm.dll -----------------------------------------------------
	{
		MD5_CTX mdContext = MD5_Section(storm_base + storm_code, storm_size);

		for (int k = 0; k < 16; ++k) {
			if (mdContext.digest[k] != storm_checksum[k]) {
				LogHash(storm_base + storm_code, storm_size, mdContext.digest);
				LogText("storm.dll contains modified code!");
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

MD5_CTX CAntiCheat::MD5_Section(int base, int size)
{
	DWORD dwOld = 0;
	VirtualProtect((void*)base, size, PAGE_EXECUTE_READWRITE, &dwOld);

	MD5_CTX mdContext;

	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char*)base, size);
	MD5Final(&mdContext);

	VirtualProtect((void*)base, size, dwOld, &dwOld);

	return mdContext;
}

int CAntiCheat::LogHash(int base, int size, unsigned char* hash)
{
	LogBegin();
	log << "MD5 of section 0x" << std::hex << base << " +0x" << std::hex << size << " requested." << std::endl;
	log << "Hash: {";
	for (int k = 0; k < 16; ++k) {
		log << "0x" << std::hex << static_cast<unsigned int>(hash[k]) << ", ";
	}
	log << "}" << std::endl << std::endl;
	LogEnd();

	return 1;
}

int CAntiCheat::TakeScreenshot()
{
	if (!d3d) return 0;

	// generate filename
	// get screen co-ords

	d3d->TakeScreenshot("filename.bmp");

	return 1;
}

int CAntiCheat::ModuleIsSafe(std::string moduleName)
{
	for (int k = 0; k < WHITELIST_LENGTH; ++k) {
		if (moduleWhitelist[k] == moduleName) {
			return 1;
		}
	}

	return 0;
}

int CAntiCheat::ModuleIsBlack(std::string moduleName)
{
	for (int k = 0; k < BLACKLIST_LENGTH; ++k) {
		if (moduleBlacklist[k] == moduleName) {
			return 1;
		}
	}

	return 0;
}

int CAntiCheat::EnumerateModules()
{
	int ret = 1;
	std::string s;
		
	HANDLE hSnap;
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);

	MODULEENTRY32 me32;
	ZeroMemory((void*)&me32, sizeof(MODULEENTRY32));
	me32.dwSize = sizeof(MODULEENTRY32);

	Module32First(hSnap, &me32);
	static int count = 0;
	do {
		s = me32.szModule;

		if (!ModuleIsSafe(s)) {
			if (ModuleIsBlack(s)) {
				d3d->PrintMessage("Illegal module detected! Module name: " + s);
				ReportViolation(VIOLATION_BLACKLIST_MODULE);
			} else {
				d3d->PrintMessage("Suspicious module detected! Module name: " + s);
			}

			LogText("\"" + s + "\",");

			ret = 0;
		}
	} while (Module32Next(hSnap, &me32));

	CloseHandle(hSnap);

	return ret;
}

int CAntiCheat::BuildWhitelist(std::string filename)
{
	// dumps all loaded modules in a textfile
	// run on a clean game

	std::ofstream of(filename.c_str());

	HANDLE hSnap;
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);

	MODULEENTRY32 me32;
	ZeroMemory((void*)&me32, sizeof(MODULEENTRY32));
	me32.dwSize = sizeof(MODULEENTRY32);

	Module32First(hSnap, &me32);
	static int count = 0;
	do {
		of << "\"" << me32.szModule << "\"," << std::endl;
		++count;
	} while (Module32Next(hSnap, &me32));

	of << count;

	CloseHandle(hSnap);
	of.close();

	return 1;
}

DWORD CAntiCheat::GetCurrentThreadId()
{
	DWORD result;

	__asm mov eax, fs:[0x24] // Current thread id
	__asm mov result, eax

	return result;
}

int CAntiCheat::SuspendThreadById(DWORD threadId)
{
	if (threadId != GetCurrentThreadId()) {
		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, 0, threadId);
		if (!hThread) d3d->PrintMessage("Could not aquire thread handle");
		SuspendThread(hThread);
		CloseHandle(hThread);

		return 1;
	}

	return 0;
}

int CAntiCheat::ResumeThreadById(DWORD threadId)
{
	if (threadId != GetCurrentThreadId()) {
		HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, 0, threadId);
		ResumeThread(hThread);
		CloseHandle(hThread);

		return 1;
	}

	return 0;
}

int CAntiCheat::EnumerateThreads(int (CAntiCheat::*callback)(DWORD threadId))
{
	HANDLE hSnap;

	// returns a list of ALL processes' threads, must filter!
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (!hSnap) return 0;

	THREADENTRY32 te32;
	ZeroMemory((void*)&te32, sizeof(THREADENTRY32));
	te32.dwSize = sizeof(THREADENTRY32);

	if (!Thread32First(hSnap, &te32)) return 0;
	do {
		// DONT suspend all threads in the system O_O
		if (te32.th32OwnerProcessID == GetCurrentProcessId()) {
			(this->*callback)(te32.th32ThreadID);
		}
	} while (Thread32Next(hSnap, &te32));

	CloseHandle(hSnap);

	return 1;
}

int CAntiCheat::ReportViolation(int code)
{
	t_client_antihack_violation packet;
	packet.h.type = CLIENT_ANTIHACK_VIOLATION;
	packet.h.size = sizeof(packet);
	packet.violation = code;

	return connection.SendBuffer((void*)&packet, sizeof(packet));
}

// -------------------------------------------------------------------------------
// GENERAL:
// -------------------------------------------------------------------------------

int CAntiCheat::FreeAllResources()
{
	// Stop vThreads
	std::vector<HANDLE>::iterator i;
	for (i = vThreads.begin(); i != vThreads.end(); ++i) {
		TerminateThread(*i, 0);
		CloseHandle(*i);
	}

	return 1;
}

int CAntiCheat::LogBegin()
{
	log.open("anticheat.txt", std::ios::app);

	return 1;
}

int CAntiCheat::LogText(std::string s)
{
	LogBegin();
	log << s << std::endl;
	LogEnd();

	return 1;
}

int CAntiCheat::LogEnd()
{
	log.close();

	return 1;
}

int CAntiCheat::GetDeepPointer(int base, int argc, ...)
{
	int addr = *(int*)base;
	int* p = &argc;
	++p; // point to first arg (argc + 4 bytes)

	while (argc > 1) {	// only add last offset, dont dereference as well {
		addr += *p; // add offset
		addr = *(int*)addr; // dereference

		++p;
		--argc;
	}

	addr += *p;

	return addr;
}