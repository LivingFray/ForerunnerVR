#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#include "common/utils/singleton.h"
#include "common/utils/log.h"
#include "payload/forerunner/patch.h"
#include "blam/game/players.h"

FORERUNNER_CREATE_LOG_CATEGORY(Delta);

class DeltaModule : public Singleton<DeltaModule>
{
public:
	bool Initialise();

protected:
	static inline const char* ModuleName = "halo2.dll";
	HMODULE GameModule;

	bool CreatePatches();
	bool ApplyPatches();

	bool FindGlobals();

	bool PatchCode();


	bool PatchSplitscreen();


// --------------
	OFFSET("halo2.dll", 0x9602a8, "ff c3 83 fb 04 7d ?? 8b cb e8 ?? ?? ?? ?? 84 c0 75 ?? ff c3 83 fb 04", 7, update_player_views__valid_user_id);
	OFFSET("halo2.dll", 0x9602fc, "be 01 00 00 00 8d 6e 01 eb ?? 83 fb 04 7d ?? 83 fb ff 74 ?? e8 ?? ?? ?? ?? 84 c0 74 ?? 8b cb e8 ?? ?? ?? ?? 48 85 c0 74 ?? 48 89 44 24 30 44 8b cf 89 5c 24 28 44 8b c6 33 d2 44 89 7c 24 20", 29, update_player_views__get_camera_result);

	GLOBAL("halo2.dll", 0xe80a20, "41 b8 40 04 00 00 48 8d 0d ?? ?? ?? ?? 48 8b d8 e8 ?? ?? ?? ?? 33 c9 66 89 4b 08", 30, players_globals*, g_players_globals);
};