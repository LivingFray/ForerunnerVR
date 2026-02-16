#include "deltamodule.h"
#include "common/utils/inject.h"

#include "patches/render/update_player_view_frustum.h"

bool DeltaModule::Initialise()
{
	// Find the correct dll (user may not have installed this game)
	GameModule = Inject::FindModule(ModuleName);
	if (GameModule == NULL)
	{
		return false;
	}

	if (!CreatePatches())
	{
		return false;
	}

	if (!ApplyPatches())
	{
		return false;
	}

	return true;
}

bool DeltaModule::CreatePatches()
{
	bool bSuccess = true;
	bSuccess |= update_player_view_frustum::Create();

	return bSuccess;
}

bool DeltaModule::ApplyPatches()
{
	bool bSuccess = true;
	bSuccess |= update_player_view_frustum::Enable();

	return bSuccess;
}
