#pragma once

#include "payload/forerunner/patch.h"
#include "payload/delta/blam/render/render.h"
#include "payload/delta/blam/render/render_cameras.h"
#include "payload/delta/blam/camera/observer.h"

PATCH("halo2.dll", 0x960780, "", update_player_view_frustum, void, render_window* render_window, int param2, int view_index, int view_count, int param5, int player_index, camera_result* result);