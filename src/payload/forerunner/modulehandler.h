#pragma once

#include "common/utils/singleton.h"
#include "common/utils/log.h"
#include "patch.h"

FORERUNNER_CREATE_LOG_CATEGORY(Forerunner);

enum class GameModule : int32_t
{
	HaloCE,
	Halo2,
	Halo3,
	Halo4,
	Groundhog,
	HaloODST,
	HaloReach,
	NONE
};

template <>
struct std::formatter<GameModule> : std::formatter<std::string_view>
{
	auto format(GameModule mod, std::format_context& ctx) const
	{
		std::string_view name;

		switch (mod)
		{
			case GameModule::HaloCE:    name = "HaloCE"; break;
			case GameModule::Halo2:     name = "Halo2"; break;
			case GameModule::Halo3:     name = "Halo3"; break;
			case GameModule::Halo4:     name = "Halo4"; break;
			case GameModule::Groundhog: name = "Groundhog"; break;
			case GameModule::HaloODST:  name = "HaloODST"; break;
			case GameModule::HaloReach: name = "HaloReach"; break;
			case GameModule::NONE:      name = "NONE"; break;
			default: name = "Unknown"; break;
		}

		// Delegate formatting to the string_view formatter
		return std::formatter<std::string_view>::format(name, ctx);
	}
};

class ModuleHandler : public Singleton<ModuleHandler>
{
public:
	void Initialise();

	void LoadModule(GameModule GameId);

protected:
	GameModule LastGameId = GameModule::NONE;
};

PATCH("", 0x1efd44, "48 89 5c 24 10 57 48 83 ec 30 48 8b 05 ?? ?? ?? ?? 48 33 c4 48 89 44 24 28 48 8b d9 48 63 fa 48 8b 0d ?? ?? ?? ?? 48 85 c9 74 ?? 39 bb", CreateGameEngine, void, void* GameState, GameModule GameId);