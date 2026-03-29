#include "cameracomponent.h"
#include "deltamodule.h"
#include "payload/delta/blam/game/players.h"
#include "payload/delta/blam/main/main_render.h"
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"
#include "payload/delta/blam/simulation/simulation.h"

#include "common/utils/utils.h"
#include "common/utils/matrices.h"
#include "common/utils/vectors.h"
#include "common/vr/IVR.h"

void CameraComponent::UpdatePlayerCamera(simulation_update* update)
{
	player_action& action = update->player_actions[0];

	const Matrix4 HMDMatrix = GetCameraTransform();

	// Extract the forward vector from the HMD matrix
	Vector3 HMDFacing = HMDMatrix.getLeftAxis();

	// Normalize the facing vector (shouldn't matter in most cases)
	HMDFacing.normalize();

	// Calculate yaw (rotation around the vertical axis)
	const float Yaw = atan2f(HMDFacing.y, HMDFacing.x);

	// Calculate pitch (rotation around the horizontal axis)
	const float Pitch = atan2f(HMDFacing.z, sqrt(HMDFacing.x * HMDFacing.x + HMDFacing.y * HMDFacing.y));

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

	Vector3 CameraPos = SameCast<Vector3>(render_window->rasterizer_camera.position);

	// TODO: HMD Transform
	// Needs to handle roomscale movement + apply remainder to camera
	// Also also needs to account for default eye height vs ground

	// TODO: Cutscenes
	// TODO: Vehicles

	Matrix4 EyeMatrix = Matrix4().scale(METRES_TO_WORLD) * GetCameraTransform() * DeltaModule::Get().VR->GetEyeTransform(Eye);

	// Add eye translation
	CameraPos = CameraPos + Vector3FromVector4(EyeMatrix * Vector4FromPoint(Vector3()));

	const Vector3 WorldForward(1.0f, 0.0f, 0.0f);
	const Vector3 WorldUp(0.0f, 0.0f, 1.0f);

	Vector3 CameraForward = Vector3FromVector4(EyeMatrix * Vector4FromVector(WorldForward));
	Vector3 CameraUp = Vector3FromVector4(EyeMatrix * Vector4FromVector(WorldUp));

	CameraForward.normalize();
	CameraUp.normalize();

	render_window->rasterizer_camera.position = SameCast<real_point3d>(CameraPos);
	render_window->render_camera.position = SameCast<real_point3d>(CameraPos);

	render_window->rasterizer_camera.forward = SameCast<real_vector3d>(CameraForward);
	render_window->render_camera.forward = SameCast<real_vector3d>(CameraForward);

	render_window->rasterizer_camera.up = SameCast<real_vector3d>(CameraUp);
	render_window->render_camera.up = SameCast<real_vector3d>(CameraUp);
}

void CameraComponent::RecentreCamera()
{
	Matrix4 CurrentTransform = DeltaModule::Get().VR->GetHMDTransform();

	OffsetYaw = 0.0f; // TODO: Extract yaw
	OffsetLocation = Vector3FromVector4(CurrentTransform * Vector4FromPoint(Vector3()));

	OffsetMatrix = Matrix4().translate(-OffsetLocation).rotateZ(-OffsetYaw);
}

Matrix4 CameraComponent::GetCameraTransform()
{
	return OffsetMatrix * DeltaModule::Get().VR->GetHMDTransform();
}
