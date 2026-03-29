#include "deltamodule.h"
// Utils
#include "common/utils/inject.h"
#include "common/vr/IVR.h"
#include "common/vr/emulatedvr.h"
#include "common/vr/openvr.h"
// Blam decomp code
#include "payload/delta/blam/game/game.h"
#include "payload/delta/blam/game/players.h"
#include "payload/delta/blam/interface/interface.h"
#include "payload/delta/blam/main/main_render.h"
#include "payload/delta/blam/main/main_time.h"
#include "payload/delta/blam/rasterizer/rasterizer_main.h"
#include "payload/delta/blam/rasterizer/rasterizer_globals.h"
#include "payload/delta/blam/render/render.h"
// DirectX
#include <d3d11.h>
#include <dxgi1_2.h>

bool DeltaModule::Initialise()
{
	// Find the correct dll (user may not have installed this game)
	GameModule = Inject::FindModule(ModuleName);
	if (GameModule == NULL)
	{
		FORERUNNER_WARN(Delta, "Can't find module, game is likely not installed");
		return false;
	}

	// TODO: This should be done at a higher level (i.e. in a forerunner module)
	VR = new OpenVR();
	//VR = new EmulatedVR();
	VR->EarlyInit();

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

	// TODO: These might need moving too

	//Render.Init();

	return true;
}

void DeltaModule::Deinitialise()
{
	// TODO: Clean up any specific resources/reset any state
	Render.Shutdown();

	DisablePatches();
	DestroyPatches();
}

static bool bHasInit = false;

void DeltaModule::PreUpdate(int ticks, float* seconds)
{
	if (bHasInit)
	{
		VR->Update(g_delta_time());
	}
}

void DeltaModule::PostUpdate(int ticks, float* seconds)
{
}

void DeltaModule::Present()
{
	// TODO: Move this
	if (!bHasInit)
	{
		VR->SetDevice(g_device());
		VR->SetDeviceContext(g_device_context());
		VR->Init();

		Render.Init();
		bHasInit = true;

		VR->Update(g_delta_time());

		Camera.RecentreCamera();
	}

	Render.Draw();
}

using AllPatches = Patch::PatchList<
	compute_window_bounds,
	players_get_window_count,
	rasterizer_present,
	interface_draw_screen,
	interface_draw_splitscreen_borders,
	players_update_before_game,
	game_update
>;

bool DeltaModule::CreatePatches()
{
	return AllPatches::CreateAll();
}

bool DeltaModule::ApplyPatches()
{
	return AllPatches::EnableAll();
}

bool DeltaModule::FindGlobals()
{
	bool bSuccess = true;

	bSuccess |= g_players_globals.Find();
	bSuccess |= g_device.Find();
	bSuccess |= g_device_context.Find();
	bSuccess |= g_output_target.Find();
	bSuccess |= g_output_depth_stencil_view.Find();
	bSuccess |= g_rasterizer_globals.Find();
	bSuccess |= g_swap_chain.Find();
	bSuccess |= rasterizer_refresh.Find();
	bSuccess |= rasterizer_initialize.Find();
	bSuccess |= rasterizer_set_display_size.Find();
	bSuccess |= rasterizer_deinitialize.Find();
	bSuccess |= player_window_index.Find();
	bSuccess |= g_render_camera.Find();
	bSuccess |= global_window_parameters.Find();
	bSuccess |= g_delta_time.Find();

	return bSuccess;
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

bool DeltaModule::DisablePatches()
{
	return AllPatches::DisableAll();
}

bool DeltaModule::DestroyPatches()
{
	return AllPatches::DestroyAll();
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		default:
			result = DefWindowProcW(hwnd, msg, wparam, lparam);
	}
	return result;
}
