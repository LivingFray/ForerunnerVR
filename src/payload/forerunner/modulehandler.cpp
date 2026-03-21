#include "modulehandler.h"

#include <iostream>
#include "payload/forerunner/patch.h"
#include "payload/delta/deltamodule.h"
#include "common/utils/log.h"

void ModuleHandler::Initialise()
{
	// Do any shared initialisation logic first
	if (!Patch::Initialise())
	{
		return;
	}

	FORERUNNER_LOG(Forerunner, "ModuleHandler loaded, patching MCC");
	
	CreateGameEngine::Create();
	CreateGameEngine::Enable();
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