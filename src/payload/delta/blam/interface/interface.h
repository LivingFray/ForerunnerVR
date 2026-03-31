#pragma once
#include "payload/forerunner/patch.h"

PATCH("halo2.dll", 0x822190, "48 83 ec 28 80 3d ?? ?? ?? ?? 00 48 89 5c 24 30 48 89 74 24 40 75 ?? f3 0f 10 15 ?? ?? ?? ?? f3 0f 10 0d ?? ?? ?? ?? 0f 28 c2", interface_draw_screen, void);
PATCH("halo2.dll", 0x831d90, "40 55 48 8b ec 48 83 ec 20 48 8b 05 ?? ?? ?? ?? 48 85 c0 74 ?? 48 83 3d ?? ?? ?? ?? 00", interface_draw_splitscreen_borders, void);

// This probably doesn't go in this file (or have this name), but I don't fully understand it atm
struct hud_scaling
{
	float main;
	float crosshair;
};

GLOBAL(hud_scaling, g_hud_scaling, "halo2.dll", 0xe14f28, "3b c1 0f 4f c1 83 f8 01 7e ?? f3 0f 10 05 ?? ?? ?? ?? f3 0f 10 0d ?? ?? ?? ?? f3 0f 11 05", +30);