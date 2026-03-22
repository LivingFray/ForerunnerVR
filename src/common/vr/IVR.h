#pragma once
#include <stdint.h>

enum class EVR_Eye : uint8_t
{
	Left,
	Right
};

struct VR_Bounds
{
	float x;
	float y;
	float w;
	float h;
};

// Generic interface for interacting with a VR runtime, should be implemented for each vendor (OpenVR, OpenXR, etc) supported
class IVR
{
public:

	// Early initialisation, called near startup
	virtual bool EarlyInit() = 0;
	// Late initialisation, called once important systems such as directx have been initialised
	virtual bool Init() = 0;
	// Called when VR should end and be cleaned up gracefully
	virtual void Shutdown() = 0;
	// Called every frame
	virtual void Update(float DeltaTime) = 0;
	// Called when the game has a new frame to submit for an eye, ViewBounds defines which portion of the render target holds the current eye
	virtual void SubmitEye(EVR_Eye Eye, struct ID3D11RenderTargetView* RenderTargetView, const VR_Bounds& ViewBounds = VR_Bounds{ .x = 0.0f, .y = 0.0f, .w = 1.0f, .h = 1.0f}) = 0;

	virtual void SetDevice(struct ID3D11Device* Device) = 0;
	virtual void SetDeviceContext(struct ID3D11DeviceContext* Context) = 0;

	// Get the desired width of the render target for one eye
	virtual float GetDesiredWidth() = 0;
	// Get the desired height of the render target for one eye
	virtual float GetDesiredHeight() = 0;
};