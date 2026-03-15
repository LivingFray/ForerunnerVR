#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include "common/utils/singleton.h"
#include "common/utils/log.h"
#include "payload/forerunner/patch.h"
#include "blam/game/players.h"
#include "blam/rasterizer/rasterizer_globals.h"
#include "blam/render/render_cameras.h"

FORERUNNER_CREATE_LOG_CATEGORY(Delta);

// TEMP!!
// Use to test simple d3d11 interactions, later replace this with VR/move into a VR emulator backend
class RenderTest
{
public:
	void ResizeBuffers();
	void Init();

	void Draw();

	struct IDXGISwapChain1* SwapChain = nullptr;
	struct ID3D11RenderTargetView* RenderTargetView;
	struct ID3D11RenderTargetView* MirrorTargetView;
	struct ID3D11RenderTargetView* UITargetView;
	struct ID3D11Texture2D* UITexture;

	HWND NewWindow = NULL;

	UINT UI_WIDTH = 800;
	UINT UI_HEIGHT = 600;
};

class DeltaModule : public Singleton<DeltaModule>
{
public:
	bool Initialise();

	void ResourcesInitialize();
	void Present();

	static struct ID3D11Device* GetDevice();
	static struct ID3D11DeviceContext* GetDeviceContext();
	static struct ID3D11RenderTargetView*& GetOutputRenderTarget();
	static struct rasterizer_globals& GetRasterizerGlobals();

	RenderTest Test;

protected:
	static inline const char* ModuleName = "halo2.dll";
	HMODULE GameModule;

	bool CreatePatches();
	bool ApplyPatches();

	bool FindGlobals();

	bool PatchCode();


	bool PatchSplitscreen();


// --------------
	OFFSET(update_player_views__valid_user_id,      "halo2.dll", 0x9602a8, "ff c3 83 fb 04 7d ?? 8b cb e8 ?? ?? ?? ?? 84 c0 75 ?? ff c3 83 fb 04", +7);
	OFFSET(update_player_views__get_camera_result,  "halo2.dll", 0x9602fc, "be 01 00 00 00 8d 6e 01 eb ?? 83 fb 04 7d ?? 83 fb ff 74 ?? e8 ?? ?? ?? ?? 84 c0 74 ?? 8b cb e8 ?? ?? ?? ?? 48 85 c0 74 ?? 48 89 44 24 30 44 8b cf 89 5c 24 28 44 8b c6 33 d2 44 89 7c 24 20", +29);
	OFFSET(calculate_viewport__left,                "halo2.dll", 0x7e0b35, "66 83 42 02 04 41 0f bf 06 0f bf 0e 03 c8 0f bf 45 00", +4);
	OFFSET(calculate_viewport__right,               "halo2.dll", 0x7e0b56, "66 83 42 06 fc 66 83 7e 02 00 75 ?? 0f b7 44 24 50 66 89 07", +4);
	OFFSET(calculate_viewport__bottom,              "halo2.dll", 0x7e0b6b, "66 83 02 04 41 0f bf 46 02 0f bf 4e 02 03 c8 0f bf 45 02 3b c8", +3);
	OFFSET(calculate_viewport__top,                 "halo2.dll", 0x7e0b9a, "66 83 42 04 fc 48 8b 6c 24 58 48 83 c4 20 41 5e 5f 5e c3", +4);
	OFFSET(interface_draw_screen__window_count,     "halo2.dll", 0x8221f1, "", 0);

	GLOBAL(players_globals*, g_players_globals,     "halo2.dll", 0xe80a20, "41 b8 40 04 00 00 48 8d 0d ?? ?? ?? ?? 48 8b d8 e8 ?? ?? ?? ?? 33 c9 66 89 4b 08", +30);

	GLOBAL(struct ID3D11Device*, g_device,                   "halo2.dll", 0x197ed68, "", 0);
	GLOBAL(struct ID3D11DeviceContext*, g_device_context,    "halo2.dll", 0x197ed70, "", 0);
	GLOBAL(struct ID3D11RenderTargetView*, g_output_target,  "halo2.dll", 0x197ee58, "", 0);

	GLOBAL(rasterizer_globals, g_rasterizer_globals, "halo2.dll", 0x1994858, "", 0);

public:

	bool bRenderingHUD = false;

	//OFFSET(render_hud, "halo2.dll", 0x7e3507, "", 0);
	OFFSET(render_hud, "halo2.dll", 0x6a7635, "", 0);

	OFFSET(unk_render, "halo2.dll", 0x7e350c, "", 0);
	//OFFSET(unk_render2, "halo2.dll", 0x828f70, "", 0);

	//GLOBAL(struct IDXGISwapChain*, g_swap_chain, "halo2.dll", 0x1a23008, "", 0);
	GLOBAL(struct IDXGISwapChain*, g_swap_chain, "halo2.dll", 0x197ee48, "", 0); // "swap_chain_1"

	GLOBAL(void(), rasterizer_refresh, "halo2.dll", 0x37ed0, "", 0);

	//GLOBAL(void(), rasterizer_initialize, "halo2.dll", 0x953050, "", 0);
	FUNCTION("halo2.dll", 0x953050, "", 0, void, rasterizer_initialize);
	GLOBAL(void(int, int), rasterizer_set_display_size, "halo2.dll", 0x954dd0, "", 0);
	GLOBAL(void(), rasterizer_deinitialize, "halo2.dll", 0x953030, "", 0);

	GLOBAL(int, player_window_index, "halo2.dll", 0x165c394, "", 0);
};

PATCH("halo2.dll", 0x829050, "", draw_hud_layer, void);

PATCH("halo2.dll", 0x822190, "", interface_draw_screen, void);

PATCH("halo2.dll", 0x973620, "", rasterizer_render_screen_flash, void);

GLOBAL(render_camera, g_render_camera, "halo2.dll", 0x165c260, "", 0);