#pragma once
#include <cstdint>
#include "payload/delta/blam/math/integer_math.h"
#include "payload/delta/blam/math/real_math.h"

struct render_camera
{
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	bool field3_0x24;
	uint8_t field4_0x25;
	uint8_t field5_0x26;
	uint8_t field6_0x27;
	float vertical_field_of_view;
	float field8_0x2c;
	rectangle2d viewport_bounds;
	rectangle2d window_bounds;
	float z_near;
	float z_far;
	real_plane3d mirror_plane;
	bool use_letterboxing;
	uint8_t field15_0x59;
	uint8_t field16_0x5a;
	uint8_t field17_0x5b;
	float centre_bias_x;
	float centre_bias_y;
	float crop_factor;
	bool use_safe_area;
	uint8_t field22_0x69;
	uint8_t field23_0x6a;
	uint8_t field24_0x6b;
	float safe_area_scale_x;
	float safe_area_scale_y;
};
static_assert(sizeof(render_camera) == 0x74);

struct render_projection
{
	real_matrix4x3 world_to_view;
	real_matrix4x3 view_to_world;
	real_rectangle2d projection_bounds;
	real_matrix4x4 projection_matrix;
	real_vector2d scale;
};
static_assert(sizeof(render_projection) == 0xc0);

GLOBAL(render_camera, g_render_camera, "halo2.dll", 0x165c260, "0f 11 41 e0 0f 10 48 f0 0f 11 49 f0 48 83 ea 01 75 ?? 0f 10 00 4c 8d 05 ?? ?? ?? ?? 0f 11 01 0f 10 48 10 0f 11 49 10 88 15 ?? ?? ?? ?? 48 8d 0d ?? ?? ?? ??", +48);

GLOBAL(int, player_window_index, "halo2.dll", 0x165c394, "48 89 5c 24 30 48 8d 1d ?? ?? ?? ?? 48 03 cb e8 ?? ?? ?? ?? 48 63 05 ?? ?? ?? ?? 48 8d 8b c0 01 00 00 4c 69 c0 18 03 00 00 ba 02 00 00 00 49 03 c8 41 0f bf 04 18 89 05", +56);