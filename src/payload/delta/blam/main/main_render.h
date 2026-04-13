#pragma once

#include "payload/forerunner/patch.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"
#include "payload/delta/blam/camera/observer.h"
#include "payload/delta/blam/memory/data.h"
#include "payload/delta/blam/math/real_math.h"
// In Blam 1 unit = 10 feet = 3.048m
const static float WORLD_TO_METRES = 3.048f;
const static float METRES_TO_WORLD = 1 / WORLD_TO_METRES;

PATCH("halo2.dll", 0x960780, "40 53 55 56 57 41 57 48 81 ec 90 00 00 00 8b 84 24 e8 00 00 00 45 8b f9", compute_window_bounds, void, render_window* render_window, int param2, int view_index, int view_count, int param5, int player_index, camera_result* result);
FUNCTION("halo2.dll", 0x8d6000, "?? 89 5c ?? 08 57 ?? 83 ec 20 ?? 8b da 8b f9 e8 ?? ?? ?? ?? 84 c0 74 ?? ?? 8b c3 ?? 8b 5c ?? 30 ?? 83 c4 20 5f c3", 0, real_vector3d*, interpolation_get_object_position, datum param1, real_vector3d* param2);