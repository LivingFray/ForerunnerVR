#include "cameracomponent.h"
#include "deltamodule.h"
#include "payload/delta/blam/main/main_render.h"
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"
#include "common/vr/IVR.h"

void CameraComponent::UpdateRenderCamera(struct render_window* render_window, int view_index)
{
	const EVR_Eye Eye = view_index == 0 ? EVR_Eye::Left : EVR_Eye::Right;

	float FOV = DeltaModule::Get().VR->GetVerticalFieldOfView(Eye);

	render_window->rasterizer_camera.vertical_field_of_view = FOV;
	render_window->render_camera.vertical_field_of_view = FOV;

	VR::Vector3 CameraPos = VR::Cast<VR::Vector3>(render_window->rasterizer_camera.position);

	// TODO: HMD Transform
	// Needs to handle roomscale movement + apply remainder to camera
	// Also needs to account for resetting orientation/centre
	// Also also needs to account for default eye height vs ground

	// TODO: Cutscenes
	// TODO: Vehicles
	// TODO: Convert between metres and halo units

	// Get current camera transform from forward + up vectors
	VR::Vector3 CameraForward = VR::Cast<VR::Vector3>(render_window->rasterizer_camera.forward);
	VR::Vector3 CameraUp = VR::Cast<VR::Vector3>(render_window->rasterizer_camera.up);
	VR::Vector3 CameraRight = VR::Vector3::Cross(CameraForward, CameraUp);

	VR::Matrix4x4 CameraMatrix = VR::Matrix4x4::FromBasisVectors(CameraForward, CameraRight, CameraUp);

	// Add eye translation
	CameraPos = CameraPos + (DeltaModule::Get().VR->GetEyeTransform(Eye) * CameraMatrix).GetTranslation() * METRES_TO_WORLD;

	render_window->rasterizer_camera.position = VR::Cast<real_point3d>(CameraPos);
	render_window->render_camera.position = VR::Cast<real_point3d>(CameraPos);
}
