#include "cinematics.h"
#include "common/config/config.h"

void cinematic_render::Patch()
{
	// Force disable letterboxing by setting the amount to a negative amount if the setting is enabled
	if (Config::Forerunner::UI::Cinematics::DisableLetterboxing)
	{
		cinematic_globals()->letterbox_amount = -1.0f;
	}

	Original();
}