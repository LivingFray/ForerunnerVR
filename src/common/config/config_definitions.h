#pragma once
#include "json.hpp"
#include "common/utils/vectors.h"

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

enum class ETurnType
{
	SnapTurn,
	SmoothTurn,
	Disabled,
	E_MAX
};

NLOHMANN_JSON_SERIALIZE_ENUM(ETurnType, {
	{ETurnType::SnapTurn, "Snap Turn"},
	{ETurnType::SmoothTurn, "Smooth Turn"},
	{ETurnType::Disabled, "Disabled"},
});

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2, x, y);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3, x, y, z);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector4, x, y, z, w);

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
			CFG_VALUE(ETurnType, TurnType, ETurnType::SnapTurn, "The method used to apply the Look input to the camera.\n  Smooth: Rotate <SmoothTurnSpeed> degrees per second while input is given\n  Snap: Rotate <SnapTurnAmount> degrees once per input\n  Disabled: Only allow turning by physically moving") \
			CFG_VALUE(float, SnapTurnAmount, 45.0f, "Angle in degrees turned per input when <TurnType> is set to 'Snap Turn'")	\
			CFG_VALUE(float, SmoothTurnSpeed, 180.0f, "Turn rate in degrees/second turned while applying the Look input when <TurnType> is set to 'Smooth Turn'")	\
		CFG_CAT_END(Movement) \
		CFG_CAT(UI) \
			CFG_CAT(Menu) \
				CFG_VALUE(Vector3, Offset, Vector3(0.0f, 0.0f, -2.0f), "The relative location of the menu UI Overlay from the HMD.\nUnits are in meters, +x = right, +y = up, -z = forward") \
				CFG_VALUE(float, Scale, 2.0f, "Width of the menu UI overlay in meters") \
				CFG_VALUE(bool, FollowHead, false, "Move the menu UI overlay to always be in front of the HMD, rather than at a set position determined when the UI is first shown") \
			CFG_CAT_END(Menu) \
			CFG_CAT(Cinematics) \
				CFG_VALUE(bool, DisableLetterboxing, true, "Disable the black bars that appear at the top and bottom of the screen during cutscenes/cinematic moments") \
			CFG_CAT_END(Cinematics) \
		CFG_CAT_END(UI) \
	CFG_CAT_END(Forerunner)

// Settings specific to Halo2
#define CFG_DELTA \
	CFG_CAT(Delta) \
	CFG_CAT_END(Delta)

#define FORERUNNER_CONFIGS \
	CFG_LAUNCHER \
	CFG_FORERUNNER \
	CFG_DELTA