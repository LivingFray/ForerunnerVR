#include "get_player_window_count.h"
#include "payload/delta/deltamodule.h"

int get_player_window_count::Patch()
{
	return DeltaModule::Get().bRenderingHUD ? 1 : 2;
}