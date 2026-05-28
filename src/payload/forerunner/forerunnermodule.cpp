#include "payload/forerunner/forerunnermodule.h"
#include "common/vr/emulatedvr.h"
#include "common/vr/openvr.h"
#include "common/vr/IVR.h"
#include <MinHook.h>
#include <d3d11.h>
#include <vector>

// Static member definitions
HRESULT(STDMETHODCALLTYPE* ForerunnerModule::OriginalPresent)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags) = nullptr;

void ForerunnerModule::InitVR()
{
	if (bHasInitVR || !VR)
	{
		return;
	}

	IDXGISwapChain* SwapChain = UE4SwapChain();
	if (!SwapChain)
	{
		return;
	}

	ID3D11Device* Device = nullptr;
	HRESULT hr = SwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&Device);
	if (SUCCEEDED(hr) && Device)
	{
		ID3D11DeviceContext* Context = nullptr;
		Device->GetImmediateContext(&Context);
		if (Context)
		{
			FORERUNNER_LOG(Forerunner, "Successfully retrieved Device and Context from SwapChain");
			if (VR->Init(Device, Context))
			{
				bHasInitVR = true;
				FORERUNNER_LOG(Forerunner, "Successfully initialised VR");
			}
			else
			{
				FORERUNNER_ERROR(Forerunner, "Failed to initialise VR");
			}

			if (CreateBlackTextures(Device))
			{
				FORERUNNER_LOG(Forerunner, "Successfully created black fallback textures");
			}
			else
			{
				FORERUNNER_ERROR(Forerunner, "Failed to create black fallback textures");
			}

			Context->Release();
		}
		Device->Release();
	}

	if (!WasLastFrameInVR())
	{
		VR->ShowOverlay();
	}
}

using AllPatches = Patch::PatchList<
	CreateGameEngine,
	UUIRendererInterface_OnPostResizeBackBuffer,
	UMCCUUIInterface_InitInternal
>;

void ForerunnerModule::Initialise()
{
	FORERUNNER_LOG(Forerunner, "Initialising Forerunner Module");

#ifdef USE_EMULATOR
	VR = new EmulatedVR();
#else
	VR = new OpenVR();
#endif

	VR->EarlyInit();

	if (!AllPatches::CreateAll() || !AllPatches::EnableAll())
	{
		FORERUNNER_ERROR(Forerunner, "Couldn't patch MCC");
		return;
	}
	else
	{
		FORERUNNER_LOG(Forerunner, "Successfully patched MCC");
	}

	bool bFoundGlobals = true;
	bFoundGlobals |= UE4SwapChain.Find();

	if (!bFoundGlobals)
	{
		FORERUNNER_ERROR(Forerunner, "Couldn't find all MCC globals");
		return;
	}
	else
	{
		FORERUNNER_LOG(Forerunner, "Successfully found all MCC globals");
	}

	// Injection didn't occur until after UE4's start up completed, hook rendering immediately
	if (UE4SwapChain())
	{
		RefreshSwapchain();
		InitVR();
	}
}

void ForerunnerModule::Deinitialise()
{
	FORERUNNER_LOG(Forerunner, "Deinitialising Forerunner Module");

	if (VR)
	{
		VR->Shutdown();
		delete VR;
		VR = nullptr;
	}

	for (int i = 0; i < 2; ++i)
	{
		if (BlackTextures[i])
		{
			BlackTextures[i]->Release();
			BlackTextures[i] = nullptr;
		}
	}
}

void ForerunnerModule::RefreshSwapchain()
{
	IDXGISwapChain* SwapChain = UE4SwapChain();
	FORERUNNER_LOG(Forerunner, "Swapchain: {:#08x}", reinterpret_cast<uint64_t>(SwapChain));

	if (!SwapChain)
	{
		return;
	}

	// TODO: This is untested with multiple calls to this function (e.g. from resizing game window)
	InitVR();

	if (OriginalPresent != nullptr)
	{
		return; // Already hooked
	}

	// Get the Present method from the vtable (IDXGISwapChain::Present is the 8th virtual method)
	// The vtable layout for IDXGISwapChain:
	// 0: QueryInterface
	// 1: AddRef
	// 2: Release
	// 3: SetPrivateData
	// 4: SetPrivateDataInterface
	// 5: GetPrivateData
	// 6: GetParent
	// 7: GetDevice
	// 8: Present
	void** VTable = *reinterpret_cast<void***>(SwapChain);
	void* PresentMethod = VTable[8];

	if (!PresentMethod)
	{
		FORERUNNER_ERROR(Forerunner, "Failed to get Present method from swapchain vtable");
		return;
	}

	MH_STATUS Status = MH_CreateHook(PresentMethod, &ForerunnerModule::HookedPresent, reinterpret_cast<LPVOID*>(&OriginalPresent));
	if (Status != MH_OK)
	{
		FORERUNNER_ERROR(Forerunner, "Failed to hook Present method: {}", MH_StatusToString(Status));
		return;
	}

	Status = MH_EnableHook(PresentMethod);
	if (Status != MH_OK)
	{
		FORERUNNER_ERROR(Forerunner, "Failed to enable Present hook: {}", MH_StatusToString(Status));
		MH_RemoveHook(PresentMethod);
		return;
	}

	FORERUNNER_LOG(Forerunner, "Successfully hooked IDXGISwapChain::Present");
}

void ForerunnerModule::MarkFrameAsVR()
{
	bNextFrameInVR = true;
}

void ForerunnerModule::ClearNextFrameAsVR()
{
	bLastFrameInVR = bNextFrameInVR;
	bNextFrameInVR = false;
}

bool ForerunnerModule::IsNextFrameInVR() const
{
	return bNextFrameInVR;
}

bool ForerunnerModule::WasLastFrameInVR() const
{
	return bLastFrameInVR;
}

bool ForerunnerModule::CreateBlackTextures(ID3D11Device* Device)
{
	if (!Device)
	{
		return false;
	}

	// Get VR dimensions for the texture size
	if (!VR)
	{
		return false;
	}

	int32_t Width = VR->GetDesiredWidth();
	int32_t Height = VR->GetDesiredHeight();

	if (Width <= 0 || Height <= 0)
	{
		FORERUNNER_ERROR(Forerunner, "Invalid VR dimensions for black texture creation: {}x{}", Width, Height);
		return false;
	}

	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = Width;
	TextureDesc.Height = Height;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// Initialize with black (0, 0, 0, 255)
	uint32_t BlackPixel = 0xFF000000;
	std::vector<uint32_t> BlackData(Width * Height, BlackPixel);

	D3D11_SUBRESOURCE_DATA InitialData{};
	InitialData.pSysMem = BlackData.data();
	InitialData.SysMemPitch = Width * sizeof(uint32_t);

	for (int i = 0; i < 2; ++i)
	{
		HRESULT hr = Device->CreateTexture2D(&TextureDesc, &InitialData, &BlackTextures[i]);
		if (FAILED(hr) || !BlackTextures[i])
		{
			FORERUNNER_ERROR(Forerunner, "Failed to create black texture for eye {}: 0x{:08x}", i, static_cast<uint32_t>(hr));
			return false;
		}
	}

	return true;
}


void CreateGameEngine::Patch(void* GameState, GameModule GameId)
{
	Original(GameState, GameId);

	ModuleHandler::Get().LoadModule(GameId);
}

void UUIRendererInterface_OnPostResizeBackBuffer::Patch(void* This, void* BackBuffer)
{
	Original(This, BackBuffer);

	ForerunnerModule::Get().RefreshSwapchain();
}

void UMCCUUIInterface_InitInternal::Patch(void* This)
{
	Original(This);

	ForerunnerModule::Get().RefreshSwapchain();
}

HRESULT STDMETHODCALLTYPE ForerunnerModule::HookedPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
{
	// Call the original Present method
	HRESULT Result = OriginalPresent(This, SyncInterval, Flags);

	// Passthrough for any other swapchains
	if (This != UE4SwapChain())
	{
		return Result;
	}

	ForerunnerModule& Module = ForerunnerModule::Get();

	if (!Module.HasInitVR())
	{
		Module.ClearNextFrameAsVR();
		return Result;
	}

	if (!Module.IsNextFrameInVR())
	{
		// Update poses/inputs on the rendering thread when in shell/menus
		Module.VR->Update(0.016f); // TODO: Get a real value for here

		// Submit black textures while UI is up
		Module.VR->SubmitEye(EVR_Eye::Left, Module.BlackTextures[0]);
		Module.VR->SubmitEye(EVR_Eye::Right, Module.BlackTextures[1]);

		ID3D11Texture2D* BackBuffer = nullptr;
		HRESULT hr = This->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
		if (SUCCEEDED(hr) && BackBuffer)
		{
			ID3D11Device* Device = nullptr;
			BackBuffer->GetDevice(&Device);
			if (Device)
			{
				ID3D11DeviceContext* Context = nullptr;
				Device->GetImmediateContext(&Context);
				if (Context)
				{
					// Create black textures on first use, passing the device
					if (!Module.BlackTextures[0] || !Module.BlackTextures[1])
					{
						Module.CreateBlackTextures(Device);
					}

					// Show the overlay while in the UI
					if (Module.WasLastFrameInVR())
					{
						Module.VR->ShowOverlay();
					}

					Module.VR->DrawOverlay(Context, BackBuffer);
					Context->Release();
				}
				Device->Release();
			}
			BackBuffer->Release();
		}
	}
	else
	{
		// Hide the overlay while rendering the game
		if (!Module.WasLastFrameInVR())
		{
			Module.VR->HideOverlay();
		}
	}

	Module.VR->EndFrame();

	Module.ClearNextFrameAsVR();

	return Result;
}