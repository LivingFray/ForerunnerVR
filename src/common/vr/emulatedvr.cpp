#include "emulatedvr.h"
#include "common/utils/utils.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <numbers>

static constexpr int32_t VR_WIDTH = 1600;
static constexpr int32_t VR_HEIGHT = 1200;

bool EmulatedVR::EarlyInit()
{
	FORERUNNER_LOG(EmuVR, "Performing Early initialisation");

	return true;
}

bool EmulatedVR::Init(struct ID3D11Device* InDevice, struct ID3D11DeviceContext* InContext)
{
	FORERUNNER_LOG(EmuVR, "Initialising");

	Device = InDevice;
	DeviceContext = InContext;

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

	float NewYaw = CameraYaw;
	float NewPitch = CameraPitch;

	// Check numpad keys for camera rotation
	if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
	{
		NewYaw -= DeltaTime;
	}
	if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
	{
		NewYaw += DeltaTime;
	}
	if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000)
	{
		NewPitch += DeltaTime;
	}
	if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000)
	{
		NewPitch -= DeltaTime;
	}

	// Limit rotation values
	NewYaw = fmodf(NewYaw, std::numbers::pi_v<float> * 2.0f);
	NewPitch = std::min(NewPitch, std::numbers::pi_v<float> * 0.5f);
	NewPitch = std::max(NewPitch, -std::numbers::pi_v<float> * 0.5f);

	{
		std::lock_guard<std::mutex> Lock(PoseMutex);
		CameraYaw = NewYaw;
		CameraPitch = NewPitch;
	}

	UpdateKeyInputs();
}

void EmulatedVR::SubmitEye(EVR_Eye Eye, ID3D11Texture2D* Texture, const VR_Bounds& ViewBounds)
{
	{
		ID3D11Texture2D* srcTex = Texture;

		if (!srcTex)
		{
			FORERUNNER_ERROR(EmuVR, "SubmitEye called with invalid Texture");
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
		srcBox.top = srcDesc.Height - static_cast<UINT>(ViewBounds.y * srcDesc.Height + ViewBounds.h * srcDesc.Height);
		srcBox.front = 0;
		srcBox.right = static_cast<UINT>(ViewBounds.x * srcDesc.Width + ViewBounds.w * srcDesc.Width);
		srcBox.bottom = srcDesc.Height - static_cast<UINT>(ViewBounds.y * srcDesc.Height);
		srcBox.back = 1;

		UINT StartX = (Eye == EVR_Eye::Left) ? 0 : VR_WIDTH;

		DeviceContext->CopySubresourceRegion(dstResource, 0, StartX, 0, 0, srcTex, 0, &srcBox);

		dstTex->Release();
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

void EmulatedVR::EndFrame()
{
}

int32_t EmulatedVR::GetDesiredWidth() const
{
	return VR_WIDTH;
}

int32_t EmulatedVR::GetDesiredHeight() const
{
	return VR_HEIGHT;
}

float EmulatedVR::GetVerticalFieldOfView(EVR_Eye Eye) const
{
    return Deg2Rad(90.0f);
}

Matrix4 EmulatedVR::GetHMDTransform() const
{
	float Yaw, Pitch;
	Vector3 Offset;
	{
		std::lock_guard<std::mutex> Lock(PoseMutex);
		Yaw = CameraYaw;
		Pitch = CameraPitch;
		Offset = CameraOffset;
	}

	// Convert pitch and yaw into rotation transform, then add camera offset
	Matrix4 Rotation = Matrix4().rotateZ(Rad2Deg(-Yaw)) * Matrix4().rotateY(Rad2Deg(-Pitch));

	return Rotation * Matrix4().translate(Offset);
}

Matrix4 EmulatedVR::GetEyeTransform(EVR_Eye Eye) const
{
	switch (Eye)
	{
		case EVR_Eye::Left:
			return Matrix4().translate(0.0f, 0.05f, 0.0f);
		case EVR_Eye::Right:
			return Matrix4().translate(0.0f, -0.05f, 0.0f);
		default:
			FORERUNNER_WARN(EmuVR, "Unexpected value for Eye passed to GetEyeTransform: {}", static_cast<int>(Eye));
			return Matrix4();
	}
}

Matrix4 EmulatedVR::GetControllerTransform(EVR_Controller Controller) const
{
	Matrix4 ControllerOffset;

	switch (Controller)
	{
		case EVR_Controller::Left:
			ControllerOffset.translate(0.05f, 0.1f, -0.1f);
			break;
		case EVR_Controller::Right:
			ControllerOffset.translate(0.05f, -0.1f, -0.1f);
			break;
		default:
			FORERUNNER_WARN(EmuVR, "Unexpected value for Controller passed to GetControllerTransform: {}", static_cast<int>(Controller));
			break;
	}

	return GetHMDTransform() * ControllerOffset;
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

// TODO: Need a (slightly) better way to do this, so that the inputs aren't game dependent. Maybe create a simple json file with bindings and parse that

struct Binding
{
	std::string bindingName;
	int virtualKey = 0;
	bool bHasChanged = false;
	bool bPressed = false;
};

// Copied from HaloCEVR. Will need replacing with new bindings
Binding bindings[13] = {
	{"Jump", VK_SPACE},
	{"SwitchGrenades", 'G'},
	{"Interact", 'E'},
	{"SwitchWeapons", VK_TAB},
	{"Melee", 'Q'},
	{"Flashlight", 'F'},
	{"Grenade", VK_RBUTTON},
	{"Fire", VK_LBUTTON},
	{"MenuBack", 'P'}, // Intentionally weird binding because we don't override this in the same way and it would conflict
	{"Crouch", VK_LCONTROL},
	{"Zoom", 'Z'},
	{"Reload", 'R'},
	{"EMU_MoveHandSwap", 'H'}
};

struct AxisBinding
{
	int virtualKey = 0;
	int scale = 0;
	int axisId = 0;
};

struct Axis2D
{
	std::string axisName;
	int indexX = 0;
	int indexY = 0;
};

float axes1D[6] = {
	0.0f,
	0.0f,
	0.0f,
	0.0f,
	0.0f
};

Axis2D axes2D[3] =
{
	{"Move", 0, 1},
	{"EMU_MoveHandFlat", 2, 3},
	{"EMU_MoveHandVert", 4, 5}
};

AxisBinding axisBindings[10] =
{
	{'W', 1, 1},
	{'S', -1, 1},
	{'A', -1, 0},
	{'D', 1, 0},
	{'I', 1, 3},
	{'K', -1, 3},
	{'J', -1, 2},
	{'L', 1, 2},
	{'U', -1, 4},
	{'O', 1, 4},
};

template<typename T, std::size_t N>
constexpr std::size_t ArraySize(T(&)[N])
{
	return N;
}

InputBindingID EmulatedVR::RegisterBoolInput(const std::string& Set, const std::string& Action)
{
	for (size_t i = 0; i < ArraySize(bindings); i++)
	{
		if (bindings[i].bindingName == Action)
		{
			FORERUNNER_LOG(EmuVR, "Registered Bool input {} with id {}", Action, i);
			return i;
		}
	}
	FORERUNNER_LOG(EmuVR, "Registered Bool input {} with id {}", Action, 999);
	return 999;
}

InputBindingID EmulatedVR::RegisterVector2Input(const std::string& Set, const std::string& Action)
{
	for (size_t i = 0; i < ArraySize(axes2D); i++)
	{
		if (axes2D[i].axisName == Action)
		{
			FORERUNNER_LOG(EmuVR, "Registered Vector2 input {} with id {}", Action, i);
			return i;
		}
	}
	FORERUNNER_LOG(EmuVR, "Registered Vector2 input {} with id {}", Action, 999);
	return 999;
}

bool EmulatedVR::GetBoolInput(InputBindingID ID) const
{
	static bool bDummy = false;
	return GetBoolInput(ID, bDummy);
}

bool EmulatedVR::GetBoolInput(InputBindingID ID, bool& bHasChanged) const
{
	if (ID < ArraySize(bindings))
	{
		bHasChanged = bindings[ID].bHasChanged;
		return bindings[ID].bPressed;
	}
	return false;
}

Vector2 EmulatedVR::GetVector2Input(InputBindingID ID) const
{
	if (ID < ArraySize(axes2D))
	{
		return Vector2(axes1D[axes2D[ID].indexX], axes1D[axes2D[ID].indexY]);
	}
	return Vector2();
}

void EmulatedVR::UpdateKeyInputs()
{
	for (size_t i = 0; i < ArraySize(bindings); i++)
	{
		bool bPressed = GetAsyncKeyState(bindings[i].virtualKey) & 0x8000;
		bindings[i].bHasChanged = bPressed != bindings[i].bPressed;
		bindings[i].bPressed = bPressed;
	}

	for (size_t i = 0; i < ArraySize(axes1D); i++)
	{
		axes1D[i] = 0.0f;
	}

	for (size_t i = 0; i < ArraySize(axisBindings); i++)
	{
		bool bPressed = GetAsyncKeyState(axisBindings[i].virtualKey) & 0x8000;
		if (bPressed)
		{
			axes1D[axisBindings[i].axisId] += axisBindings[i].scale * 1.0f;
		}
	}

	/*
	// Respond to fake inputs used to control gun hand
	Vector2 handMoveFlat = GetVector2Input(inputMoveHandFlat) * Game::instance.lastDeltaTime;
	Vector2 handMoveVert = GetVector2Input(inputMoveHandVert) * Game::instance.lastDeltaTime;

	// Swap between moving/rotating
	bool bhandModeChanged;
	bool bSwapHandMove = GetBoolInput(inputMoveHandSwap, bhandModeChanged);
	if (bhandModeChanged && bSwapHandMove)
	{
		bMoveHand ^= true;
	}

	constexpr float moveSpeed = 0.5f;
	constexpr float rotSpeed = 180.0f;

	if (bMoveHand)
	{
		mainHandOffset.x += handMoveFlat.x * moveSpeed;
		mainHandOffset.y += handMoveFlat.y * moveSpeed;
		mainHandOffset.z += handMoveVert.x * moveSpeed;
	}
	else
	{
		mainHandRot.x += handMoveFlat.x * rotSpeed;
		mainHandRot.y += handMoveFlat.y * rotSpeed;
		mainHandRot.z += handMoveVert.x * rotSpeed;
	}
	*/
}

InputBindingID EmulatedVR::RegisterActionSet(const std::string& Set)
{
	return InputBindingID();
}

void EmulatedVR::ActivateActionSet(InputBindingID ID)
{
}

void EmulatedVR::DeactivateActionSet(InputBindingID ID)
{
}

void EmulatedVR::DrawOverlay(ID3D11DeviceContext* Context, ID3D11Texture2D* SourceTexture)
{
}

void EmulatedVR::ShowOverlay()
{
}

void EmulatedVR::HideOverlay()
{
}
