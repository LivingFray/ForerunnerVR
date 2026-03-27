#include "game.h"
#include "payload/delta/deltamodule.h"

void game_update::Patch(int ticks, float* seconds)
{
	DeltaModule::Get().PreUpdate(ticks, seconds);

	Original(ticks, seconds);

	DeltaModule::Get().PostUpdate(ticks, seconds);
}