#include "players.h"
#include "payload/delta/deltamodule.h"

#include "payload/delta/blam/memory/data.h"
#include "payload/delta/blam/simulation/simulation.h"

int players_get_window_count::Patch()
{
	return DeltaModule::Get().bRenderingHUD ? 1 : 2;
}

void players_update_before_game::Patch(simulation_update* update)
{
	DeltaModule::Get().Camera.UpdatePlayerCamera(update);

	Original(update);
}