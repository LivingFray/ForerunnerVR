#pragma once

#include "common/utils/singleton.h"

class ModuleHandler : public Singleton<ModuleHandler>
{
public:
	void Initialise();
};