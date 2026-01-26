#include "inject.h"

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
