#include "deltamodule.h"
#include "common/utils/inject.h"
#include "patches/render/update_player_view_frustum.h"
#include "patches/render/draw_splitscreen_borders.h"
#include "patches/render/get_player_window_count.h"
#include "patches/render/rasterizer_present.h"
#include <d3d11.h>
#include <dxgi1_2.h>

bool DeltaModule::Initialise()
{
	// Find the correct dll (user may not have installed this game)
	GameModule = Inject::FindModule(ModuleName);
	if (GameModule == NULL)
	{
		FORERUNNER_WARN(Delta, "Can't find module, game is likely not installed");
		return false;
	}

	if (!FindGlobals())
	{
		FORERUNNER_ERROR(Delta, "Failed to find all necessary globals");
		return false;
	}

	if (!CreatePatches())
	{
		FORERUNNER_ERROR(Delta, "Failed to create all patches");
		return false;
	}

	if (!ApplyPatches())
	{
		FORERUNNER_ERROR(Delta, "Failed to apply all patches");
		return false;
	}

	if (!PatchCode())
	{
		FORERUNNER_ERROR(Delta, "Failed to apply all inline patches");
		return false;
	}

	FORERUNNER_LOG(Delta, "Successfully patched");

	return true;
}

void DeltaModule::Present()
{
	Test.Draw();
}

ID3D11Device* DeltaModule::GetDevice()
{
	return g_device;
}

ID3D11DeviceContext* DeltaModule::GetDeviceContext()
{
	return g_device_context;
}

ID3D11RenderTargetView* DeltaModule::GetOutputRenderTarget()
{
	return g_output_target;
}

bool DeltaModule::CreatePatches()
{
	bool bSuccess = true;
	bSuccess |= update_player_view_frustum::Create();
	bSuccess |= draw_splitscreen_borders::Create();
	bSuccess |= get_player_window_count::Create();
	bSuccess |= rasterizer_present::Create();

	return bSuccess;
}

bool DeltaModule::ApplyPatches()
{
	bool bSuccess = true;
	bSuccess |= update_player_view_frustum::Enable();
	bSuccess |= draw_splitscreen_borders::Enable();
	bSuccess |= get_player_window_count::Enable();
	bSuccess |= rasterizer_present::Enable();

	return bSuccess;
}

bool DeltaModule::FindGlobals()
{
	bool bSuccess = true;

	bSuccess |= g_players_globals.Find();
	bSuccess |= g_device.Find();
	bSuccess |= g_device_context.Find();
	bSuccess |= g_output_target.Find();

	return true;
}

bool DeltaModule::PatchCode()
{
	bool bSuccess = true;
	Patch::SuspendThreads();
	FORERUNNER_LOG(Delta, "Suspending threads");

	bSuccess |= PatchSplitscreen();

	Patch::ResumeThreads();
	FORERUNNER_LOG(Delta, "Resuming threads");
	return bSuccess;
}


bool DeltaModule::PatchSplitscreen()
{
	// Patch 2 function calls in update_player_views to use player 0 as the target player
	WriteBytes(update_player_views__valid_user_id, {0x31, 0xc9}); // XOR ECX, ECX: sets player_index to 0
	WriteBytes(update_player_views__get_camera_result, {0x31, 0xc9}); // XOR ECX, ECX: sets player_index to 0

	// Remove the slight shrinking applied to the viewport in splitscreen (without the borders between views this is no longer needed)
	WriteBytes(calculate_viewport__left, {0x0});
	WriteBytes(calculate_viewport__right, {0x0});
	WriteBytes(calculate_viewport__bottom, {0x0});
	WriteBytes(calculate_viewport__top, {0x0});

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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

void RenderTest::Init()
{
	FORERUNNER_LOG(Delta, "Render Init");

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
		FORERUNNER_ERROR(Delta, "RegisterClassEx failed");
		return;
	}

	NewWindow = CreateWindowExW(0, WinClass.lpszClassName, L"Forerunner Render Test", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, 0, 0, GetModuleHandleW(0), 0);

	FORERUNNER_LOG(Delta, "Created window");

	// Create swapchain for new window
	ID3D11Device* Device = DeltaModule::Get().GetDevice();
	ID3D11DeviceContext* Context = DeltaModule::Get().GetDeviceContext();

	FORERUNNER_LOG(Delta, "Waiting on DX11");
	while (!Device)
	{
		Sleep(20);
		Device = DeltaModule::Get().GetDevice();
		Context = DeltaModule::Get().GetDeviceContext();
	}

	FORERUNNER_LOG(Delta, "Got device and context ({:#08x} + {:#08x})", reinterpret_cast<int64_t>(Device), reinterpret_cast<int64_t>(Context));

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

	pIDXGIFactory->CreateSwapChainForHwnd(Device, NewWindow, &SwapChainDesc, NULL, NULL, &SwapChain);

	if (!SwapChain)
	{
		FORERUNNER_ERROR(Delta, "Can't create swap chain for window: {}", GetLastError());
		return;
	}

	FORERUNNER_LOG(Delta, "Created swapchain");

	ID3D11Texture2D* BackBuffer;
	SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
	Device->CreateRenderTargetView(BackBuffer, nullptr, &RenderTargetView);
	BackBuffer->Release();
	FORERUNNER_LOG(Delta, "Grabbed backbuffer");

	ShowWindow(NewWindow, SW_SHOW);
	UpdateWindow(NewWindow);
}

void RenderTest::Draw()
{
	//FORERUNNER_LOG(Delta, "Render Draw");
	if (!NewWindow)
	{
		Init();
	}

	MSG Message{};
	while (PeekMessageW(&Message, NewWindow, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	ID3D11Device* Device = DeltaModule::Get().GetDevice();
	ID3D11DeviceContext* Context = DeltaModule::Get().GetDeviceContext();

	ID3D11RenderTargetView* PrevTargetView;
	ID3D11DepthStencilView* DepthStencilView;

	Context->OMGetRenderTargets(1, &PrevTargetView, &DepthStencilView);

	ID3D11RenderTargetView* TargetView = DeltaModule::Get().GetOutputRenderTarget();

	// If we grabbed a valid target view, attempt to blit/copy it to our swapchain backbuffer.
	if (TargetView && SwapChain)
	{
		ID3D11Resource* srcResource = nullptr;
		TargetView->GetResource(&srcResource);
		if (srcResource)
		{
			ID3D11Texture2D* srcTex = nullptr;
			if (SUCCEEDED(srcResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&srcTex)) && srcTex)
			{
				ID3D11Texture2D* backBuffer = nullptr;
				if (SUCCEEDED(SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))) && backBuffer)
				{
					// Compare descs to see if a direct GPU copy is possible.
					D3D11_TEXTURE2D_DESC srcDesc{};
					D3D11_TEXTURE2D_DESC dstDesc{};
					srcTex->GetDesc(&srcDesc);
					backBuffer->GetDesc(&dstDesc);

					// Fast path: same size & format -> GPU CopyResource (no scaling).
					if (srcDesc.Width == dstDesc.Width &&
						srcDesc.Height == dstDesc.Height &&
						srcDesc.Format == dstDesc.Format)
					{
						Context->CopyResource(backBuffer, srcTex);
					}
					else
					{
						// Fallback: copy a clipped region (no scaling). This will copy the top-left portion of the source
						// into the backbuffer. To perform proper scaling/stretching you should render a fullscreen textured
						// quad: create a shader-resource-view for srcTex, set the backbuffer RTV and draw a quad with a
						// vertex/pixel shader that samples the SRV (preferred approach).
						D3D11_BOX srcBox;
						srcBox.left = 0;
						srcBox.top = 0;
						srcBox.front = 0;
						srcBox.right = std::min(srcDesc.Width, dstDesc.Width);
						srcBox.bottom = std::min(srcDesc.Height, dstDesc.Height);
						srcBox.back = 1;
						Context->CopySubresourceRegion(backBuffer, 0, 0, 0, 0, srcTex, 0, &srcBox);
					}

					// Present the swapchain (shows the copied contents)
					SwapChain->Present(0, 0);

					backBuffer->Release();
				}
				srcTex->Release();
			}
			srcResource->Release();
		}
	}
	else
	{
		const float clear_color_with_alpha[4] = {static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 1.0f};
		Context->OMSetRenderTargets(1, &RenderTargetView, nullptr);
		Context->ClearRenderTargetView(RenderTargetView, clear_color_with_alpha);
		// No target view; just present (or you could clear the swapchain RT to indicate no content).
		SwapChain->Present(0, 0);
	}

	Context->OMSetRenderTargets(1, &PrevTargetView, DepthStencilView);
	if (PrevTargetView)
	{
		PrevTargetView->Release();
	}
	if (DepthStencilView)
	{
		DepthStencilView->Release();
	}
}