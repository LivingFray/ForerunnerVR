#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include "common/utils/singleton.h"

class DeltaModule : public Singleton<DeltaModule>
{
public:
	bool Initialise();

protected:
	static inline const char* ModuleName = "halo2.dll";
	HMODULE GameModule;
};