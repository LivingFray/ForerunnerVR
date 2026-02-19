#pragma once
#include <cstdint>
#include "payload/delta/blam/math/real_math.h"
#include "payload/delta/blam/objects/object_location.h"

struct camera_command
{
	uint8_t unk0[4];
	real_point3d position;
	uint8_t unk1[24];
	float field_of_view;
	real_vector3d forward;
	real_vector3d up;
	real_vector3d velocity;
	uint8_t unk2[96];
};
static_assert(sizeof(camera_command) == 0xb0);

struct camera_result
{
	real_point3d position;
	object_location location;
	real_vector3d velocity;
	real_vector3d forward;
	real_vector3d up;
	float hozitonal_field_of_view;
	float vertical_field_of_view;
	uint8_t unk0[40];
};
static_assert(sizeof(camera_result) == 0x68);

struct observer
{
	uint64_t header_signature;
	camera_command* pending_command;
	camera_command command;
	bool updated_for_frame;
	uint8_t padding[3];
	camera_result result;
	uint8_t unk0[12];
	float focus_distance;
	float field_of_view;
	real_vector3d forward;
	real_vector3d up;
	real_matrix4x3 unk_matrix;
	uint8_t unk1[468];
	uint64_t trailer_signature;
};
static_assert(sizeof(observer) == 0x368);
