#pragma once
#include "payload/forerunner/patch.h"

#include <cstdint>

struct cinematic_globals_definition
{
	float letterbox_amount;
	int32_t letterbox_last_game_time;
	bool letterbox;
	bool unk0;
	bool cinematic_in_progress;
	uint8_t unk1[21];
	int32_t cinematic_title_string_id;
	float cineimatic_title_time;
	uint8_t unk2[4];
};
static_assert(sizeof(cinematic_globals_definition) == 0x2c);

GLOBAL(cinematic_globals_definition*, cinematic_globals, "halo2.dll", 0x15f4928, "?? 39 3d ?? ?? ?? ?? ?? 0f 57 c0 0f 84 a2 04 00 00 ?? 89 70 20 ?? 89 70 d8 0f 29 78 b8 ?? 0f 29 48 98", +3);

FUNCTION("halo2.dll", 0x6f4a20, "?? 8b 05 ?? ?? ?? ?? ?? 85 c0 74 ?? 0f b6 40 0a c3", 0, bool, cinematic_in_progress);
PATCH("halo2.dll", 0x6f4c70, "?? 8b c4 55 53 ?? 57 ?? 8d a8 18 fc ff ff ?? 81 ec d0 04 00 00 ?? 33 ff ?? 0f 29 40 a8 ?? 39 3d ?? ?? ?? ?? ?? 0f 57 c0 0f 84", cinematic_render, void);