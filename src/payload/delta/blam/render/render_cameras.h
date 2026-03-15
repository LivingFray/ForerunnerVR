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