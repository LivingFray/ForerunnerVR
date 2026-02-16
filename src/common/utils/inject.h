#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

namespace Inject
{
	bool FindProcess(const char* ProcessName, DWORD& OutProcess);
	bool InjectDLL(const char* DLLName, DWORD Process);
	HMODULE FindModule(const char* ModuleName);
}
