#include "common/utils/log.h"
#include "common/utils/singleton.h"
#include "payload/forerunner/modulehandler.h"
#include "payload/forerunner/patch.h"
#include <dxgi.h>
#include <cstdint>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;

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
	bool WasLastFrameInVR() const;
	bool HasInitVR() const { return bHasInitVR; }

	void InitVR();

	class IVR* VR = nullptr;

private:
	static HRESULT STDMETHODCALLTYPE HookedPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
	static HRESULT(STDMETHODCALLTYPE* OriginalPresent)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);

	bool CreateBlackTextures(ID3D11Device* Device);

	bool bNextFrameInVR = false;
	bool bLastFrameInVR = false;
	bool bHasInitVR = false;
	bool bOverlayInitialised = false;
	ID3D11Texture2D* BlackTextures[2] = { nullptr, nullptr }; // Left and Right eye black textures
};

PATCH("", 0x1efd44, "48 89 5c 24 10 57 48 83 ec 30 48 8b 05 ?? ?? ?? ?? 48 33 c4 48 89 44 24 28 48 8b d9 48 63 fa 48 8b 0d ?? ?? ?? ?? 48 85 c9 74 ?? 39 bb", CreateGameEngine, void, void* GameState, GameModule GameId);

PATCH("", 0xba2e74, "?? 89 5c ?? 10 ?? 89 6c ?? 18 ?? 89 74 ?? 20 57 ?? 83 ec 20 ?? 8b 12 ?? 8d b1 70 01 00 00 ?? 8b d9 ?? 8b ce", UUIRendererInterface_OnPostResizeBackBuffer, void, void* This, void* BackBuffer);

PATCH("", 0xb95740, "?? 83 ec 28 33 d2 38 91 89 01 00 00 75 ?? ?? 8b 81 60 01 00 00 38 50 38 74 ?? 38 50 39 74", UMCCUUIInterface_InitInternal, void, void* This);

GLOBAL(struct IDXGISwapChain*, UE4SwapChain, "", 0x3fb4e98, "?? 89 0c d0 ?? 2b df 74 ?? ?? 8b 0d ?? ?? ?? ?? e9 ?? ?? ?? ?? ?? 8b 05 ?? ?? ?? ?? ?? 8b 05 ?? ?? ?? ?? ?? 89 05 ?? ?? ?? ?? ?? 89 05 ?? ?? ?? ?? c7 05 ?? ?? ?? ?? 02 00 00 00", +45);

OFFSET(debugger_crash, "", 0x87fdb9, "74 ?? ?? 8b cb ff 15 ?? ?? ?? ?? c7 44 ?? 20 00 10 00 00 ?? 89 7c ?? 28 89 44 ?? 30 c7 44 ?? 34 00 00 00 00 ?? 8d 4c ?? 20 33 d2 b9 88 13 6d 40", +0);