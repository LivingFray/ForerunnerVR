#include "inputcomponent.h"
#include "cameracomponent.h"
#include "deltamodule.h"

#include "payload/delta/blam/game/players.h"
#include "payload/delta/blam/simulation/simulation.h"

#include "common/utils/vectors.h"
#include "common/config/config.h"

void InputComponent::RegisterInputs()
{
	IVR* VR = DeltaModule::Get().VR;

	BindingMove = VR->RegisterVector2Input("default", "Move");
}

void InputComponent::UpdateInputs(simulation_update* update)
{
	player_action& action = update->player_actions[0];

	// Throttle is movement input, relative to the character facing direction (+X = forward, +Y = left)
	action.throttle = CalculateMovementInput();
}

real_vector2d InputComponent::CalculateMovementInput()
{
	IVR* VR = DeltaModule::Get().VR;

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

static float AngleBetweenVector2(const Vector2& v1, const Vector2& v2)
{
	const float dot = v1.dot(v2);
	const float determinant = v1.x * v2.y - v1.y * v2.x;
	const float angle = atan2(determinant, dot);
	return angle;
}

static Vector2 RotateVector2(const Vector2& v, float angle)
{
	Vector2 rotated;
	rotated.x = v.x * cos(angle) - v.y * sin(angle);
	rotated.y = v.x * sin(angle) + v.y * cos(angle);
	return rotated;
}

void InputComponent::ModifyInputVector(Vector2& Input, EVR_Controller Controller) const
{
	CameraComponent& CameraComponent = DeltaModule::Get().Camera;

	Matrix4 HMDTransform = CameraComponent.GetCameraTransform();
	Matrix4 ControllerTransform = CameraComponent.GetControllerTransform(Controller);

	Vector3 CameraFacing = HMDTransform.getLeftAxis();
	Vector3 ContollerFacing = ControllerTransform.getLeftAxis();

	const float angle = AngleBetweenVector2(Vector2(CameraFacing.x, CameraFacing.y), Vector2(ContollerFacing.x, ContollerFacing.y));
	Input = RotateVector2(Input, angle);
}
