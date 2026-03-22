#include "emulatedvr.h"
#include <d3d11.h>
#include <dxgi1_2.h>

static constexpr int VR_WIDTH = 800;
static constexpr int VR_HEIGHT = 600;

bool EmulatedVR::EarlyInit()
{
	FORERUNNER_LOG(EmuVR, "Performing Early initialisation");

	return true;
}

bool EmulatedVR::Init()
{
	FORERUNNER_LOG(EmuVR, "Initialising");


	return true;
}

void EmulatedVR::Shutdown()
{
	CloseWindow(VRWindow);

	VRWindow = nullptr;
	UnregisterClassW(L"ForerunnerVRClass", GetModuleHandleW(0));

	// Release DX11 objects:
	if (BackBufferView) BackBufferView->Release();

	Device = nullptr;
	DeviceContext = nullptr;
	SwapChain = nullptr;
	BackBufferView = nullptr;
}

void EmulatedVR::Update(float DeltaTime)
{
	// Dirty hack for Win32 API requiring window updates happen on the same thread, but Init + Update potentially being called from different threads
	if (!VRWindow)
	{
		if (!CreateVRWindow())
		{
			return;
		}

		if (!CreateDirectXObjects())
		{
			return;
		}

		ShowWindow(VRWindow, SW_SHOW);
		UpdateWindow(VRWindow);
	}

	MSG Message{};
	while (PeekMessageW(&Message, VRWindow, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
}

void EmulatedVR::SubmitEye(EVR_Eye Eye, ID3D11RenderTargetView* RenderTargetView, const VR_Bounds& ViewBounds)
{
	{
		ID3D11Resource* srcResource = nullptr;
		RenderTargetView->GetResource(&srcResource);

		if (!srcResource)
		{
			FORERUNNER_ERROR(EmuVR, "Can't get resource from RenderTargetView {:#08x})", reinterpret_cast<int64_t>(RenderTargetView));
			return;
		}

		ID3D11Texture2D* srcTex = nullptr;
		srcResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&srcTex);

		if (!srcTex)
		{
			FORERUNNER_ERROR(EmuVR, "Can't get texture from RenderTargetView {:#08x})", reinterpret_cast<int64_t>(RenderTargetView));
			return;
		}

		ID3D11Resource* dstResource = nullptr;
		BackBufferView->GetResource(&dstResource);

		if (!dstResource)
		{
			FORERUNNER_ERROR(EmuVR, "Can't get resource from Backbuffer RenderTargetView {:#08x})", reinterpret_cast<int64_t>(BackBufferView));
			return;
		}

		ID3D11Texture2D* dstTex = nullptr;
		dstResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&dstTex);

		if (!dstTex)
		{
			FORERUNNER_ERROR(EmuVR, "Can't get texture from Backbuffer RenderTargetView {:#08x})", reinterpret_cast<int64_t>(BackBufferView));
			return;
		}

		D3D11_TEXTURE2D_DESC srcDesc{};
		D3D11_TEXTURE2D_DESC dstDesc{};
		srcTex->GetDesc(&srcDesc);
		dstTex->GetDesc(&dstDesc);

		D3D11_BOX srcBox{};
		srcBox.left = static_cast<UINT>(ViewBounds.x * srcDesc.Width);
		srcBox.top = static_cast<UINT>(ViewBounds.y * srcDesc.Height);
		srcBox.front = 0;
		srcBox.right = static_cast<UINT>(ViewBounds.x * srcDesc.Width + ViewBounds.w * srcDesc.Width);
		srcBox.bottom = static_cast<UINT>(ViewBounds.y * srcDesc.Height + ViewBounds.h * srcDesc.Height);
		srcBox.back = 1;

		UINT StartX = (Eye == EVR_Eye::Left) ? 0 : VR_WIDTH;

		DeviceContext->CopySubresourceRegion(dstResource, 0, StartX, 0, 0, srcTex, 0, &srcBox);

		srcTex->Release();
		dstTex->Release();
		srcResource->Release();
		dstResource->Release();
	}

	// Got both eyes, present
	if (Eye == EVR_Eye::Right)
	{
		SwapChain->Present(0, 0);

		const float clear_color_with_alpha[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		DeviceContext->ClearRenderTargetView(BackBufferView, clear_color_with_alpha);
	}
}

void EmulatedVR::SetDevice(ID3D11Device* InDevice)
{
	this->Device = InDevice;
}

void EmulatedVR::SetDeviceContext(ID3D11DeviceContext* InContext)
{
	this->DeviceContext = InContext;
}

float EmulatedVR::GetDesiredWidth()
{
	return VR_WIDTH;
}

float EmulatedVR::GetDesiredHeight()
{
	return VR_HEIGHT;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		default:
			result = DefWindowProcW(hwnd, msg, wparam, lparam);
	}
	return result;
}

bool EmulatedVR::CreateVRWindow()
{
	FORERUNNER_LOG(EmuVR, "Creating VR display window");

	// Create window class
	WNDCLASSEXW WinClass = {};
	WinClass.cbSize = sizeof(WNDCLASSEXW);
	WinClass.style = CS_HREDRAW | CS_VREDRAW;
	WinClass.lpfnWndProc = &WndProc;
	WinClass.hInstance = GetModuleHandleW(0);
	WinClass.hCursor = LoadCursorW(0, IDC_ARROW);
	WinClass.lpszClassName = L"ForerunnerVRClass";

	if (!RegisterClassExW(&WinClass))
	{
		FORERUNNER_ERROR(EmuVR, "RegisterClassEx failed");
		return false;
	}

	RECT WindowRect = {0, 0, VR_WIDTH * 2, VR_HEIGHT}; // Double width to account for both eyes
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

	VRWindow = CreateWindowExW(0, WinClass.lpszClassName, L"Forerunner Render Test", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, 0, 0, GetModuleHandleW(0), 0);

	return VRWindow != nullptr;
}

bool EmulatedVR::CreateDirectXObjects()
{
	FORERUNNER_LOG(EmuVR, "Creating DirectX Objects");

	if (!VRWindow)
	{
		FORERUNNER_ERROR(EmuVR, "VR Window not found");
		return false;
	}

	if (!Device)
	{
		FORERUNNER_ERROR(EmuVR, "IDirect3D11Device not found");
		return false;
	}

	if (!DeviceContext)
	{
		FORERUNNER_ERROR(EmuVR, "IDirect3D11DeviceContext not found");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc{};
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.Width = 0;
	SwapChainDesc.Height = 0;
	SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_0};

	HRESULT hr;
	IDXGIDevice2* pDXGIDevice;
	hr = Device->QueryInterface(__uuidof(IDXGIDevice2), (void**)&pDXGIDevice);

	IDXGIAdapter* pDXGIAdapter;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);

	IDXGIFactory2* pIDXGIFactory;
	pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pIDXGIFactory);

	pIDXGIFactory->CreateSwapChainForHwnd(Device, VRWindow, &SwapChainDesc, NULL, NULL, &SwapChain);

	if (!SwapChain)
	{
		FORERUNNER_ERROR(EmuVR, "Can't create swap chain for window: {}", GetLastError());
		return false;
	}

	FORERUNNER_LOG(EmuVR, "Created swapchain");

	ID3D11Texture2D* BackBuffer;
	SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &BackBufferView);
	BackBuffer->Release();
	FORERUNNER_LOG(EmuVR, "Grabbed backbuffer");

	return true;
}
