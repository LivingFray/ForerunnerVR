#pragma once
#include "json.hpp"

// Define all configs used across the project here via a single X-Macro
// Must be done here so the launcher can edit game specific configs safely
// For organisational purposes configs within the macro can be split out into
// separate sub-macros

enum class EMovementInputType
{
	HMD,
	LeftHand,
	RightHand,
	E_MAX
};

NLOHMANN_JSON_SERIALIZE_ENUM(EMovementInputType, {
	{EMovementInputType::HMD, "HMD"},
	{EMovementInputType::LeftHand, "Left Hand"},
	{EMovementInputType::RightHand, "Right Hand"},
});



// Settings for the launcher exe
#define CFG_LAUNCHER \
	CFG_CAT(Launcher) \
		CFG_VALUE(bool, AutoLaunch, false, "Automatically attempt to launch MCC after the launcher is opened") \
		CFG_VALUE(bool, AutoInject, true, "Automatically inject into MCC once the process has been detected") \
		CFG_VALUE(bool, CloseOnInject, true, "Automatically close the launcher after mod injection") \
	CFG_CAT_END(Launcher) \

// Settings for the payload dll as a whole
#define CFG_FORERUNNER \
	CFG_CAT(Forerunner) \
		CFG_CAT(Movement) \
			CFG_VALUE(EMovementInputType, MovementDirection, EMovementInputType::HMD, "The relative direction in which movement inputs are applied") \
		CFG_CAT_END(Movement) \
	CFG_CAT_END(Forerunner)

// Settings specific to Halo2
#define CFG_DELTA \
	CFG_CAT(Delta) \
	CFG_CAT_END(Delta)

#define FORERUNNER_CONFIGS \
	CFG_LAUNCHER \
	CFG_FORERUNNER \
	CFG_DELTA