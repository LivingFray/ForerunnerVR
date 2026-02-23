#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <Psapi.h>
#include <iostream>
#include "payload/forerunner/modulehandler.h"
#include "common/utils/log.h"


void ConsoleLogConsumer(LogLevel Level, const LogCategory& Category, const std::string& Message)
{
	std::cout << "[" << LogLevelToString(Level) << "][" << Category.Name << "] " << Message << std::endl;
}

DWORD WINAPI MainLoop(HMODULE hModule)
{	
	// Debug console
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	LogManager::AddConsumer(ConsoleLogConsumer);

	// Let MCC get any startup logic done (e.g. loading the game dlls)
	Sleep(5000);

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