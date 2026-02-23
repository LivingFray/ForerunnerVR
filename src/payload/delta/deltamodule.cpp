#include "deltamodule.h"
#include "common/utils/inject.h"
#include "patches/render/update_player_view_frustum.h"
#include "patches/render/draw_splitscreen_borders.h"
#include "patches/render/get_player_window_count.h"

bool DeltaModule::Initialise()
{
	// Find the correct dll (user may not have installed this game)
	GameModule = Inject::FindModule(ModuleName);
	if (GameModule == NULL)
	{
		FORERUNNER_WARN(Delta, "Can't find module, game is likely not installed");
		return false;
	}

	if (!FindGlobals())
	{
		FORERUNNER_ERROR(Delta, "Failed to find all necessary globals");
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

	if (!PatchCode())
	{
		FORERUNNER_ERROR(Delta, "Failed to apply all inline patches");
		return false;
	}

	FORERUNNER_LOG(Delta, "Successfully patched");

	return true;
}

bool DeltaModule::CreatePatches()
{
	bool bSuccess = true;
	bSuccess |= update_player_view_frustum::Create();
	bSuccess |= draw_splitscreen_borders::Create();
	bSuccess |= get_player_window_count::Create();

	return bSuccess;
}

bool DeltaModule::ApplyPatches()
{
	bool bSuccess = true;
	bSuccess |= update_player_view_frustum::Enable();
	bSuccess |= draw_splitscreen_borders::Enable();
	bSuccess |= get_player_window_count::Enable();

	return bSuccess;
}

bool DeltaModule::FindGlobals()
{
	bool bSuccess = true;

	bSuccess |= g_players_globals.Find();

	return true;
}

bool DeltaModule::PatchCode()
{
	bool bSuccess = true;
	Patch::SuspendThreads();
	FORERUNNER_LOG(Delta, "Suspending threads");

	bSuccess |= PatchSplitscreen();

	Patch::ResumeThreads();
	FORERUNNER_LOG(Delta, "Resuming threads");
	return bSuccess;
}


bool DeltaModule::PatchSplitscreen()
{
	// Patch 2 function calls in update_player_views to use player 0 as the target player
	WriteBytes(update_player_views__valid_user_id, {0x31, 0xc9}); // XOR ECX, ECX: sets player_index to 0
	WriteBytes(update_player_views__get_camera_result, {0x31, 0xc9}); // XOR ECX, ECX: sets player_index to 0

	// Remove the slight shrinking applied to the viewport in splitscreen (without the borders between views this is no longer needed)
	WriteBytes(calculate_viewport__left, {0x0});
	WriteBytes(calculate_viewport__right, {0x0});
	WriteBytes(calculate_viewport__bottom, {0x0});
	WriteBytes(calculate_viewport__top, {0x0});

	return true;
}