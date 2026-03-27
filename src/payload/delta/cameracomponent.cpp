#include "cameracomponent.h"
#include "deltamodule.h"
#include "payload/delta/blam/game/players.h"
#include "payload/delta/blam/main/main_render.h"
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"
#include "payload/delta/blam/simulation/simulation.h"

#include "common/vr/IVR.h"

void CameraComponent::UpdatePlayerCamera(simulation_update* update)
{
	player_action& action = update->player_actions[0];

	const VR::Matrix4x4 HMDMatrix = DeltaModule::Get().VR->GetHMDTransform();

	// Extract the forward vector from the HMD matrix
	const VR::Vector3 HMDFacing = {HMDMatrix.m[0][0], HMDMatrix.m[1][0], HMDMatrix.m[2][0]};

	// Normalize the facing vector (shouldn't matter in most cases)
	VR::Vector3 NormalizedFacing = HMDFacing;
	NormalizedFacing.SafeNormalize();

	// Calculate yaw (rotation around the vertical axis)
	const float Yaw = atan2f(NormalizedFacing.y, NormalizedFacing.x);

	// Calculate pitch (rotation around the horizontal axis)
	const float Pitch = atan2f(NormalizedFacing.z, sqrt(NormalizedFacing.x * NormalizedFacing.x + NormalizedFacing.y * NormalizedFacing.y));

	// Update the player's action rotation
	action.rotation.yaw = Yaw;
	action.rotation.pitch = Pitch;
}

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

	VR::Matrix4x4 EyeMatrix = VR::Matrix4x4::Scale(METRES_TO_WORLD) * DeltaModule::Get().VR->GetHMDTransform() * DeltaModule::Get().VR->GetEyeTransform(Eye);

	// Add eye translation
	CameraPos = CameraPos + VR::Vector3::FromVector4(EyeMatrix * VR::Vector4::Identity());

	const VR::Vector3 WorldForward(1.0f, 0.0f, 0.0f);
	const VR::Vector3 WorldUp(0.0f, 0.0f, 1.0f);

	VR::Vector3 CameraForward = VR::Vector3::FromVector4(EyeMatrix * VR::Vector4::FromVector(WorldForward));
	VR::Vector3 CameraUp = VR::Vector3::FromVector4(EyeMatrix * VR::Vector4::FromVector(WorldUp));

	CameraForward.SafeNormalize();
	CameraUp.SafeNormalize();

	render_window->rasterizer_camera.position = VR::Cast<real_point3d>(CameraPos);
	render_window->render_camera.position = VR::Cast<real_point3d>(CameraPos);

	render_window->rasterizer_camera.forward = VR::Cast<real_vector3d>(CameraForward);
	render_window->render_camera.forward = VR::Cast<real_vector3d>(CameraForward);

	render_window->rasterizer_camera.up = VR::Cast<real_vector3d>(CameraUp);
	render_window->render_camera.up = VR::Cast<real_vector3d>(CameraUp);
}
