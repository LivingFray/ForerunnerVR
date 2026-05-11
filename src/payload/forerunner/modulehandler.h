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
	void Initialise(wchar_t* InDLLPath);

	void LoadModule(GameModule GameId);

protected:
	GameModule LastGameId = GameModule::NONE;
	wchar_t DLLPath[MAX_PATH];
	HMODULE OpenVRHandle = nullptr;
};