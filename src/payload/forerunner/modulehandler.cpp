#include "modulehandler.h"

#include "payload/delta/deltamodule.h"
#include <iostream>

void ModuleHandler::Initialise()
{
	// TODO: Menu/MCC module
	if (DeltaModule::Get().Initialise())
	{
		std::cout << "Delta Module loaded" << std::endl;
	}
}