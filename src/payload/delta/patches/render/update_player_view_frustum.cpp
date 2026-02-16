#include "update_player_view_frustum.h"


void update_player_view_frustum::Patch(render_window* render_window, int param2, int view_index, int view_count, int param5, int player_index, camera_result* result)
{
	Original(render_window, param2, view_index, view_count, param5, player_index, result);

	std::cout << "Intercepted update_player_view_frustum" << std::endl;
}