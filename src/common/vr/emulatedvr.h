#pragma once
#include "IVR.h"
#include "common/utils/matrices.h"
#include "common/utils/vectors.h"
#include "common/utils/log.h"
#include <mutex>

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
	bool Init(struct ID3D11Device* Device, struct ID3D11DeviceContext* Context) override;
	void Shutdown() override;
	void Update(float DeltaTime) override;
	void SubmitEye(EVR_Eye Eye, struct ID3D11Texture2D* Texture, const VR_Bounds& ViewBounds) override;
	void EndFrame() override;
	int32_t GetDesiredWidth() const override;
	int32_t GetDesiredHeight() const override;
	virtual float GetVerticalFieldOfView(EVR_Eye Eye) const override;
	virtual Matrix4 GetHMDTransform() const override;
	virtual Matrix4 GetEyeTransform(EVR_Eye Eye) const override;
	Matrix4 GetControllerTransform(EVR_Controller Controller) const override;
	InputBindingID RegisterBoolInput(const std::string& Set, const std::string& Action) override;
	InputBindingID RegisterVector2Input(const std::string& Set, const std::string& Action) override;
	bool GetBoolInput(InputBindingID ID) const override;
	bool GetBoolInput(InputBindingID ID, bool& bHasChanged) const override;
	Vector2 GetVector2Input(InputBindingID ID) const override;
	InputBindingID RegisterActionSet(const std::string& Set) override;
	void ActivateActionSet(InputBindingID ID) override;
	void DeactivateActionSet(InputBindingID ID) override;
	void DrawOverlay(struct ID3D11DeviceContext* Context, struct ID3D11Texture2D* SourceTexture) override;
	void ShowOverlay() override;
	void HideOverlay() override;

protected:
	bool CreateVRWindow();
	bool CreateDirectXObjects();

	void UpdateKeyInputs();

	HWND VRWindow = NULL;
	struct ID3D11Device* Device = nullptr;
	struct ID3D11DeviceContext* DeviceContext = nullptr;
	struct IDXGISwapChain1* SwapChain = nullptr;
	struct ID3D11RenderTargetView* BackBufferView = nullptr;

	float CameraYaw = 0.0f;
	float CameraPitch = 0.0f;
	Vector3 CameraOffset = Vector3(0.0f, 0.0f, 0.05f);
	mutable std::mutex PoseMutex;
};