#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

namespace Inject
{
	bool FindProcess(const char* ProcessName, DWORD& OutProcess);
}