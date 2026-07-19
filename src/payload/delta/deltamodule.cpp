#include "deltamodule.h"
#include "payload/forerunner/forerunnermodule.h"
// Utils
#include "common/utils/inject.h"
#include "common/vr/IVR.h"
// Blam decomp code
#include "payload/delta/blam/halo_main.h"
#include "payload/delta/blam/animations/animation_graph_definitions.h"
#include "payload/delta/blam/game/game.h"
#include "payload/delta/blam/game/players.h"
#include "payload/delta/blam/interface/first_person_weapons.h"
#include "payload/delta/blam/interface/interface.h"
#include "payload/delta/blam/interface/hud.h"
#include "payload/delta/blam/main/main_render.h"
#include "payload/delta/blam/main/main_time.h"
#include "payload/delta/blam/math/matrix_math.h"
#include "payload/delta/blam/objects/objects.h"
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

	Input.RegisterInputs();

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
}

void DeltaModule::PostUpdate(int ticks, float* seconds)
{
	// TODO: This assumes update + render are on the same thread, which is only true for original graphics
	ForerunnerModule::Get().VR->Update(g_delta_time());
}

void DeltaModule::Present()
{
	// TODO: Move this
	if (!bHasInit)
	{
		Render.Init();
		bHasInit = true;

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
	game_update,
	first_person_update_bones,
	h2a2_change_render_mode,
	c_animation2_rig_definition__node_matrices_from_orientations
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
	bSuccess |= g_players.Find();
	bSuccess |= g_objects.Find();
	bSuccess |= g_objects_pool.Find();
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
	bSuccess |= g_hud_scaling.Find();
	bSuccess |= hud_set_size_and_safe_area.Find();
	bSuccess |= interpolation_get_object_position.Find();
	bSuccess |= target_graphics_mode.Find();
	bSuccess |= matrix4x3_from_orientation.Find();
	bSuccess |= matrix4x3_multiply.Find();
	bSuccess |= g_cache_file_globals__tags_header.Find();
	bSuccess |= g_tags_physical_memory.Find();

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
