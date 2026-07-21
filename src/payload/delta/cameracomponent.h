#pragma once
#include "common/utils/matrices.h"
#include "common/utils/vectors.h"
#include "common/vr/IVR.h"

// Handles positioning the game camera based on the HMD's transform
class CameraComponent
{
public:
	// Calculate the updated pitch/yaw of the player, including any yaw input from snap/smooth turning
	void UpdatePlayerCamera(float InYaw, float& OutYaw, float& OutPitch);

	// Update the camera at render time
	void UpdateRenderCamera(struct render_window* render_window, int view_index);

	void RecentreCamera();

	// Get the HMD transform in local space, accounting for recentres
	Matrix4 GetHMDTransform() const;

	// Get a controller transform in local space, accounting for recentres
	Matrix4 GetControllerTransform(EVR_Controller Controller) const;

	// Get the game's camera location, before being modified for VR
	Vector3 GetGameCameraLocation() const
	{
		return TrueCameraLocation;
	}

	// Get the game's camera forward vector, before being modified for VR
	Vector3 GetGameCameraForward() const
	{
		return TrueCameraForward;
	}

	// Get the game's camera up vector, before being modified for VR
	Vector3 GetGameCameraUp() const
	{
		return TrueCameraUp;
	}

protected:

	void SetCameraTransform(struct render_window* RenderWindow, const Vector3& Position, const Vector3& Forward, const Vector3& Up);

	void UpdateOffsetMatrix();

	Vector3 TrueCameraLocation;
	Vector3 TrueCameraForward;
	Vector3 TrueCameraUp;

	Vector3 OffsetLocation;
	float OffsetYaw = 0.0f;

	Matrix4 OffsetMatrix;
};