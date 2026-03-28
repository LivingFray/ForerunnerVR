#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <iostream>
#include "payload/forerunner/modulehandler.h"
#include "common/utils/log.h"
#include <filesystem>
#include <fstream>

FORERUNNER_CREATE_LOG_CATEGORY(DLLMain);

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

static std::ofstream LogFile;

void FileLogConsumer(LogLevel Level, const LogCategory& Category, const std::string& Message)
{
	if (!LogFile.is_open())
	{
		return;
	}

	LogFile << "[" << LogLevelToString(Level) << "][" << Category.Name << "] " << Message << std::endl;
}

DWORD WINAPI MainLoop(HMODULE hModule)
{
	// Debug console
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	LogManager::AddConsumer(ConsoleLogConsumer);

	wchar_t DLLPath[MAX_PATH];
	GetModuleFileNameW(hModule, DLLPath, MAX_PATH);

	std::filesystem::path LogPath{DLLPath};
	LogPath = LogPath.parent_path() / L"forerunner.log";

	LogFile.open(LogPath.c_str(), std::ios::out);

	if (LogFile.is_open())
	{
		LogManager::AddConsumer(FileLogConsumer);
	}

	ModuleHandler::Get().Initialise(DLLPath);

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

	if (LogFile.is_open())
	{
		LogFile.close();
	}

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
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainLoop, hModule, 0, NULL);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}