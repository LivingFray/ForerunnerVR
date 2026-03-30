#include "interface.h"

#include "payload/delta/blam/math/integer_math.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"

#include "payload/delta/deltamodule.h"
#include "payload/delta/rendercomponent.h"
#include "payload/delta/uicomponent.h"

#include <DirectXMath.h>
#include <d3d11.h>

void interface_draw_screen::Patch()
{
	int index = player_window_index();
	if (index >= 0 && index < 2)
	{
		const auto& proj = global_window_parameters().projection;

		DirectX::XMMATRIX viewMat;
		viewMat.r[0] = DirectX::XMVectorSet(proj.world_to_view.rotation.matrix[0][0], proj.world_to_view.rotation.matrix[0][1], proj.world_to_view.rotation.matrix[0][2], 0.0f);
		viewMat.r[1] = DirectX::XMVectorSet(proj.world_to_view.rotation.matrix[1][0], proj.world_to_view.rotation.matrix[1][1], proj.world_to_view.rotation.matrix[1][2], 0.0f);
		viewMat.r[2] = DirectX::XMVectorSet(proj.world_to_view.rotation.matrix[2][0], proj.world_to_view.rotation.matrix[2][1], proj.world_to_view.rotation.matrix[2][2], 0.0f);
		viewMat.r[3] = DirectX::XMVectorSet(proj.world_to_view.translation.x, proj.world_to_view.translation.y, proj.world_to_view.translation.z, 1.0f);

		DirectX::XMMATRIX projMat;
		projMat.r[0] = DirectX::XMVectorSet(proj.projection_matrix.matrix[0][0], proj.projection_matrix.matrix[0][1], proj.projection_matrix.matrix[0][2], proj.projection_matrix.matrix[0][3]);
		projMat.r[1] = DirectX::XMVectorSet(proj.projection_matrix.matrix[1][0], proj.projection_matrix.matrix[1][1], proj.projection_matrix.matrix[1][2], proj.projection_matrix.matrix[1][3]);
		projMat.r[2] = DirectX::XMVectorSet(proj.projection_matrix.matrix[2][0], proj.projection_matrix.matrix[2][1], proj.projection_matrix.matrix[2][2], proj.projection_matrix.matrix[2][3]);
		projMat.r[3] = DirectX::XMVectorSet(proj.projection_matrix.matrix[3][0], proj.projection_matrix.matrix[3][1], proj.projection_matrix.matrix[3][2], proj.projection_matrix.matrix[3][3]);

		DeltaModule::Get().Render.SetEyeViewProj(index, viewMat * projMat);
	}

	// Only draw hud for first eye
	if (index != 0)
	{
		return;
	}

	DeltaModule::Get().UI.UpdateHUD();

	ID3D11RenderTargetView* ActiveRenderTarget = g_output_target();
	D3D11_VIEWPORT Viewport;
	UINT NumViewports = 1;

	g_output_target() = DeltaModule::Get().Render.GetUITargetView();

	// HUD messaging just assumes the render target is set, while all other UI explicity sets the target, so we need to manually set it to our target
	g_device_context()->OMSetRenderTargets(1, &(g_output_target()), g_output_depth_stencil_view());
	g_device_context()->RSGetViewports(&NumViewports, &Viewport);

	rectangle2d OriginalViewBounds = g_render_camera().viewport_bounds;
	rectangle2d OriginalWindowBounds = g_render_camera().window_bounds;

	DeltaModule::Get().bRenderingHUD = true;

	g_render_camera().viewport_bounds.x0 = 0;
	g_render_camera().viewport_bounds.x1 = DeltaModule::Get().Render.GetUIWidth();
	g_render_camera().viewport_bounds.y0 = 0;
	g_render_camera().viewport_bounds.y1 = DeltaModule::Get().Render.GetUIHeight();

	g_render_camera().window_bounds.x0 = 0;
	g_render_camera().window_bounds.x1 = DeltaModule::Get().Render.GetUIWidth();
	g_render_camera().window_bounds.y0 = 0;
	g_render_camera().window_bounds.y1 = DeltaModule::Get().Render.GetUIHeight();

	rectangle2d OriginalUIViewportBounds = global_window_parameters().camera.viewport_bounds;
	rectangle2d OriginalUIWindowBounds = global_window_parameters().camera.window_bounds;

	global_window_parameters().camera.viewport_bounds.x0 = 0;
	global_window_parameters().camera.viewport_bounds.x1 = DeltaModule::Get().Render.GetUIWidth();
	global_window_parameters().camera.viewport_bounds.y0 = 0;
	global_window_parameters().camera.viewport_bounds.y1 = DeltaModule::Get().Render.GetUIHeight();

	global_window_parameters().camera.window_bounds.x0 = 0;
	global_window_parameters().camera.window_bounds.x1 = DeltaModule::Get().Render.GetUIWidth();
	global_window_parameters().camera.window_bounds.y0 = 0;
	global_window_parameters().camera.window_bounds.y1 = DeltaModule::Get().Render.GetUIHeight();

	Original();

	g_render_camera().viewport_bounds = OriginalViewBounds;
	g_render_camera().window_bounds = OriginalWindowBounds;
	global_window_parameters().camera.viewport_bounds = OriginalUIViewportBounds;
	global_window_parameters().camera.window_bounds = OriginalUIWindowBounds;

	g_output_target() = ActiveRenderTarget;

	// Restore the render target so subsequent calls go to the right screen (e.g. screen flashes/fades)
	g_device_context()->OMSetRenderTargets(1, &(g_output_target()), g_output_depth_stencil_view());
	g_device_context()->RSSetViewports(NumViewports, &Viewport);

	DeltaModule::Get().bRenderingHUD = false;
}

void interface_draw_splitscreen_borders::Patch()
{
	// Do nothing, we don't want any borders for splitscreen
	return;
}