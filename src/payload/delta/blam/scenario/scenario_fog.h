#pragma once
#include <cstdint>
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/memory/data.h"

struct scenario_fog
{
	int fog_mode;
	real_rgb_color clear_color;
	bool draw_sky;
	bool draw_sky_fog;
	uint8_t padding0[2];
	float view_max_distance;
	bool view_max_distance_changed;
	uint8_t padding1[3];
	real_rgb_color atmospheric_color;
	float atmospheric_max_density;
	float atmospheric_min_distance;
	float atmospheric_max_distance;
	real_rgb_color secondary_color;
	float secondary_max_density;
	float secondary_min_distance;
	float secondary_max_distance;
	uint8_t unk0[16];
	real_rgb_color sky_fog_color;
	float sky_fog_alpha;
	datum patchy_fog_tag_index;
	uint8_t unk1[176];
};
static_assert(sizeof(scenario_fog) == 0x120);