#include "players.h"
#include "payload/delta/deltamodule.h"

#include "payload/delta/blam/memory/data.h"
#include "payload/delta/blam/objects/objects.h"
#include "payload/delta/blam/simulation/simulation.h"

int players_get_window_count::Patch()
{
	return DeltaModule::Get().bRenderingHUD ? 1 : 2;
}

void players_update_before_game::Patch(simulation_update* update)
{
	DeltaModule::Get().Camera.UpdatePlayerCamera(update);
	DeltaModule::Get().Input.UpdateInputs(update);

	Original(update);
}

datum players_get_player_id(int16_t user_index)
{
	if (!g_players_globals() || user_index < 0 || user_index >= g_players_globals()->player_count)
	{
		return datum();
	}
	return g_players_globals()->player_ids[user_index];
}

datum players_get_object_id(int16_t user_index)
{
	datum player_id = players_get_player_id(user_index);
	player_datum* player = players_get_datum(player_id);
	if (!player)
	{
		return datum();
	}
	return player->object;
}

object_datum* player_get_object_datum(int16_t user_index)
{
	datum object_id = players_get_object_id(user_index);
	return objects_get_datum(object_id);
}

player_datum* players_get_datum(datum index)
{
    return reinterpret_cast<player_datum*>(data_get_datum(g_players(), index));
}
