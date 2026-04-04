#pragma once
#include<filesystem>

// Configs layout
#include "config_definitions.h"

namespace Config
{
#define CFG_CAT(Name) namespace Name {
#define CFG_CAT_END(Name) };
#define CFG_VALUE(Type, Name, DefaultValue, Description) inline Type Name = DefaultValue;
	FORERUNNER_CONFIGS
#undef CFG_CAT
#undef CFG_CAT_END
#undef CFG_VALUE

	void Load(const std::filesystem::path& File);
	void Save(const std::filesystem::path& File);
};