#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include <d3d11.h>
#include <format>
#include "../../resources/resource.h"

static ID3D11Device* D3D11Device;
static ID3D11DeviceContext* D3D11DeviceContext;
static IDXGISwapChain* D3D11SwapChain;
static ID3D11RenderTargetView* D3D11FrameBufferView;

static bool InitialiseDirectX(HWND Window)
{
	// Create device + swapchain
	
	DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.BufferDesc.Width = 0;
	SwapChainDesc.BufferDesc.Height = 0;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = Window;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Windowed = TRUE;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_0};
	
	HRESULT Result = D3D11CreateDeviceAndSwapChain(0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		FeatureLevels,
		1,
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&D3D11SwapChain,
		&D3D11Device,
		0,
		&D3D11DeviceContext
	);

	if (FAILED(Result))
	{
		MessageBoxA(0, "D3D11CreateDeviceAndSwapChain Failed", std::format("Error: {}", Result).c_str(), MB_OK);
		return false;
	}

	// Create Framebuffer Render Target
	ID3D11Texture2D* FrameBuffer = nullptr;
	Result = D3D11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &FrameBuffer);

	if (FAILED(Result))
	{
		MessageBoxA(0, "D3D11SwapChain->GetBuffer Failed", std::format("Error: {}", Result).c_str(), MB_OK);
		return false;
	}

	if (!FrameBuffer)
	{
		MessageBoxA(0, "FrameBuffer was null", "Error", MB_OK);
		return false;
	}

	D3D11Device->CreateRenderTargetView(FrameBuffer, 0, &D3D11FrameBufferView);

	FrameBuffer->Release();

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	switch (msg)
	{
		case WM_KEYDOWN:
		{
			if (wparam == VK_ESCAPE)
				DestroyWindow(hwnd);
			break;
		}
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInsance, LPSTR lpCmdLine, int nShowCmd)
{
	// Create launcher window
	WNDCLASSEXW WinClass = {};
	WinClass.cbSize = sizeof(WNDCLASSEXW);
	WinClass.style = CS_HREDRAW | CS_VREDRAW;
	WinClass.lpfnWndProc = &WndProc;
	WinClass.hInstance = hInstance;
	WinClass.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ICON));
	WinClass.hCursor = LoadCursorW(0, IDC_ARROW);
	WinClass.lpszClassName = L"ForerunnerClass";
	WinClass.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ICON));

	if (!RegisterClassExW(&WinClass))
	{
		MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
		return GetLastError();
	}

	RECT InitialRect = {0, 0, 1024, 768};
	AdjustWindowRectEx(&InitialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
	LONG InitialWidth = InitialRect.right - InitialRect.left;
	LONG InitialHeight = InitialRect.bottom - InitialRect.top;

	HWND Window = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
		WinClass.lpszClassName,
		L"Forerunner",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		InitialWidth,
		InitialHeight,
		0, 0, hInstance, 0);

	if (!Window)
	{
		MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
		return GetLastError();
	}

	// Create DX11 objects
	if (!InitialiseDirectX(Window))
	{
		return GetLastError();
	}

	// Main update loop
	bool bShouldQuit = false;
	while (!bShouldQuit)
	{
		MSG Message{};
		while (PeekMessageW(&Message, 0, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT)
			{
				bShouldQuit = true;
			}
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		FLOAT backgroundColor[4] = {0.1f, 0.2f, 0.6f, 1.0f};
		D3D11DeviceContext->ClearRenderTargetView(D3D11FrameBufferView, backgroundColor);

		D3D11SwapChain->Present(1, 0);
	}

	return S_OK;
}