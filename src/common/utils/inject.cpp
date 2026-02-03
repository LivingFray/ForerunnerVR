#include "inject.h"
#include "utils.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

#include <psapi.h>
#include <string>

static std::string GetProcessName(DWORD Process)
{
	HANDLE Proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Process);

	if (!Proc)
	{
		return "";
	}

	char szProcessName[MAX_PATH] = "";


	if (GetProcessImageFileNameA(Proc, szProcessName, MAX_PATH))
	{
		CloseHandle(Proc);
		return szProcessName;
	}

	CloseHandle(Proc);
	return "";
}


struct FindProcessParam
{
	const char* ProcessName = nullptr;
	DWORD Process = 0;
	HWND Window = 0;
	bool bFound = false;
};

static BOOL CALLBACK EnumWindowsProc(HWND HWnd, LPARAM lParam)
{
	FindProcessParam* Param = reinterpret_cast<FindProcessParam*>(lParam);

	DWORD ThreadId = 0;

	GetWindowThreadProcessId(HWnd, &ThreadId);

	std::string CurrentProcessName = GetProcessName(ThreadId);

	if (CurrentProcessName.find(Param->ProcessName) != std::string::npos)
	{
		Param->bFound = true;
		Param->Process = ThreadId;
		Param->Window = HWnd;
		return FALSE;
	}

	return TRUE;
}


bool Inject::FindProcess(const char* ProcessName, DWORD& OutProcess)
{
	FindProcessParam Param;
	Param.ProcessName = ProcessName;
	EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&Param));

	if (!Param.bFound)
	{
		return false;
	}

	OutProcess = Param.Process;

	return true;
}

bool Inject::InjectDLL(const char* DLLName, DWORD Process)
{
	// Open target process with necessary permissions to inject the DLL
	HANDLE Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Process);
	if (!Proc)
	{
		return false;
	}

	// Ensure the process handle gets closed when we are done, regardless of when we exit
	SCOPE_EXIT(CloseHandle(Proc));

	// Get the address of kernel32 -> LoadLibraryA so we can use it later to load our payload DLL
	HMODULE KernelModule = GetModuleHandleA("kernel32.dll");
	if (!KernelModule)
	{
		return false;
	}

	LPVOID BaseAddress = GetProcAddress(KernelModule, "LoadLibraryA");
	if (!BaseAddress)
	{
		return false;
	}

	// Allocate enough space inside the target process to store the name of the DLL we want to inject
	LPVOID Space = VirtualAllocEx(Proc, NULL, strlen(DLLName), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!Space)
	{
		return false;
	}

	// Write the DLLName into the target process at our newly allocated space
	BOOL HasWritten = WriteProcessMemory(Proc, Space, DLLName, strlen(DLLName), NULL);
	if (HasWritten == 0)
	{
		return false;
	}

	// Create a thread on the target process using LoadLibraryA as the entry function and our target DLL as the parameter, tricking the process into loading our payload
	HANDLE NewThread = CreateRemoteThread(Proc, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(BaseAddress), Space, NULL, NULL);
	if (!NewThread)
	{
		return false;
	}

	return true;
}

HMODULE Inject::FindModule(const char* ModuleName)
{
	return GetModuleHandleA(ModuleName);
}
