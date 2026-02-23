#pragma once
#include "payload/forerunner/patch.h"

PATCH("halo2.dll", 0x6a4380, "48 8b 05 ?? ?? ?? ?? 0f bf 40 08 c3", get_player_window_count, int);