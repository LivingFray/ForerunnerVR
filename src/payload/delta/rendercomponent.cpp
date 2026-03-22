#include "rendercomponent.h"
#include "deltamodule.h"
// Utils
#include "common/vr/IVR.h"
// Blam decomp code
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/rasterizer/rasterizer_main.h"
// DirectX
#include <d3d11.h>

static constexpr UINT UI_WIDTH = 800;
static constexpr UINT UI_HEIGHT = 600;

void RenderComponent::Init()
{
	FORERUNNER_LOG(Delta_Render, "Initialising Render Component");

	ResizeBuffers();

	CreateRenderTarget(GameWidth, GameHeight, &RenderTargetView, &RenderTargetTexture);
	CreateRenderTarget(UI_WIDTH, UI_HEIGHT, &UITargetView, &UITexture);

	g_output_target() = RenderTargetView;
}

void RenderComponent::Shutdown()
{
	if (RenderTargetView) RenderTargetView->Release();
	RenderTargetView = nullptr;
	if (RenderTargetTexture) RenderTargetTexture->Release();
	RenderTargetTexture = nullptr;
	if (UITargetView) UITargetView->Release();
	UITargetView = nullptr;
	if (UITexture) UITexture->Release();
	UITexture = nullptr;
}

void RenderComponent::Draw()
{
	ID3D11Device* Device = g_device();
	ID3D11DeviceContext* Context = g_device_context();

	// Clear game window
	const float clear_color_with_alpha[4] = {0.2f, 0.2f, 0.4f, 1.0f};
	Context->ClearRenderTargetView(MirrorTargetView, clear_color_with_alpha);

	// Copy to Game backbuffer for mirror view
	{
		ID3D11Resource* srcResource = nullptr;
		RenderTargetView->GetResource(&srcResource);

		ID3D11Texture2D* srcTex = nullptr;
		srcResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&srcTex);

		ID3D11Resource* dstResource = nullptr;
		MirrorTargetView->GetResource(&dstResource);

		ID3D11Texture2D* dstTex = nullptr;
		dstResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&dstTex);

		D3D11_TEXTURE2D_DESC srcDesc{};
		D3D11_TEXTURE2D_DESC dstDesc{};
		srcTex->GetDesc(&srcDesc);
		dstTex->GetDesc(&dstDesc);

		D3D11_BOX srcBox{};
		srcBox.left = 0;
		srcBox.top = 0;
		srcBox.front = 0;
		srcBox.right = std::min(srcDesc.Width, dstDesc.Width);
		srcBox.bottom = std::min(srcDesc.Height, dstDesc.Height);
		srcBox.back = 1;

		Context->CopySubresourceRegion(dstResource, 0, 0, 0, 0, srcTex, 0, &srcBox);

		UINT UIStart = srcBox.right;

		// TODO: Re-impl UI via a shader (also replace above copy subresource call with this shader)
		srcBox.left = 0;
		srcBox.top = 0;
		srcBox.front = 0;
		srcBox.right = std::min(UI_WIDTH, dstDesc.Width);
		srcBox.bottom = std::min(UI_HEIGHT, dstDesc.Height);
		srcBox.back = 1;


		Context->CopySubresourceRegion(dstResource, 0, UIStart, 0, 0, UITexture, 0, &srcBox);

		srcTex->Release();
		dstTex->Release();
		srcResource->Release();
		dstResource->Release();
	}

	// TODO: Convert to horizontal (+ account for scope at some point)
	const VR_Bounds LeftEyeBounds
	{
		.x = 0.0f,
		.y = 0.0f,
		.w = 1.0f,
		.h = 0.5f
	};
	const VR_Bounds RightEyeBounds
	{
		.x = 0.0f,
		.y = 0.5f,
		.w = 1.0f,
		.h = 0.5f
	};

	DeltaModule::Get().VR->SubmitEye(EVR_Eye::Left, RenderTargetView, LeftEyeBounds);
	DeltaModule::Get().VR->SubmitEye(EVR_Eye::Right, RenderTargetView, RightEyeBounds);

	// Clear UI target now we are done with it
	Context->ClearRenderTargetView(UITargetView, clear_color_with_alpha);
}

ID3D11RenderTargetView* RenderComponent::GetUITargetView() const
{
	return UITargetView;
}

uint32_t RenderComponent::GetUIWidth() const
{
	return UI_WIDTH;
}

uint32_t RenderComponent::GetUIHeight() const
{
	return UI_HEIGHT;
}

void RenderComponent::ResizeBuffers()
{
	DXGI_SWAP_CHAIN_DESC desc;
	g_swap_chain()->GetDesc(&desc);
	UINT bufferCount = desc.BufferCount;

	FORERUNNER_LOG(Delta_Render, "g_swap_chain: {}x{} ({})", desc.BufferDesc.Width, desc.BufferDesc.Height, desc.BufferCount);

	FORERUNNER_LOG(Delta_Render, "Deinitializing rasterizer");
	rasterizer_deinitialize();

	// 2x Width to account for both eyes
	// TODO: Scope will require a third view
	// TODO: Need to patch splitscreen calculations to make sure it does a horizontal split (currently does vertical but looks like it can sometimes be overridden)
	GameWidth = static_cast<int>(DeltaModule::Get().VR->GetDesiredWidth());
	GameHeight = static_cast<int>(DeltaModule::Get().VR->GetDesiredHeight() * 2.0f);

	FORERUNNER_LOG(Delta_Render, "Setting rasterizer display size to {}x{}", GameWidth, GameHeight);
	rasterizer_set_display_size(GameWidth, GameHeight);

	FORERUNNER_LOG(Delta_Render, "Initializing rasterizer");
	rasterizer_initialize();

	MirrorTargetView = g_output_target();
	g_output_target() = RenderTargetView;
}

void RenderComponent::CreateRenderTarget(uint32_t Width, uint32_t Height, ID3D11RenderTargetView** OutRenderTarget, ID3D11Texture2D** OutTexture)
{
	FORERUNNER_LOG(Delta_Render, "Creating render target with size: {}x{}", Width, Height);

	ID3D11Device* Device = g_device();

	D3D11_TEXTURE2D_DESC bufferDesc{};
	bufferDesc.ArraySize = 1;
	bufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.Width = Width;
	bufferDesc.Height = Height;
	bufferDesc.MipLevels = 1;
	bufferDesc.MiscFlags = 0;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	Device->CreateTexture2D(&bufferDesc, 0, OutTexture);

	//Creating a view of the texture to be used when binding it as a render target
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	Device->CreateRenderTargetView(*OutTexture, 0, OutRenderTarget);
}
