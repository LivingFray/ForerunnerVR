#pragma once

// Handles positioning the game camera based on the HMD's transform
class CameraComponent
{
public:
	// Update the look vector of the player
	void UpdatePlayerCamera(struct simulation_update* update);

	// Update the camera at render time
	void UpdateRenderCamera(struct render_window* render_window, int view_index);
};