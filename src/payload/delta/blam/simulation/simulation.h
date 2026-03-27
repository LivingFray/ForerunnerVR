#pragma once
#include <cstdint>
#include "payload/delta/blam/memory/data.h"
#include "payload/delta/blam/units/unit_control.h"

struct simulation_update
{
	uint8_t unk0[256];
	player_action player_actions[16];
	uint32_t unit_control_mask;
	datum control_unit_index[16];
	uint8_t unk1[4];
	unit_control_data unit_control[16];
	bool machine_update_valid;
	uint8_t unk2[1279];
};
static_assert(sizeof(simulation_update) == 0x1448);
