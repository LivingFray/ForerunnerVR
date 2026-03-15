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

GLOBAL(players_globals*, g_players_globals, "halo2.dll", 0xe80a20, "41 b8 40 04 00 00 48 8d 0d ?? ?? ?? ?? 48 8b d8 e8 ?? ?? ?? ?? 33 c9 66 89 4b 08", +30);