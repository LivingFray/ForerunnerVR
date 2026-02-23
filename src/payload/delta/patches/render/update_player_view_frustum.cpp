#include "update_player_view_frustum.h"
#include "payload/delta/deltamodule.h"

void update_player_view_frustum::Patch(render_window* render_window, int param2, int view_index, int view_count, int param5, int player_index, camera_result* result)
{
	// Force player index to 0 so both views render the first player's perspective
	Original(render_window, param2, view_index, view_count, param5, 0, result);
}