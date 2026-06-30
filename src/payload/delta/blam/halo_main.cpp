#include "halo_main.h"

void h2a2_change_render_mode::Patch()
{
	target_graphics_mode() = 0;

	Original();
}