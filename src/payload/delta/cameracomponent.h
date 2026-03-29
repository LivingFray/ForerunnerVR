#pragma once
#include "common/utils/matrices.h"
#include "common/utils/vectors.h"

// Handles positioning the game camera based on the HMD's transform
class CameraComponent
{
public:
	// Update the look vector of the player
	void UpdatePlayerCamera(struct simulation_update* update);

	// Update the camera at render time
	void UpdateRenderCamera(struct render_window* render_window, int view_index);

	void RecentreCamera();

	// Get the HMD transform in local space, accounting for recentres
	Matrix4 GetCameraTransform();

protected:

	Vector3 OffsetLocation;
	float OffsetYaw = 0.0f;

	Matrix4 OffsetMatrix;
};