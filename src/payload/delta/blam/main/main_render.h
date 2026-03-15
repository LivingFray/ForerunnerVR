#pragma once

#include "payload/forerunner/patch.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"
#include "payload/delta/blam/camera/observer.h"

PATCH("halo2.dll", 0x960780, "40 53 55 56 57 41 57 48 81 ec 90 00 00 00 8b 84 24 e8 00 00 00 45 8b f9", compute_window_bounds, void, render_window* render_window, int param2, int view_index, int view_count, int param5, int player_index, camera_result* result);