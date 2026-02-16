#include "modulehandler.h"

#include <iostream>
#include "payload/forerunner/patch.h"
#include "payload/delta/deltamodule.h"

void ModuleHandler::Initialise()
{
	// Do any shared initialisation logic first
	if (!Patch::Initialise())
	{
		return;
	}
	
	// TODO: Menu/MCC module
	if (DeltaModule::Get().Initialise())
	{
		std::cout << "Delta Module loaded" << std::endl;
	}
	else
	{
		std::cout << "Delta Module failed to load" << std::endl;
	}
}