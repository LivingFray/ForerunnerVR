#include "players.h"
#include "payload/delta/deltamodule.h"

int players_get_window_count::Patch()
{
	return DeltaModule::Get().bRenderingHUD ? 1 : 2;
}