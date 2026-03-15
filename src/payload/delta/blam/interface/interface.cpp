#include "interface.h"

#include "payload/delta/blam/math/integer_math.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"

#include "payload/delta/deltamodule.h"

void interface_draw_screen::Patch()
{
	// Only draw hud for first eye
	if (player_window_index() != 0)
	{
		return;
	}

	ID3D11RenderTargetView* ActiveRenderTarget = g_output_target();

	g_output_target() = DeltaModule::Get().Test.UITargetView;

	rectangle2d OriginalViewBounds = g_render_camera().viewport_bounds;
	rectangle2d OriginalWindowBounds = g_render_camera().window_bounds;

	DeltaModule::Get().bRenderingHUD = true;

	g_render_camera().viewport_bounds.x0 = 0;
	g_render_camera().viewport_bounds.x1 = DeltaModule::Get().Test.UI_WIDTH;
	g_render_camera().viewport_bounds.y0 = 0;
	g_render_camera().viewport_bounds.y1 = DeltaModule::Get().Test.UI_HEIGHT;

	g_render_camera().window_bounds.x0 = 0;
	g_render_camera().window_bounds.x1 = DeltaModule::Get().Test.UI_WIDTH;
	g_render_camera().window_bounds.y0 = 0;
	g_render_camera().window_bounds.y1 = DeltaModule::Get().Test.UI_HEIGHT;

	rectangle2d OriginalUIBounds = global_window_parameters().camera.viewport_bounds;

	global_window_parameters().camera.viewport_bounds.x0 = 0;
	global_window_parameters().camera.viewport_bounds.x1 = DeltaModule::Get().Test.UI_WIDTH;
	global_window_parameters().camera.viewport_bounds.y0 = 0;
	global_window_parameters().camera.viewport_bounds.y1 = DeltaModule::Get().Test.UI_HEIGHT;

	Original();

	g_render_camera().viewport_bounds = OriginalViewBounds;
	g_render_camera().window_bounds = OriginalWindowBounds;
	global_window_parameters().camera.viewport_bounds = OriginalUIBounds;

	g_output_target() = ActiveRenderTarget;

	DeltaModule::Get().bRenderingHUD = false;
}

void interface_draw_splitscreen_borders::Patch()
{
	// Do nothing, we don't want any borders for splitscreen
	return;
}