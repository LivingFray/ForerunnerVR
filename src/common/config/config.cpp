#include "config.h"
#include "common/utils/log.h"
#include <json.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

FORERUNNER_CREATE_LOG_CATEGORY(Config);

void Config::Load(const std::filesystem::path& File)
{
	// Config doesn't exist yet, save with the default values
	if (!std::filesystem::exists(File))
	{
		char AnsiBuffer[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, File.c_str(), static_cast<int>(wcslen(File.c_str())) + 1, AnsiBuffer, sizeof(AnsiBuffer), NULL, NULL);

		FORERUNNER_WARN(Config, "Couldn't find config file at {}, creating new file with default values", AnsiBuffer);
		Save(File);
		return;
	}

	std::ifstream FileStream{File};

	nlohmann::json JsonData = nlohmann::json::parse(FileStream);
	nlohmann::json* CurrentObject = &JsonData;
	std::vector<nlohmann::json*> CategoryStack;
	CategoryStack.push_back(CurrentObject);
#define CFG_VALUE(Type, Name, DefaultValue, Description) \
	{                                                    \
		auto Found = (*CurrentObject).find(#Name);       \
		if (Found != (*CurrentObject).end())             \
		{                                                \
			Name = Found.value();                        \
			FORERUNNER_LOG(Config, "Loaded " #Name);     \
		}                                                \
	}
#define CFG_CAT(Name) { using namespace Name;            \
	CurrentObject = &(*CurrentObject)[#Name];            \
	CategoryStack.push_back(CurrentObject);
#define CFG_CAT_END(Name)                                       \
	CurrentObject = CategoryStack.at(CategoryStack.size() - 2); \
	CategoryStack.pop_back();                                   \
	};
	FORERUNNER_CONFIGS
#undef CFG_VALUE
#undef CFG_CAT
#undef CFG_CAT_END
}

void Config::Save(const std::filesystem::path& File)
{
	std::ofstream FileStream{File, std::ios::out};

	if (!FileStream.is_open())
	{
		char AnsiBuffer[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, File.c_str(), static_cast<int>(wcslen(File.c_str())) + 1, AnsiBuffer, sizeof(AnsiBuffer), NULL, NULL);

		FORERUNNER_ERROR(Config, "Couldn't save config file to {}", AnsiBuffer);
		return;
	}

	nlohmann::json JsonData = nlohmann::json::object();
	nlohmann::json* CurrentObject = &JsonData;
	std::vector<nlohmann::json*> CategoryStack;
	CategoryStack.push_back(CurrentObject);

#define CFG_VALUE(Type, Name, DefaultValue, Description) \
	{                                                    \
		(*CurrentObject)[#Name] = Name;                  \
	}
#define CFG_CAT(Name) { using namespace Name;            \
	(*CurrentObject)[#Name] = nlohmann::json::object();  \
	CurrentObject = &(*CurrentObject)[#Name];            \
	CategoryStack.push_back(CurrentObject);
#define CFG_CAT_END(Name)                                        \
	CurrentObject = CategoryStack.at(CategoryStack.size() - 2); \
	CategoryStack.pop_back();                                    \
	};
	FORERUNNER_CONFIGS
#undef CFG_VALUE
#undef CFG_CAT
#undef CFG_CAT_END

	FileStream << JsonData.dump(4) << std::endl;
}
