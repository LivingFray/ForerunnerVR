#include "payload/forerunner/forerunnermodule.h"
#include "common/vr/emulatedvr.h"
#include "common/vr/openvr.h"
#include <MinHook.h>


// Static member definitions
HRESULT(STDMETHODCALLTYPE* ForerunnerModule::OriginalPresent)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags) = nullptr;

using AllPatches = Patch::PatchList<
	CreateGameEngine,
	UUIRendererInterface_OnPostResizeBackBuffer,
	UMCCUUIInterface_InitInternal
>;

void ForerunnerModule::Initialise()
{
	FORERUNNER_LOG(Forerunner, "Initialising Forerunner Module");

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
	}

	// TODO: This should be done at a higher level (i.e. in a forerunner module)
#ifdef USE_EMULATOR
	VR = new EmulatedVR();
#else
	VR = new OpenVR();
#endif

	VR->EarlyInit();
}

void ForerunnerModule::Deinitialise()
{
	FORERUNNER_LOG(Forerunner, "Deinitialising Forerunner Module");
}

void ForerunnerModule::RefreshSwapchain()
{
	IDXGISwapChain* SwapChain = UE4SwapChain();
	FORERUNNER_LOG(Forerunner, "Swapchain: {:#08x}", reinterpret_cast<uint64_t>(SwapChain));

	if (!SwapChain)
	{
		return;
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
	bNextFrameInVR = false;
}

bool ForerunnerModule::IsNextFrameInVR() const
{
	return bNextFrameInVR;
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

	// TODO: UI rendering

	if (!ForerunnerModule::Get().IsNextFrameInVR())
	{
		//FORERUNNER_LOG(Forerunner, "TODO: Draw UI here");
	}

	ForerunnerModule::Get().ClearNextFrameAsVR();

	return Result;
}