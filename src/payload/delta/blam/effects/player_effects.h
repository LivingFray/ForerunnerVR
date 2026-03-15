#pragma once
#include <cstdint>
#include "payload/delta/blam/math/real_math.h"

struct effects_screen_flash
{
	int32_t state;
	float intensity;
	real_argb_color color;
};
static_assert(sizeof(effects_screen_flash) == 0x18);