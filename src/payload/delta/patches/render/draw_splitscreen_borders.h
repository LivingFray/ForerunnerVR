#pragma once

#include "payload/forerunner/patch.h"

PATCH("halo2.dll", 0x831d90, "40 55 48 8b ec 48 83 ec 20 48 8b 05 ?? ?? ?? ?? 48 85 c0 74 ?? 48 83 3d ?? ?? ?? ?? 00", draw_splitscreen_borders, void);