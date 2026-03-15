#pragma once
#include <cstdint>
#include "payload/delta/blam/render/render_cameras.h"
#include "payload/delta/blam/scenario/scenario_fog.h"
#include "payload/delta/blam/effects/player_effects.h"
#include "payload/forerunner/patch.h"

struct render_window
{
	int single_view;
	int player_window_index;
	int player_index;
	struct render_camera render_camera;
	struct render_camera rasterizer_camera;
	uint8_t field5_0xf4;
	uint8_t field6_0xf5;
	uint8_t field7_0xf6;
	uint8_t field8_0xf7;
	uint8_t field9_0xf8;
	uint8_t field10_0xf9;
	uint8_t field11_0xfa;
	uint8_t field12_0xfb;
	uint8_t field13_0xfc;
	uint8_t field14_0xfd;
	uint8_t field15_0xfe;
	uint8_t field16_0xff;
	uint8_t field17_0x100;
	uint8_t field18_0x101;
	uint8_t field19_0x102;
	uint8_t field20_0x103;
	uint8_t field21_0x104;
	uint8_t field22_0x105;
	uint8_t field23_0x106;
	uint8_t field24_0x107;
	uint8_t field25_0x108;
	uint8_t field26_0x109;
	uint8_t field27_0x10a;
	uint8_t field28_0x10b;
	uint8_t field29_0x10c;
	uint8_t field30_0x10d;
	uint8_t field31_0x10e;
	uint8_t field32_0x10f;
	uint8_t field33_0x110;
	uint8_t field34_0x111;
	uint8_t field35_0x112;
	uint8_t field36_0x113;
	uint8_t field37_0x114;
	uint8_t field38_0x115;
	uint8_t field39_0x116;
	uint8_t field40_0x117;
	uint8_t field41_0x118;
	uint8_t field42_0x119;
	uint8_t field43_0x11a;
	uint8_t field44_0x11b;
	uint8_t field45_0x11c;
	uint8_t field46_0x11d;
	uint8_t field47_0x11e;
	uint8_t field48_0x11f;
};
static_assert(sizeof(render_window) == 0x120);


struct window_parameters
{
	short unk0;
	short unk1;
	uint8_t unk2[4];
	short unk3;
	uint8_t unk4[2];
	real_rgb_color clear_color;
	render_camera camera;
	short window_bound_index;
	uint8_t unk5[114];
	render_projection projection;
	scenario_fog fog;
	bool unk6;
	uint8_t unk7[3];
	effects_screen_flash screen_flash;
	uint8_t unk8[28];
};
static_assert(sizeof(window_parameters) == 0x318);

// Accessed in rasterizer_window_state_apply, which is called by rasterizer_window_begin + rasterizer_window_end
GLOBAL(window_parameters, global_window_parameters, "halo2.dll", 0x1996a10, "48 89 74 24 10 57 48 83 ec 20 48 8b f9 48 8d 15 ?? ?? ?? ?? 48 8b c1 b9 06 00 00 00 0f 1f 40 00 48 8d 92 80 00 00 00", +16);

static inline const char* Signature_d3d11_devices = "48 8b 0d ?? ?? ?? ?? 48 85 c9 74 ?? 48 8b 01 ff 90 70 03 00 00 48 8b 0d ?? ?? ?? ?? 48 8b 01 ff 90 78 03 00 00 48 8b 0d ?? ?? ?? ?? 48 8b 15 ?? ?? ?? ?? 33 db 48 85 d2 74 ?? 48 8b 02 48 8b ca ff 50 10";

GLOBAL(struct ID3D11Device*, g_device, "halo2.dll", 0x197ed68, Signature_d3d11_devices, +137);
GLOBAL(struct ID3D11DeviceContext*, g_device_context, "halo2.dll", 0x197ed70, Signature_d3d11_devices, +3);
GLOBAL(struct IDXGISwapChain*, g_swap_chain, "halo2.dll", 0x197ee48, Signature_d3d11_devices, +77);
GLOBAL(struct ID3D11RenderTargetView*, g_output_target, "halo2.dll", 0x197ee58, "48 8b 05 ?? ?? ?? ?? 48 85 c0 74 ?? 48 8b 0d ?? ?? ?? ?? ff d0 48 8b 0d ?? ?? ?? ?? 48 85 c9 74 ?? 48 8b 01 ff 50 10 48 89 1d ?? ?? ?? ?? 48 83 c4 20 5b c3", +15);