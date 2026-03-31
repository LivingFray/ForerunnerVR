#pragma once
#include "payload/delta/blam/math/integer_math.h"

struct rasterizer_globals
{
	int size_x;
	int size_y;
	rectangle2d screen_bounds;
	rectangle2d frame_bounds;
	short unk0;
	short unk1;
	float ui_scale;
};

GLOBAL(rasterizer_globals, g_rasterizer_globals, "halo2.dll", 0x1994858, "f3 0f 10 05 ?? ?? ?? ?? b8 3c 00 00 00 f3 0f 10 0d ?? ?? ?? ?? f3 0f 11 05 ?? ?? ?? ?? f3 0f 10 05 ?? ?? ?? ?? 66 89 05 ?? ?? ?? ?? b8 20 03 00 00", +73);