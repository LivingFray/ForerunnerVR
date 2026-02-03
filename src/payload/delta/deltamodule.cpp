#include "deltamodule.h"
#include "common/utils/inject.h"

bool DeltaModule::Initialise()
{
	// Find the correct dll (user may not have installed this game)
	GameModule = Inject::FindModule(ModuleName);
	if (GameModule == NULL)
	{
		return false;
	}


	return true;
}