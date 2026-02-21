#pragma once

#include "common/utils/singleton.h"
#include "common/utils/log.h"

FORERUNNER_CREATE_LOG_CATEGORY(Forerunner);

class ModuleHandler : public Singleton<ModuleHandler>
{
public:
	void Initialise();
};