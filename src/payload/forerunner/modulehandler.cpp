#include "modulehandler.h"

#include <iostream>
#include "payload/forerunner/patch.h"
#include "payload/delta/deltamodule.h"
#include "common/utils/log.h"
#include "common/utils/utils.h"
#include "common/utils/utils.h"
#include <filesystem>

void ModuleHandler::Initialise(wchar_t* InDLLPath)
{
	wcscpy_s(DLLPath, InDLLPath);

	{
		char AnsiBuffer[sizeof(DLLPath) / sizeof(DLLPath[0])];
		WideCharToMultiByte(CP_ACP, 0, DLLPath, static_cast<int>(wcslen(DLLPath)) + 1, AnsiBuffer, sizeof(AnsiBuffer), NULL, NULL);
		FORERUNNER_LOG(Forerunner, "ModuleHandler loaded from {}", AnsiBuffer);
	}

	ForerunnerPath = std::filesystem::path{DLLPath}.parent_path();

	// Do any shared initialisation logic first
	if (!Patch::Initialise())
	{
		FORERUNNER_ERROR(Forerunner, "Failed to initialise patcher");
		return;
	}

	FORERUNNER_LOG(Forerunner, "Loading OpenVR...");

	std::filesystem::path Path = ForerunnerPath / L"openvr_api.dll";
	
	FORERUNNER_LOG(Forerunner, "Looking for OpenVR at {}", Path.string());

	OpenVRHandle = LoadLibraryW(Path.wstring().data());

	if (!OpenVRHandle)
	{
		FORERUNNER_ERROR(Forerunner, "Couldn't load OpenVR ({})", GetLastError());
	}
	else
	{
		FORERUNNER_LOG(Forerunner, "Successfully loaded OpenVR");
	}

	FORERUNNER_LOG(Forerunner, "Patching MCC...");
	
	bool bPatchSuccess = true;
	bPatchSuccess |= CreateGameEngine::Create();
	bPatchSuccess |= CreateGameEngine::Enable();

	if (!bPatchSuccess)
	{
		FORERUNNER_ERROR(Forerunner, "Couldn't patch MCC");
	}
	else
	{
		FORERUNNER_LOG(Forerunner, "Successfully patched MCC");
	}

	FORERUNNER_LOG(Forerunner, "Finished Initialising");
}

void ModuleHandler::LoadModule(GameModule GameId)
{
	FORERUNNER_LOG(Forerunner, "New game loaded, was {} now {}", LastGameId, GameId);

	// Unload active game
	switch (LastGameId)
	{
		case GameModule::HaloCE:
			break;
		case GameModule::Halo2:
			DeltaModule::Get().Deinitialise();
			break;
		case GameModule::Halo3:
			break;
		case GameModule::Halo4:
			break;
		case GameModule::Groundhog:
			break;
		case GameModule::HaloODST:
			break;
		case GameModule::HaloReach:
			break;
		case GameModule::NONE:
			break;
		default:
			break;
	}

	// Load new game
	switch (GameId)
	{
		case GameModule::HaloCE:
			break;
		case GameModule::Halo2:
			if (!DeltaModule::Get().Initialise())
			{
				FORERUNNER_WARN(Forerunner, "Delta Module failed to load");
			}
			break;
		case GameModule::Halo3:
			break;
		case GameModule::Halo4:
			break;
		case GameModule::Groundhog:
			break;
		case GameModule::HaloODST:
			break;
		case GameModule::HaloReach:
			break;
		case GameModule::NONE:
			break;
		default:
			break;
	}

	LastGameId = GameId;
}

void CreateGameEngine::Patch(void* GameState, GameModule GameId)
{
	Original(GameState, GameId);

	ModuleHandler::Get().LoadModule(GameId);
}