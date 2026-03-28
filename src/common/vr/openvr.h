#pragma once
#include "IVR.h"
#include "common/vr/matrices.h"
#include "common/utils/log.h"
#include <openvr.h>

FORERUNNER_CREATE_LOG_CATEGORY(OpenVR);

class OpenVR : public IVR
{
public:
	// Inherited via IVR
	bool EarlyInit() override;
	bool Init() override;
	void Shutdown() override;
	void Update(float DeltaTime) override;
	void SubmitEye(EVR_Eye Eye, ID3D11RenderTargetView* RenderTargetView, const VR_Bounds& ViewBounds) override;
	void SetDevice(ID3D11Device* Device) override;
	void SetDeviceContext(ID3D11DeviceContext* Context) override;
	float GetDesiredWidth() const override;
	float GetDesiredHeight() const override;
	float GetVerticalFieldOfView(EVR_Eye Eye) const override;
	VR::Matrix4x4 GetHMDTransform() const override;
	VR::Matrix4x4 GetEyeTransform(EVR_Eye Eye) const override;

protected:

	// Subsystems
	vr::IVRSystem* VRSystem;
	vr::IVRCompositor* VRCompositor;
	vr::IVRInput* VRInput;
	vr::IVROverlay* VROverlay;

	// Viewport info
	uint32_t RecommendedWidth = 0;
	uint32_t RecommendedHeight = 0;
	vr::VRTextureBounds_t TextureBounds[2] = {};
	float AspectRatio = 0.0f;
	float VerticalFieldOfView = 0.0f;
};