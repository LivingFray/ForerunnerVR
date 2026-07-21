#include "cameracomponent.h"
#include "payload/forerunner/forerunnermodule.h"
#include "payload/delta/blam/game/players.h"
#include "payload/delta/blam/main/main_render.h"
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"
#include "payload/delta/blam/cutscene/cinematics.h"

#include "common/utils/utils.h"
#include "common/utils/matrices.h"
#include "common/utils/vectors.h"
#include "common/vr/IVR.h"

void CameraComponent::UpdatePlayerCamera(float InYaw, float& OutYaw, float& OutPitch)
{
	// Apply any turn inputs first
	if (abs(InYaw) > FLT_EPSILON)
	{
		OffsetYaw += InYaw;
		UpdateOffsetMatrix();
	}
	
	const Matrix4 HMDMatrix = GetHMDTransform();

	// Extract the forward vector from the HMD matrix
	Vector3 HMDFacing = HMDMatrix.getLeftAxis();

	// Normalize the facing vector (shouldn't matter in most cases)
	HMDFacing.normalize();

	// Calculate yaw (rotation around the vertical axis)
	OutYaw = atan2f(HMDFacing.y, HMDFacing.x);

	// Calculate pitch (rotation around the horizontal axis)
	OutPitch = atan2f(HMDFacing.z, sqrt(HMDFacing.x * HMDFacing.x + HMDFacing.y * HMDFacing.y));
}

// TODO: Config/ignore + use real player height
constexpr float PlayerHeight = 0.257572f;

void CameraComponent::UpdateRenderCamera(struct render_window* render_window, int view_index)
{
	const EVR_Eye Eye = view_index == 0 ? EVR_Eye::Left : EVR_Eye::Right;

	float FOV = ForerunnerModule::Get().VR->GetVerticalFieldOfView(Eye);

	render_window->rasterizer_camera.vertical_field_of_view = FOV;
	render_window->render_camera.vertical_field_of_view = FOV;

	const bool IsInCinematic = cinematic_in_progress();

	// Ensure last cinematic state is tracked regardless of code path
	SCOPE_EXIT(
		WasInCinematic = IsInCinematic;
	);

	if (IsInCinematic)
	{
		// First frame in cinematic, reset the camera to face the correct direction
		if (!WasInCinematic)
		{
			RecentreCamera();
		}

		Vector3 CameraPos = SameCast<Vector3>(render_window->rasterizer_camera.position);

		// Get yaw from cutscene camera
		const Vector3 CutsceneForward = SameCast<Vector3>(render_window->rasterizer_camera.forward);
		const float CameraYaw = atan2f(CutsceneForward.y, CutsceneForward.x);

		const Matrix4 CutsceneMatrix = Matrix4().rotateZ(Rad2Deg(CameraYaw));

		const Matrix4 EyeMatrix = CutsceneMatrix * Matrix4().scale(METRES_TO_WORLD) * GetHMDTransform() * ForerunnerModule::Get().VR->GetEyeTransform(Eye);

		// Add eye translation
		CameraPos = CameraPos + Vector3FromVector4(EyeMatrix * Vector4FromPoint(Vector3()));

		const Vector3 WorldForward(1.0f, 0.0f, 0.0f);
		const Vector3 WorldUp(0.0f, 0.0f, 1.0f);

		// Transform eye location
		Vector3 CameraForward = Vector3FromVector4(EyeMatrix * Vector4FromVector(WorldForward));
		Vector3 CameraUp = Vector3FromVector4(EyeMatrix * Vector4FromVector(WorldUp));

		CameraForward.normalize();
		CameraUp.normalize();

		// Apply new vectors
		SetCameraTransform(render_window, CameraPos, CameraForward, CameraUp);

		return;
	}
	
	// Normal VR camera logic for gameplay
	// TODO: Vehicles + cutscenes will ruin this
	// TODO: Crouch will also break this
	real_vector3d InterpolatedPosition;
	interpolation_get_object_position(players_get_object_id(render_window->player_index), &InterpolatedPosition);

	Vector3 CameraPos = SameCast<Vector3>(InterpolatedPosition);
	CameraPos.z += PlayerHeight;

	if (view_index == 0)
	{
		// TODO: Remove this, used for hacky placement of viewmodel
		TrueCameraLocation = SameCast<Vector3>(render_window->rasterizer_camera.position);
		TrueCameraForward = SameCast<Vector3>(render_window->rasterizer_camera.forward);
		TrueCameraUp = SameCast<Vector3>(render_window->rasterizer_camera.up);
	}

	// TODO: HMD Transform
	// Needs to handle roomscale movement + apply remainder to camera
	// Also also needs to account for default eye height vs ground

	// TODO: Vehicles

	Matrix4 EyeMatrix = Matrix4().scale(METRES_TO_WORLD) * GetHMDTransform() * ForerunnerModule::Get().VR->GetEyeTransform(Eye);

	// Add eye translation
	CameraPos = CameraPos + Vector3FromVector4(EyeMatrix * Vector4FromPoint(Vector3()));

	const Vector3 WorldForward(1.0f, 0.0f, 0.0f);
	const Vector3 WorldUp(0.0f, 0.0f, 1.0f);

	Vector3 CameraForward = Vector3FromVector4(EyeMatrix * Vector4FromVector(WorldForward));
	Vector3 CameraUp = Vector3FromVector4(EyeMatrix * Vector4FromVector(WorldUp));

	CameraForward.normalize();
	CameraUp.normalize();

	SetCameraTransform(render_window, CameraPos, CameraForward, CameraUp);
}

void CameraComponent::RecentreCamera()
{
	Matrix4 CurrentTransform = ForerunnerModule::Get().VR->GetHMDTransform();

	const Vector3 CurrentFacing = CurrentTransform.getLeftAxis(); // Math library uses different coordinate system, "left" is forwards

	OffsetYaw = Rad2Deg(atan2f(CurrentFacing.y, CurrentFacing.x));
	OffsetLocation = Vector3FromVector4(CurrentTransform * Vector4FromPoint(Vector3()));

	UpdateOffsetMatrix();
}

Matrix4 CameraComponent::GetHMDTransform() const
{
	return OffsetMatrix * ForerunnerModule::Get().VR->GetHMDTransform();
}

Matrix4 CameraComponent::GetControllerTransform(EVR_Controller Controller) const
{
	return OffsetMatrix * ForerunnerModule::Get().VR->GetControllerTransform(Controller);
}

void CameraComponent::SetCameraTransform(render_window* RenderWindow, const Vector3& Position, const Vector3& Forward, const Vector3& Up)
{
	RenderWindow->rasterizer_camera.position = SameCast<real_point3d>(Position);
	RenderWindow->render_camera.position = SameCast<real_point3d>(Position);

	RenderWindow->rasterizer_camera.forward = SameCast<real_vector3d>(Forward);
	RenderWindow->render_camera.forward = SameCast<real_vector3d>(Forward);

	RenderWindow->rasterizer_camera.up = SameCast<real_vector3d>(Up);
	RenderWindow->render_camera.up = SameCast<real_vector3d>(Up);
}

void CameraComponent::UpdateOffsetMatrix()
{
	OffsetMatrix = Matrix4().translate(-OffsetLocation).rotateZ(-OffsetYaw);
}
