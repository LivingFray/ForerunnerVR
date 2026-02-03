#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <Psapi.h>
#include <iostream>
#include "payload/forerunner/modulehandler.h"

static long long BaseAddress;

DWORD WINAPI MainLoop(HMODULE hModule)
{	
	HMODULE H2Handle = GetModuleHandleA("halo2.dll");

	if (!H2Handle)
	{
		std::cout << "Can't find halo2 module!" << std::endl;
		return 1;
	}


	MODULEINFO ModuleInfo;

	BOOL Res = GetModuleInformation(GetCurrentProcess(), H2Handle, &ModuleInfo, sizeof(ModuleInfo));

	if (!Res)
	{
		std::cout << "Can't get module information!" << std::endl;
		return 1;
	}

	BaseAddress = reinterpret_cast<long long>(ModuleInfo.lpBaseOfDll);

	// Debug console
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	ModuleHandler::Get().Initialise();

	while (true)
	{
		Sleep(20);
		if (GetAsyncKeyState(VK_NUMPAD0))
			break;
	}


	// Cleanup
	if (fp)
	{
		fclose(fp);
	}
	FreeConsole();

	return 0;
}



BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			// Create thread with main loop
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainLoop, NULL, 0, NULL);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}