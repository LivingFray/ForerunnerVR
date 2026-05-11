#include "common/utils/log.h"
#include "common/utils/singleton.h"
#include "payload/forerunner/modulehandler.h"
#include "payload/forerunner/patch.h"
#include <dxgi.h>

class ForerunnerModule : public Singleton<ForerunnerModule>
{
public:
	void Initialise();
	void Deinitialise();

	void RefreshSwapchain();

	// Indicate that this frame has been intercepted by a game module and the game shouldn't be treated as in a menu
	void MarkFrameAsVR();
	// Reset flag for if this frame should be drawn in VR. Shouldn't be called by game modules
	void ClearNextFrameAsVR();

	bool IsNextFrameInVR() const;


	class IVR* VR = nullptr;

private:
	static HRESULT STDMETHODCALLTYPE HookedPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
	static HRESULT(STDMETHODCALLTYPE* OriginalPresent)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);

	bool bNextFrameInVR = false;
};

PATCH("", 0x1efd44, "48 89 5c 24 10 57 48 83 ec 30 48 8b 05 ?? ?? ?? ?? 48 33 c4 48 89 44 24 28 48 8b d9 48 63 fa 48 8b 0d ?? ?? ?? ?? 48 85 c9 74 ?? 39 bb", CreateGameEngine, void, void* GameState, GameModule GameId);

PATCH("", 0xba2e74, "", UUIRendererInterface_OnPostResizeBackBuffer, void, void* This, void* BackBuffer);

PATCH("", 0xb95740, "", UMCCUUIInterface_InitInternal, void, void* This);

GLOBAL(struct IDXGISwapChain*, UE4SwapChain, "", 0x3fb4e98, "", 0);