#include "inputcomponent.h"
#include "cameracomponent.h"
#include "deltamodule.h"
#include "payload/forerunner/forerunnermodule.h"

#include "payload/delta/blam/game/players.h"
#include "payload/delta/blam/main/main_time.h"
#include "payload/delta/blam/simulation/simulation.h"

#include "common/utils/utils.h"
#include "common/utils/vectors.h"
#include "common/config/config.h"

constexpr const char* DefaultActionSet = "delta_default";

void InputComponent::RegisterInputs()
{
	IVR* VR = ForerunnerModule::Get().VR;

	BindingMove = VR->RegisterVector2Input(DefaultActionSet, "Move");
	BindingLook = VR->RegisterVector2Input(DefaultActionSet, "Look");

	SetDefault = VR->RegisterActionSet(DefaultActionSet);
	VR->ActivateActionSet(SetDefault);
}

void InputComponent::UpdateInputs(simulation_update* update)
{
	player_action& action = update->player_actions[0];

	// Throttle is movement input, relative to the character facing direction (+X = forward, +Y = left)
	action.throttle = CalculateMovementInput();
	// Delegate rotation calculations to camera component
	DeltaModule::Get().Camera.UpdatePlayerCamera(CalculateTurnInput(), action.rotation.yaw, action.rotation.pitch);
}

real_vector2d InputComponent::CalculateMovementInput()
{
	IVR* VR = ForerunnerModule::Get().VR;

	// Assume this comes from a joystick input where +x = right, +y = up
	Vector2 Input = VR->GetVector2Input(BindingMove);

	switch (Config::Forerunner::Movement::MovementDirection)
	{
		case EMovementInputType::HMD:
			// Nothing to do, halo treats the input as camera relative already
			break;
		case EMovementInputType::LeftHand:
			ModifyInputVector(Input, EVR_Controller::Left);
			break;
		case EMovementInputType::RightHand:
			ModifyInputVector(Input, EVR_Controller::Right);
			break;
	}

	// Convert from (+X = right, +Y = up) to (+X = forward, +Y = left)
	return real_vector2d{Input.y, -Input.x};
}

float InputComponent::CalculateTurnInput()
{
	IVR* VR = ForerunnerModule::Get().VR;
	Vector2 Input = VR->GetVector2Input(BindingLook);

	switch (Config::Forerunner::Movement::TurnType)
	{
		case ETurnType::SnapTurn:
		{
			// Latch the snap input around half pressed
			constexpr float SnapMin = 0.4f;
			constexpr float SnapMax = 0.6f;

			if (bHadTurnInput)
			{
				if (abs(Input.x) < SnapMin)
				{
					bHadTurnInput = false;
				}
			}
			else
			{
				if (abs(Input.x) > SnapMax)
				{
					bHadTurnInput = true;
					return (Input.x > 0.0f ? 1.0f : -1.0f) * Config::Forerunner::Movement::SnapTurnAmount;
				}
			}

			return 0.0f;
		}
		case ETurnType::SmoothTurn:
		{
			return Config::Forerunner::Movement::SmoothTurnSpeed * Input.x * g_delta_time();
		}
		case ETurnType::Disabled:
			return 0.0f;
		default:
			break;
	}

	return 0.0f;
}

static float AngleBetweenVector2(const Vector2& V1, const Vector2& V2)
{
	const float Dot = V1.dot(V2);
	const float Determinant = V1.x * V2.y - V1.y * V2.x;
	const float Angle = atan2(Determinant, Dot);
	return Angle;
}

static Vector2 RotateVector2(const Vector2& V, float Angle)
{
	Vector2 Rotated;
	Rotated.x = V.x * cos(Angle) - V.y * sin(Angle);
	Rotated.y = V.x * sin(Angle) + V.y * cos(Angle);
	return Rotated;
}

void InputComponent::ModifyInputVector(Vector2& Input, EVR_Controller Controller) const
{
	CameraComponent& CameraComponent = DeltaModule::Get().Camera;

	Matrix4 HMDTransform = CameraComponent.GetCameraTransform();
	Matrix4 ControllerTransform = CameraComponent.GetControllerTransform(Controller);

	Vector3 CameraFacing = HMDTransform.getLeftAxis();
	Vector3 ContollerFacing = ControllerTransform.getLeftAxis();

	const float Angle = AngleBetweenVector2(Vector2(CameraFacing.x, CameraFacing.y), Vector2(ContollerFacing.x, ContollerFacing.y));
	Input = RotateVector2(Input, Angle);
}
