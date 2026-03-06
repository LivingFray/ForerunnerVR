#pragma once
#include "blam/math/integer_math.h"

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