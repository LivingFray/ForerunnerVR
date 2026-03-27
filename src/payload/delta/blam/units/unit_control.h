#pragma once
#include <cstdint>
#include "payload/delta/blam/game/aim_assist.h"
#include "payload/delta/blam/math/real_math.h"


struct unit_control_data
{
	int32_t string_id;
	int16_t aiming_speed;
	int16_t weapon_index;
	int16_t field3_0x8;
	int16_t grenade_index;
	int16_t zoom_level;	
	uint8_t unk0[2];
	int64_t control_flags;
	real_vector3d throttle;
	float primary_trigger;
	float secondary_trigger;
	real_vector3d facing_vector;
	real_vector3d aiming_vector;
	real_vector3d looking_vector;
	int32_t unk1;
	int32_t unk2;
	int32_t unk3;
	aim_assist_targeting_result target_info;
};
static_assert(sizeof(unit_control_data) == 0x80);