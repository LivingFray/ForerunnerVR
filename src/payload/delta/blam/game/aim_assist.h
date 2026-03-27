#pragma once
#include <cstdint>
#include "payload/delta/blam/memory/data.h"

struct aim_assist_targeting_result
{
	datum target_object;
	datum model_target;
	datum target_player;
	uint8_t unk0[12];
	uint16_t auto_aim_flags;
	uint8_t unk1[2];
	float primary_auto_aim_level;
	float secondary_auto_aim_level;
};
static_assert(sizeof(aim_assist_targeting_result) == 0x24);