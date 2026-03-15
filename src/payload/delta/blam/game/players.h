#pragma once
#include "blam/memory/data.h"

struct players_globals
{
	int player_count;
	bool unk0;
	bool unk1;
	bool input_disabled;
	bool movement_disabled;
	short player_user_count;
	short player_controller_count;
	datum player_ids[4];
	datum player_objects[4];
	uint8_t unk2[304];
};
static_assert(sizeof(players_globals) == 0x15c);