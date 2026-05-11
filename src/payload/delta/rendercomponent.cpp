#include "rendercomponent.h"
#include "deltamodule.h"
#include "payload/forerunner/forerunnermodule.h"
// Utils
#include "common/vr/IVR.h"
// Blam decomp code
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/rasterizer/rasterizer_main.h"
// DirectX
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3dcompiler.lib")

static constexpr UINT UI_WIDTH = 800;
static constexpr UINT UI_HEIGHT = 600;

struct UIConstantBufferData
{
	DirectX::XMMATRIX ViewProjection;
};

struct UIVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 Tex;
};

void RenderComponent::SetUITransform(const DirectX::XMMATRIX& Transform)
{
	UIWorldTransform = Transform;
	bUIWorldTransformSet = true;
}

void RenderComponent::SetEyeViewProj(int EyeIndex, const DirectX::XMMATRIX& ViewProj)
{
	if (EyeIndex >= 0 && EyeIndex < 2)
	{
		EyeViewProj[EyeIndex] = ViewProj;
	}
}

void RenderComponent::Init()
{
	FORERUNNER_LOG(Delta_Render, "Initialising Render Component");

	ResizeBuffers();

	CreateRenderTarget(GameWidth, GameHeight, &RenderTargetView, &RenderTargetTexture);
	CreateRenderTarget(UI_WIDTH, UI_HEIGHT, &UITargetView, &UITexture, &UITextureSRV);

	InitUIShader();

	g_output_target() = RenderTargetView;
}

void RenderComponent::Shutdown()
{
	CleanupUIShader();

	if (RenderTargetView) RenderTargetView->Release();
	RenderTargetView = nullptr;
	if (RenderTargetTexture) RenderTargetTexture->Release();
	RenderTargetTexture = nullptr;
	if (UITargetView) UITargetView->Release();
	UITargetView = nullptr;
	if (UITexture) UITexture->Release();
	UITexture = nullptr;
	if (UITextureSRV) UITextureSRV->Release();
	UITextureSRV = nullptr;
}

void RenderComponent::Draw()
{
	ID3D11Device* Device = g_device();
	ID3D11DeviceContext* Context = g_device_context();

	// Clear game window
	const float clear_color_with_alpha[4] = {0.2f, 0.2f, 0.4f, 1.0f};
	Context->ClearRenderTargetView(MirrorTargetView, clear_color_with_alpha);

	// Draw UI Shader onto RenderTargetView for VR
	if (UIVertexShader && UIPixelShader && UITextureSRV)
	{
		Context->OMSetRenderTargets(1, &RenderTargetView, nullptr);

		Context->IASetInputLayout(UIInputLayout);
		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		UINT stride = sizeof(UIVertex);
		UINT offset = 0;
		Context->IASetVertexBuffers(0, 1, &UIVertexBuffer, &stride, &offset);

		Context->VSSetShader(UIVertexShader, nullptr, 0);
		Context->VSSetConstantBuffers(0, 1, &UIConstantBuffer);

		Context->PSSetShader(UIPixelShader, nullptr, 0);
		Context->PSSetShaderResources(0, 1, &UITextureSRV);
		Context->PSSetSamplers(0, 1, &UISamplerState);

		float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
		Context->OMSetBlendState(UIBlendState, blendFactor, 0xffffffff);

		D3D11_VIEWPORT vp{};
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;

		// Top half (Left Eye)
		{
			UIConstantBufferData cbData;
			cbData.ViewProjection = DirectX::XMMatrixTranspose(UIWorldTransform * EyeViewProj[0]);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (SUCCEEDED(Context->Map(UIConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			{
				memcpy(mappedResource.pData, &cbData, sizeof(UIConstantBufferData));
				Context->Unmap(UIConstantBuffer, 0);
			}

			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			vp.Width = (float)GameWidth;
			vp.Height = (float)GameHeight / 2.0f;
			Context->RSSetViewports(1, &vp);
			Context->Draw(4, 0);
		}

		// Bottom half (Right Eye)
		{
			UIConstantBufferData cbData;
			cbData.ViewProjection = DirectX::XMMatrixTranspose(UIWorldTransform * EyeViewProj[1]);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (SUCCEEDED(Context->Map(UIConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
			{
				memcpy(mappedResource.pData, &cbData, sizeof(UIConstantBufferData));
				Context->Unmap(UIConstantBuffer, 0);
			}

			vp.TopLeftX = 0;
			vp.TopLeftY = (float)GameHeight / 2.0f;
			vp.Width = (float)GameWidth;
			vp.Height = (float)GameHeight / 2.0f;
			Context->RSSetViewports(1, &vp);
			Context->Draw(4, 0);
		}

		// Reset state
		ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
		Context->PSSetShaderResources(0, 1, nullSRV);
	}

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
		.y = 0.5f,
		.w = 1.0f,
		.h = 0.5f
	};
	const VR_Bounds RightEyeBounds
	{
		.x = 0.0f,
		.y = 0.0f,
		.w = 1.0f,
		.h = 0.5f
	};

	ForerunnerModule::Get().VR->SubmitEye(EVR_Eye::Left, RenderTargetTexture, LeftEyeBounds);
	ForerunnerModule::Get().VR->SubmitEye(EVR_Eye::Right, RenderTargetTexture, RightEyeBounds);

	ForerunnerModule::Get().MarkFrameAsVR();

	// Clear UI target now we are done with it
	const float ui_clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	Context->ClearRenderTargetView(UITargetView, ui_clear_color);
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

	FORERUNNER_LOG(Delta_Render, "Delta Swapchain: {:#08x}", reinterpret_cast<int64_t>(g_swap_chain()));

	FORERUNNER_LOG(Delta_Render, "g_swap_chain: {}x{} ({})", desc.BufferDesc.Width, desc.BufferDesc.Height, desc.BufferCount);

	FORERUNNER_LOG(Delta_Render, "Deinitializing rasterizer");
	rasterizer_deinitialize();

	// 2x Width to account for both eyes
	// TODO: Scope will require a third view
	// TODO: Need to patch splitscreen calculations to make sure it does a horizontal split (currently does vertical but looks like it can sometimes be overridden)
	GameWidth = ForerunnerModule::Get().VR->GetDesiredWidth();
	GameHeight = ForerunnerModule::Get().VR->GetDesiredHeight() * 2;

	FORERUNNER_LOG(Delta_Render, "Setting rasterizer display size to {}x{}", GameWidth, GameHeight);
	rasterizer_set_display_size(GameWidth, GameHeight);

	FORERUNNER_LOG(Delta_Render, "Initializing rasterizer");
	rasterizer_initialize();

	MirrorTargetView = g_output_target();
	g_output_target() = RenderTargetView;
}

void RenderComponent::CreateRenderTarget(uint32_t Width, uint32_t Height, ID3D11RenderTargetView** OutRenderTarget, ID3D11Texture2D** OutTexture, ID3D11ShaderResourceView** OutSRV)
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

	if (OutSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = bufferDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		Device->CreateShaderResourceView(*OutTexture, &srvDesc, OutSRV);
	}
}

bool RenderComponent::InitUIShader()
{
	ID3D11Device* Device = g_device();
	if (!Device) return false;

	const char* shaderSource = R"(
		cbuffer UIConstantBuffer : register(b0)
		{
			matrix ViewProjection;
		};

		struct VS_INPUT
		{
			float3 Pos : POSITION;
			float2 Tex : TEXCOORD0;
		};

		struct PS_INPUT
		{
			float4 Pos : SV_POSITION;
			float2 Tex : TEXCOORD0;
		};

		PS_INPUT VSMain(VS_INPUT input)
		{
			PS_INPUT output;
			output.Pos = mul(float4(input.Pos, 1.0f), ViewProjection);
			output.Tex = input.Tex;
			return output;
		}

		Texture2D shaderTexture : register(t0);
		SamplerState Sampler : register(s0);

		float4 PSMain(PS_INPUT input) : SV_TARGET
		{
			float4 color = shaderTexture.Sample(Sampler, input.Tex);
			return color;
		}
	)";

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3DCompile(shaderSource, strlen(shaderSource), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			FORERUNNER_ERROR(Delta_Render, "Failed to compile VS: {}", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}

	hr = D3DCompile(shaderSource, strlen(shaderSource), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			FORERUNNER_ERROR(Delta_Render, "Failed to compile PS: {}", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &UIVertexShader);
	Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &UIPixelShader);

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	Device->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &UIInputLayout);

	vsBlob->Release();
	psBlob->Release();

	// Create Vertex Buffer
	UIVertex vertices[] = {
		{ {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
		{ {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f} },
		{ { 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
		{ { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f} }
	};
	D3D11_BUFFER_DESC vbDesc{};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vbData{};
	vbData.pSysMem = vertices;
	Device->CreateBuffer(&vbDesc, &vbData, &UIVertexBuffer);

	// Create Constant Buffer
	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(UIConstantBufferData);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Device->CreateBuffer(&cbDesc, nullptr, &UIConstantBuffer);

	// Create Blend State
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Device->CreateBlendState(&blendDesc, &UIBlendState);

	// Create Sampler State
	D3D11_SAMPLER_DESC sampDesc{};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Device->CreateSamplerState(&sampDesc, &UISamplerState);

	return true;
}

void RenderComponent::CleanupUIShader()
{
	if (UIVertexShader) UIVertexShader->Release(); UIVertexShader = nullptr;
	if (UIPixelShader) UIPixelShader->Release(); UIPixelShader = nullptr;
	if (UIInputLayout) UIInputLayout->Release(); UIInputLayout = nullptr;
	if (UIVertexBuffer) UIVertexBuffer->Release(); UIVertexBuffer = nullptr;
	if (UIConstantBuffer) UIConstantBuffer->Release(); UIConstantBuffer = nullptr;
	if (UIBlendState) UIBlendState->Release(); UIBlendState = nullptr;
	if (UISamplerState) UISamplerState->Release(); UISamplerState = nullptr;
}

