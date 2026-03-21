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
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	switch (Level)
	{
		case LogLevel::Log:
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			break;
		case LogLevel::Warn:
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
			break;
		case LogLevel::Error:
			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			break;
		default:
			break;
	}

	std::cout << "[" << LogLevelToString(Level) << "][" << Category.Name << "] " << Message << std::endl;

	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

DWORD WINAPI MainLoop(HMODULE hModule)
{	
	// Debug console
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	LogManager::AddConsumer(ConsoleLogConsumer);

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