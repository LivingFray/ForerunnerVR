#pragma once
#include "common/utils/log.h"
#include <cstdint>
#include <DirectXMath.h>

FORERUNNER_CREATE_LOG_CATEGORY(Delta_Render);

// Handles the rendering logic for the mod
// e.g. swapping render targets/setting window bounds
class RenderComponent
{
public:

	void Init();
	void Shutdown();

	void Draw();

	struct ID3D11RenderTargetView* GetUITargetView() const;
	uint32_t GetUIWidth() const;
	uint32_t GetUIHeight() const;

	void SetUITransform(const DirectX::XMMATRIX& Transform);
	void SetEyeViewProj(int EyeIndex, const DirectX::XMMATRIX& ViewProj);
	bool IsUITransformSet() const { return bUIWorldTransformSet; }

protected:
	// Resize the game's internal buffers to match the size needed to render both VR eyes
	void ResizeBuffers();
	void CreateRenderTarget(uint32_t Width, uint32_t Height, struct ID3D11RenderTargetView** OutRenderTarget, struct ID3D11Texture2D** OutTexture, struct ID3D11ShaderResourceView** OutSRV = nullptr);

	struct ID3D11RenderTargetView* RenderTargetView = nullptr;
	struct ID3D11Texture2D* RenderTargetTexture = nullptr;
	struct ID3D11RenderTargetView* UITargetView = nullptr;
	struct ID3D11Texture2D* UITexture = nullptr;
	struct ID3D11ShaderResourceView* UITextureSRV = nullptr;
	struct ID3D11RenderTargetView* MirrorTargetView = nullptr;

	// UI Shader Objects
	struct ID3D11VertexShader* UIVertexShader = nullptr;
	struct ID3D11PixelShader* UIPixelShader = nullptr;
	struct ID3D11InputLayout* UIInputLayout = nullptr;
	struct ID3D11Buffer* UIVertexBuffer = nullptr;
	struct ID3D11Buffer* UIConstantBuffer = nullptr;
	struct ID3D11BlendState* UIBlendState = nullptr;
	struct ID3D11SamplerState* UISamplerState = nullptr;

	DirectX::XMMATRIX UIWorldTransform = DirectX::XMMatrixIdentity();
	bool bUIWorldTransformSet = false;
	DirectX::XMMATRIX EyeViewProj[2];

	bool InitUIShader();
	void CleanupUIShader();

	int GameWidth = 0;
	int GameHeight = 0;
};