#pragma once
#include <cstdint>
#include "payload/forerunner/patch.h"
#include "payload/delta/blam/memory/data.h"
#include "payload/delta/blam/math/real_math.h"

struct object_header_datum
{
	int16_t id;
	uint8_t unk0;
	uint8_t flags;
	int16_t unk1;
	int16_t size;
	int32_t pool_offset;
};
static_assert(sizeof(object_header_datum) == 0xc);

struct _object_datum
{
	uint32_t flags;
	uint8_t unk0[4];
	datum sibling;
	datum child;
	datum parent;
	uint8_t unk1[74];
	uint16_t time_moving;
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	real_vector3d velocity;
	real_vector3d unk2;
	float scale;
	uint8_t unk3[28];
	uint16_t physics_flags;
	uint8_t unk4[42];
	float health;
	float shield;
	float shield_effect_value;
	uint8_t unk5[4];
	float shield_effect_delta;
	uint8_t unk6[8];
	int8_t shield_effect_time;
	uint8_t unk7[39];

};
static_assert(sizeof(_object_datum) == 0x12c);

struct object_datum
{
	datum definition;
	_object_datum object;
};
static_assert(sizeof(object_datum) == 0x130);

GLOBAL(data_array*, g_objects, "halo2.dll", 0x18b7398, "ba 00 08 00 00 ?? 89 44 ?? 20 e8 ?? ?? ?? ?? ba 00 00 10 00 ?? 89 05 ?? ?? ?? ?? ?? 8d 0d ?? ?? ?? ?? 32 db", +23);
GLOBAL(data_pool*, g_objects_pool, "halo2.dll", 0x18b7360, "ba 00 08 00 00 ?? 89 44 ?? 20 e8 ?? ?? ?? ?? ba 00 00 10 00 ?? 89 05 ?? ?? ?? ?? ?? 8d 0d ?? ?? ?? ?? 32 db", +103);

object_header_datum* objects_get_header_datum(datum index);
object_datum* objects_get_datum(datum index);
