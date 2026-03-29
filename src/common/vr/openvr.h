#pragma once
#include "IVR.h"
#include "common/utils/matrices.h"
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
	void SubmitEye(EVR_Eye Eye, struct ID3D11Texture2D* Texture, const VR_Bounds& ViewBounds) override;
	void SetDevice(ID3D11Device* Device) override;
	void SetDeviceContext(ID3D11DeviceContext* Context) override;
	int32_t GetDesiredWidth() const override;
	int32_t GetDesiredHeight() const override;
	float GetVerticalFieldOfView(EVR_Eye Eye) const override;
	Matrix4 GetHMDTransform() const override;
	Matrix4 GetEyeTransform(EVR_Eye Eye) const override;

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

	// DirectX
	struct ID3D11Device* Device = nullptr;
	struct ID3D11DeviceContext* Context = nullptr;

	// Poses
	vr::TrackedDevicePose_t GamePoses[vr::k_unMaxTrackedDeviceCount];
	vr::TrackedDevicePose_t RenderPoses[vr::k_unMaxTrackedDeviceCount];
};