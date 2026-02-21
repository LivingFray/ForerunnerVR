#include "deltamodule.h"
#include "common/utils/inject.h"
#include "patches/render/update_player_view_frustum.h"

bool DeltaModule::Initialise()
{
	// Find the correct dll (user may not have installed this game)
	GameModule = Inject::FindModule(ModuleName);
	if (GameModule == NULL)
	{
		FORERUNNER_WARN(Delta, "Can't find module, game is likely not installed");
		return false;
	}

	if (!CreatePatches())
	{
		FORERUNNER_ERROR(Delta, "Failed to create all patches");
		return false;
	}

	if (!ApplyPatches())
	{
		FORERUNNER_ERROR(Delta, "Failed to apply all patches");
		return false;
	}

	FORERUNNER_LOG(Delta, "Successfully patched");

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
