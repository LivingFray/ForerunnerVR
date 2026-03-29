#pragma once
#include "IVR.h"
#include "common/utils/matrices.h"
#include "common/utils/vectors.h"
#include "common/utils/log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

FORERUNNER_CREATE_LOG_CATEGORY(EmuVR);

// Fake VR interface which creates a window displaying the "VR" view. 
// Useful for rapid testing without having to constantly be putting on/taking off an actual headset
class EmulatedVR : public IVR
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
	virtual float GetVerticalFieldOfView(EVR_Eye Eye) const override;
	virtual Matrix4 GetHMDTransform() const override;
	virtual Matrix4 GetEyeTransform(EVR_Eye Eye) const override;

protected:
	bool CreateVRWindow();
	bool CreateDirectXObjects();

	HWND VRWindow = NULL;
	struct ID3D11Device* Device = nullptr;
	struct ID3D11DeviceContext* DeviceContext = nullptr;
	struct IDXGISwapChain1* SwapChain = nullptr;
	struct ID3D11RenderTargetView* BackBufferView = nullptr;

	float CameraYaw = 0.0f;
	float CameraPitch = 0.0f;
	Vector3 CameraOffset = Vector3(0.0f, 0.0f, 0.05f);
};