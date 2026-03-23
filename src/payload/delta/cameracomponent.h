#pragma once

// Handles positioning the game camera based on the HMD's transform
class CameraComponent
{
public:
	// Update the camera at render time
	void UpdateRenderCamera(struct render_window* render_window, int view_index);
};