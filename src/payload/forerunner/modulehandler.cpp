#include "modulehandler.h"

#include <iostream>
#include "payload/forerunner/patch.h"
#include "payload/delta/deltamodule.h"
#include "common/utils/log.h"

void ModuleHandler::Initialise()
{
	// Do any shared initialisation logic first
	if (!Patch::Initialise())
	{
		return;
	}

	FORERUNNER_LOG(Forerunner, "Loading modules...");
	
	// TODO: Menu/MCC module
	if (DeltaModule::Get().Initialise())
	{
		FORERUNNER_LOG(Forerunner, "Delta Module loaded");
	}
	else
	{
		FORERUNNER_WARN(Forerunner, "Delta Module failed to load");
	}

	FORERUNNER_LOG(Forerunner, "All modules loaded");
}