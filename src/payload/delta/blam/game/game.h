#pragma once
#include "payload/forerunner/patch.h"

PATCH("halo2.dll", 0x6a7790, "85 c9 7e ?? 48 89 5c 24 08 48 89 74 24 10 57 48 83 ec 20 48 8b f2 8b f9 33 db 66 0f 1f 44 00 00", game_update, void, int ticks, float* seconds);